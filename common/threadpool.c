/*****************************************************************************
 * threadpool.c: thread pooling
 *****************************************************************************
 * Copyright (C) 2010-2025 x264 project
 *
 * Authors: Steven Walters <kemuri9@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@x264.com.
 *****************************************************************************/

#include "common.h"

#if HAVE_LIBNUMA
#include <numa.h>
#endif

typedef struct
{
    void *(*func)(void *);
    void *arg;
    void *ret;
} x264_threadpool_job_t;

typedef struct
{
    struct x264_threadpool_t *pool;
    int id;
} x264_worker_t;

struct x264_threadpool_t
{
    volatile int   exit;
    int            threads;
    x264_pthread_t *thread_handle;
    x264_worker_t  *workers;

    /* Per-thread NUMA node assignment; -1 = no affinity */
    int            *numa_node;

    /* requires a synchronized list structure and associated methods,
       so use what is already implemented for frames */
    x264_sync_frame_list_t uninit; /* list of jobs that are awaiting use */
    x264_sync_frame_list_t run;    /* list of jobs that are queued for processing by the pool */
    x264_sync_frame_list_t done;   /* list of jobs that have finished processing */
};

REALIGN_STACK static void *threadpool_thread( x264_worker_t *w )
{
    x264_threadpool_t *pool = w->pool;

#if HAVE_LIBNUMA
    if( pool->numa_node && pool->numa_node[w->id] >= 0 && numa_available() >= 0 )
    {
        struct bitmask *mask = numa_allocate_nodemask();
        numa_bitmask_setbit( mask, pool->numa_node[w->id] );
        numa_run_on_node_mask( mask );
        numa_set_interleave_mask( mask );
        numa_set_localalloc();
        numa_bitmask_free( mask );
    }
#endif

    while( !pool->exit )
    {
        x264_threadpool_job_t *job = NULL;
        x264_pthread_mutex_lock( &pool->run.mutex );
        while( !pool->exit && !pool->run.i_size )
            x264_pthread_cond_wait( &pool->run.cv_fill, &pool->run.mutex );
        if( pool->run.i_size )
        {
            job = (void*)x264_frame_shift( pool->run.list );
            pool->run.i_size--;
        }
        x264_pthread_mutex_unlock( &pool->run.mutex );
        if( !job )
            continue;
        job->ret = job->func( job->arg );
        x264_sync_frame_list_push( &pool->done, (void*)job );
    }
    return NULL;
}

/* Return the number of CPUs on a given NUMA node, or total CPUs if no NUMA. */
static int x264_numa_cpus_on_node( int node )
{
#if HAVE_LIBNUMA
    if( numa_available() >= 0 )
    {
        int max_cpu = numa_num_configured_cpus();
        int count = 0;
        for( int i = 0; i < max_cpu; i++ )
            if( numa_node_of_cpu( i ) == node )
                count++;
        if( count > 0 )
            return count;
    }
#else
    (void)node;
#endif
    return x264_cpu_num_processors();
}

/* Return the number of NUMA nodes (1 if no NUMA support). */
static int x264_numa_node_count( void )
{
#if HAVE_LIBNUMA
    if( numa_available() >= 0 )
        return numa_max_node() + 1;
#endif
    return 1;
}

/* Parse an x265-compatible pools string into per-thread NUMA node assignments.
 * Fills numa_node[0..max_threads-1] with node IDs (-1 = no affinity).
 * Returns the total number of threads requested, or:
 *   0  = use default (no affinity)
 *  -1  = "none" (disable threading) */
static int x264_pools_parse( const char *pools, int *numa_node, int max_threads )
{
    if( !pools || !*pools || !strcmp( pools, "*" ) )
        return 0;

    if( !strcasecmp( pools, "none" ) )
        return -1;

    int num_nodes = x264_numa_node_count();
    int total = 0;
    const char *p = pools;

    for( int node = 0; node < num_nodes && *p; node++ )
    {
        int count = 0;

        while( *p == ' ' ) p++;

        if( *p == '+' )
        {
            count = x264_numa_cpus_on_node( node );
            p++;
        }
        else if( *p == '-' )
        {
            count = 0;
            p++;
        }
        else if( *p == '*' )
        {
            /* All CPUs on this and all remaining nodes */
            for( int n = node; n < num_nodes; n++ )
            {
                int c = x264_numa_cpus_on_node( n );
                for( int j = 0; j < c && total < max_threads; j++ )
                    numa_node[total++] = n;
            }
            return total;
        }
        else if( *p >= '0' && *p <= '9' )
        {
            count = atoi( p );
            while( *p >= '0' && *p <= '9' ) p++;
            int node_cpus = x264_numa_cpus_on_node( node );
            if( count > node_cpus )
                count = node_cpus;
        }

        for( int j = 0; j < count && total < max_threads; j++ )
            numa_node[total++] = node;

        while( *p == ' ' ) p++;
        if( *p == ',' ) p++;
    }

    return total > 0 ? total : 0;
}

int x264_threadpool_init( x264_threadpool_t **p_pool, int threads, const char *pools )
{
    if( threads <= 0 )
        return -1;

    if( x264_threading_init() < 0 )
        return -1;

    x264_threadpool_t *pool;
    CHECKED_MALLOCZERO( pool, sizeof(x264_threadpool_t) );
    *p_pool = pool;

    /* Parse pools string for NUMA node assignments only;
     * thread count was already determined by the encoder. */
    int numa_map[X264_THREAD_MAX];
    for( int i = 0; i < X264_THREAD_MAX; i++ )
        numa_map[i] = -1;

    if( pools && *pools && strcasecmp( pools, "none" ) && strcmp( pools, "*" ) )
        x264_pools_parse( pools, numa_map, X264_THREAD_MAX );

    pool->threads = threads;

    CHECKED_MALLOC( pool->thread_handle, pool->threads * sizeof(x264_pthread_t) );
    CHECKED_MALLOC( pool->workers, pool->threads * sizeof(x264_worker_t) );
    CHECKED_MALLOC( pool->numa_node, pool->threads * sizeof(int) );

    for( int i = 0; i < pool->threads; i++ )
        pool->numa_node[i] = numa_map[i];

    if( x264_sync_frame_list_init( &pool->uninit, pool->threads ) ||
        x264_sync_frame_list_init( &pool->run, pool->threads ) ||
        x264_sync_frame_list_init( &pool->done, pool->threads ) )
        goto fail;

    for( int i = 0; i < pool->threads; i++ )
    {
       x264_threadpool_job_t *job;
       CHECKED_MALLOC( job, sizeof(x264_threadpool_job_t) );
       x264_sync_frame_list_push( &pool->uninit, (void*)job );
    }
    for( int i = 0; i < pool->threads; i++ )
    {
        pool->workers[i].pool = pool;
        pool->workers[i].id = i;
        if( x264_pthread_create( pool->thread_handle+i, NULL, (void*)threadpool_thread, &pool->workers[i] ) )
            goto fail;
    }

    return 0;
fail:
    return -1;
}

void x264_threadpool_run( x264_threadpool_t *pool, void *(*func)(void *), void *arg )
{
    x264_threadpool_job_t *job = (void*)x264_sync_frame_list_pop( &pool->uninit );
    job->func = func;
    job->arg  = arg;
    x264_sync_frame_list_push( &pool->run, (void*)job );
}

void *x264_threadpool_wait( x264_threadpool_t *pool, void *arg )
{
    x264_pthread_mutex_lock( &pool->done.mutex );
    while( 1 )
    {
        for( int i = 0; i < pool->done.i_size; i++ )
            if( ((x264_threadpool_job_t*)pool->done.list[i])->arg == arg )
            {
                x264_threadpool_job_t *job = (void*)x264_frame_shift( pool->done.list+i );
                pool->done.i_size--;
                x264_pthread_mutex_unlock( &pool->done.mutex );

                void *ret = job->ret;
                x264_sync_frame_list_push( &pool->uninit, (void*)job );
                return ret;
            }

        x264_pthread_cond_wait( &pool->done.cv_fill, &pool->done.mutex );
    }
}

static void threadpool_list_delete( x264_sync_frame_list_t *slist )
{
    for( int i = 0; slist->list[i]; i++ )
    {
        x264_free( slist->list[i] );
        slist->list[i] = NULL;
    }
    x264_sync_frame_list_delete( slist );
}

void x264_threadpool_delete( x264_threadpool_t *pool )
{
    x264_pthread_mutex_lock( &pool->run.mutex );
    pool->exit = 1;
    x264_pthread_cond_broadcast( &pool->run.cv_fill );
    x264_pthread_mutex_unlock( &pool->run.mutex );
    for( int i = 0; i < pool->threads; i++ )
        x264_pthread_join( pool->thread_handle[i], NULL );

    threadpool_list_delete( &pool->uninit );
    threadpool_list_delete( &pool->run );
    threadpool_list_delete( &pool->done );
    x264_free( pool->thread_handle );
    x264_free( pool->workers );
    x264_free( pool->numa_node );
    x264_free( pool );
}

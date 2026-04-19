# x264 — MediaMolder Fork

This is [MediaMolder](https://github.com/MediaMolder)'s fork of the
[x264](https://www.videolan.org/developers/x264.html) H.264/AVC encoder,
maintained as a component of the MediaMolder transcoding pipeline.

Upstream: <https://code.videolan.org/videolan/x264>

## Fork Additions

This fork adds the following features on top of upstream x264:

- **`--csv <file>`** — Write per-frame encoding statistics to a CSV log file.
- **`--pools <config>` / `--numa-pools <config>`** — NUMA-aware thread pool
  configuration (x265-compatible syntax).
- **`--lambda-file <file>`** — Load custom lambda/lambda2 tables from a text
  file for rate-distortion experimentation (x265-compatible feature).

## Building

```bash
./configure
make
```

Common configure options:

```bash
# Debug build
./configure --enable-debug

# Static library only
./configure --enable-static

# With assembly optimizations disabled (for debugging)
./configure --disable-asm
```

Requires: a C99 compiler (gcc, clang), NASM/YASM (for x86 assembly). Optional:
libavformat/libswscale (for extended input format support), GPAC or L-SMASH
(for MP4 output).

## Usage

```bash
# Constant quality encode
x264 --crf 23 -o output.mkv input.y4m

# Two-pass ABR
x264 --pass 1 --bitrate 5000 -o output.mkv input.y4m
x264 --pass 2 --bitrate 5000 -o output.mkv input.y4m

# With per-frame CSV logging
x264 --crf 23 --csv stats.csv -o output.mkv input.y4m

# Custom lambda tables
x264 --crf 23 --lambda-file lambdas.txt -o output.mkv input.y4m
```

Run `x264 --fullhelp` for all options, or see
[`doc/fullhelp.md`](doc/fullhelp.md) for the complete CLI reference.

## Documentation

- [`doc/fullhelp.md`](doc/fullhelp.md) — Comprehensive CLI parameter reference
- [`doc/ratecontrol.txt`](doc/ratecontrol.txt) — Rate control internals
- [`doc/threads.txt`](doc/threads.txt) — Threading model
- [`doc/vui.txt`](doc/vui.txt) — Video Usability Information (Annex E)
- [`doc/standards.txt`](doc/standards.txt) — Standards references

## License

x264 is free software licensed under the GNU GPL v2 (or later). See
[`COPYING`](COPYING) for details. A commercial license is also available —
contact licensing@x264.com.

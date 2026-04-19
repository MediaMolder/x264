# x264 Command Line Reference

Complete reference for all x264 CLI options. Every parameter that accepts an
argument lists its **type**, **default value**, and **accepted range or values**.

---

## Syntax

```
x264 [options] -o outfile infile
```

**infile** can be:

- Raw YUV (requires `--input-res` and optionally `--fps`, `--input-csp`)
- YUV4MPEG (`.y4m`)
- Avisynth (`.avs`, if compiled with support)
- Any format supported by libavformat (if compiled with `lavf` support)

**outfile** type is determined by extension:

| Extension | Format |
|-----------|--------|
| `.264`    | Raw H.264 bytestream |
| `.mkv`    | Matroska |
| `.flv`    | Flash Video |
| `.mp4`    | MP4 (requires GPAC or L-SMASH) |

Output bit depth: `8` and/or `10`, depending on build configuration.

---

## Quick Examples

```bash
# Constant quality (CRF)
x264 --crf 24 -o output.mkv input.y4m

# Two-pass ABR at 1000 kbps
x264 --pass 1 --bitrate 1000 -o output.mkv input.y4m
x264 --pass 2 --bitrate 1000 -o output.mkv input.y4m

# Lossless
x264 --qp 0 -o output.mkv input.y4m

# Constant bitrate with VBV (2-second buffer)
x264 --vbv-bufsize 2000 --bitrate 1000 -o output.mkv input.y4m
```

---

## Presets

### `--profile <string>`

Force the limits of an H.264 profile. Overrides all other settings to ensure
stream compliance. If set, lossless encoding (`--qp 0`) is unavailable except
with `high444`.

| Value | Type | Default | Accepted |
|-------|------|---------|----------|
| string | — | *not set* | `baseline`, `main`, `high`, `high10`, `high422`, `high444` |

**Profile constraints:**

| Profile | Restrictions |
|---------|-------------|
| `baseline` | `--no-8x8dct --bframes 0 --no-cabac --cqm flat --weightp 0`. No interlaced. No lossless. |
| `main` | `--no-8x8dct --cqm flat`. No lossless. |
| `high` | No lossless. 8-bit only. |
| `high10` | No lossless. 8-bit and 10-bit. |
| `high422` | No lossless. 8–10 bit. 4:2:0 and 4:2:2 chroma. |
| `high444` | 8–10 bit. 4:2:0, 4:2:2, and 4:4:4 chroma. Lossless allowed. |

Most decoders support `high`; setting this is only necessary when targeting
devices with known profile limitations.

---

### `--preset <string>`

Select a collection of encoding settings that trade compression efficiency
against encoding speed. Preset settings are applied first and will be overridden
by any explicitly specified option.

| Value | Type | Default | Accepted |
|-------|------|---------|----------|
| string | — | `medium` | `ultrafast`, `superfast`, `veryfast`, `faster`, `fast`, `medium`, `slow`, `slower`, `veryslow`, `placebo` |

Set this to the slowest you can tolerate. `placebo` offers negligible compression efficiency
gains over `veryslow` at enormous speed cost.

**Preset parameter matrix** (blank cells = same as `medium` default):

| Parameter | ultrafast | superfast | veryfast | faster | fast | **medium** | slow | slower | veryslow | placebo |
|-----------|-----------|-----------|----------|--------|------|------------|------|--------|----------|---------|
| 8x8dct | off | | | | | **on** | | | | |
| aq-mode | 0 | | | | | **1** | | | | |
| b-adapt | 0 | | | | | **1** | | 2 | 2 | 2 |
| bframes | 0 | | | | | **3** | | | 8 | 16 |
| cabac | off | | | | | **on** | | | | |
| deblock | off | | | | | **on** | | | | |
| direct | | | | | | **spatial** | auto | auto | auto | auto |
| fast-pskip | | | | | | **on** | | | | off |
| me | dia | dia | | | | **hex** | | umh | umh | tesa |
| merange | | | | | | **16** | | | 24 | 24 |
| mbtree | off | off | | | | **on** | | | | |
| mixed-refs | off | off | off | off | | **on** | | | | |
| partitions | none | i8x8,i4x4 | | | | **p8x8,b8x8,i8x8,i4x4** | | all | all | all |
| rc-lookahead | 0 | 0 | 10 | 20 | 30 | **40** | 50 | 60 | 60 | 60 |
| ref | 1 | 1 | 1 | 2 | 2 | **3** | 5 | 8 | 16 | 16 |
| scenecut | 0 | | | | | **40** | | | | |
| slow-firstpass | | | | | | **off** | | | | on |
| subme | 0 | 1 | 2 | 4 | 6 | **7** | 8 | 9 | 10 | 11 |
| trellis | 0 | 0 | 0 | | | **1** | 2 | 2 | 2 | 2 |
| weightb | off | | | | | **on** | | | | |
| weightp | 0 | 1 | 1 | 1 | 1 | **2** | | | | |

---

### `--tune <string>`

Fine-tune settings for a particular type of source material. Applied after
`--preset` but before any other user options. Multiple tunings are separated by
commas, but **only one psy tuning** can be used at a time.

| Value | Type | Default | Accepted |
|-------|------|---------|----------|
| string | — | *not set* | `film`, `animation`, `grain`, `stillimage`, `psnr`, `ssim`, `fastdecode`, `zerolatency` |

**Psy tunings** (mutually exclusive):

| Tuning | Changes |
|--------|---------|
| `film` | `--deblock -1:-1 --psy-rd <unset>:0.15` |
| `animation` | `--bframes {+2} --deblock 1:1 --psy-rd 0.4:<unset> --aq-strength 0.6 --ref {2×, min 1}` |
| `grain` | `--aq-strength 0.5 --no-dct-decimate --deadzone-inter 6 --deadzone-intra 6 --deblock -2:-2 --ipratio 1.1 --pbratio 1.1 --psy-rd <unset>:0.25 --qcomp 0.8` |
| `stillimage` | `--aq-strength 1.2 --deblock -3:-3 --psy-rd 2.0:0.7` |
| `psnr` | `--aq-mode 0 --no-psy` |
| `ssim` | `--aq-mode 2 --no-psy` |

**Non-psy tunings** (can combine with one psy tuning):

| Tuning | Changes |
|--------|---------|
| `fastdecode` | `--no-cabac --no-deblock --no-weightb --weightp 0` |
| `zerolatency` | `--bframes 0 --force-cfr --no-mbtree --sync-lookahead 0 --sliced-threads --rc-lookahead 0` |

---

### `--slow-firstpass`

*Type: boolean flag. Default: not set.*

By default, `--pass 1` forces faster settings to speed up the first pass:

- `--no-8x8dct --me dia --partitions none --ref 1 --subme {min(subme,2)} --trellis 0 --fast-pskip`

`--slow-firstpass` disables these overrides, using the same settings for both
passes. This produces a more accurate stats file at the cost of a slower first
pass. Recommended when `--b-adapt 2` or `--trellis 2` is used.

---

## Frame-Type Options

### `-I, --keyint <integer|"infinite">`

Maximum number of frames between IDR keyframes (maximum GOP size).

| Type | Default | Range |
|------|---------|-------|
| integer or `"infinite"` | `250` | `1` – `2^30` (`"infinite"`) |

IDR frames are random-access seek points. No frame can reference across an IDR
boundary. Larger values improve compression; smaller values improve seekability.
Set to `"infinite"` to never insert non-scenecut IDR frames.

I-frames are typically 10× larger than P/B-frames, so shorter GOPs produce
larger files.

---

### `-i, --min-keyint <integer>`

Minimum number of frames between IDR keyframes.

| Type | Default | Range |
|------|---------|-------|
| integer | `auto` = `min(keyint/10, fps)` | `0` – `keyint/2+1` |

Prevents IDR frames from being placed too close together (e.g., during strobing
scenes). The `auto` default is generally ideal.

---

### `--scenecut <integer>`

Scene change detection threshold. Higher values insert more I/IDR frames at
scene changes.

| Type | Default | Range |
|------|---------|-------|
| integer | `40` | `0` – any positive integer |

x264 computes a per-frame metric estimating visual difference from the previous
frame. When this metric exceeds `scenecut`, a scene change is detected. If less
than `--min-keyint` frames have elapsed since the last IDR, an I-frame (non-IDR)
is inserted; otherwise an IDR frame is placed.

Setting to `0` is equivalent to `--no-scenecut`.

---

### `--no-scenecut`

*Type: boolean flag. Default: not set.*

Completely disable adaptive I-frame decision. Only fixed-interval IDR frames
(per `--keyint`) will be inserted.

---

### `--intra-refresh`

*Type: boolean flag. Default: not set.*

Use Periodic Intra Refresh instead of IDR frames. A column of intra-coded
blocks moves across the frame over multiple frames, providing gradual refresh
without the large bitrate spike of a full IDR. Useful for low-latency streaming.

---

### `-b, --bframes <integer>`

Maximum number of consecutive B-frames between I and P frames.

| Type | Default | Range |
|------|---------|-------|
| integer | `3` | `0` – `16` |

B-frames use bidirectional prediction (past and future references) for better
compression. Higher values allow more B-frames, which can improve efficiency
but increases encoding latency and memory usage. Set to `0` to disable B-frames
entirely (useful for low-latency).

---

### `--b-adapt <integer>`

Adaptive B-frame placement algorithm.

| Type | Default | Range |
|------|---------|-------|
| integer | `1` | `0` – `2` |

| Value | Description |
|-------|-------------|
| `0` | Disabled — always use the maximum number of B-frames. |
| `1` | Fast — lightweight lookahead for B-frame decisions. |
| `2` | Optimal — Viterbi algorithm for B-frame placement (slow, especially with high `--bframes`). |

Higher values may reduce threading efficiency.

---

### `--b-bias <integer>`

Bias toward or away from B-frame usage.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` | `-100` – `100` |

Positive values make B-frames more likely; negative values favor P-frames.
This is an arbitrary metric; `100`/`-100` does not guarantee all/no B-frames.
Generally leave at default unless you have specific requirements.

---

### `--b-pyramid <string>`

Allow some B-frames to be used as references by other frames.

| Type | Default | Accepted |
|------|---------|----------|
| string | `normal` | `none`, `strict`, `normal` |

| Value | Description |
|-------|-------------|
| `none` | No B-frame references. |
| `strict` | Strictly hierarchical pyramid. One B-ref per minigop. Blu-ray compatible. |
| `normal` | Non-strict pyramid. Allows more flexible referencing. Not Blu-ray compatible. |

---

### `--open-gop`

*Type: boolean flag. Default: not set.*

Use recovery points to close GOPs instead of IDR frames. Allows better
compression across GOP boundaries. Requires `--bframes > 0`.

---

### `--no-cabac`

*Type: boolean flag. Default: not set (CABAC enabled).*

Disable CABAC (Context Adaptive Binary Arithmetic Coding) and use CAVLC
(Context Adaptive Variable Length Coding) instead. CABAC provides 10–20%
better compression but requires more decoding power. Disabling is required for
the Baseline profile.

---

### `-r, --ref <integer>`

Number of reference frames.

| Type | Default | Range |
|------|---------|-------|
| integer | `3` | `1` – `16` |

Controls the Decoded Picture Buffer (DPB) size. More references improve
compression quality (especially for complex motion) at the cost of encoding
speed and decoder memory. The effective limit may be lower depending on
`--level` and resolution.

---

### `--no-deblock`

*Type: boolean flag. Default: not set (deblock enabled).*

Completely disable the in-loop deblocking filter. Not recommended — the
deblocking filter is highly efficient in quality-per-bit.

---

### `-f, --deblock <alpha:beta>`

In-loop deblocking filter strength parameters.

| Type | Default | Range |
|------|---------|-------|
| integer pair `alpha:beta` | `0:0` | `-6` – `6` for each |

- **alpha** — Controls filter strength threshold. Higher values mean more
  aggressive deblocking.
- **beta** — Controls the range of pixels to which the filter is applied.
  Higher values blur more pixels.

Negative values reduce deblocking (sharper but blockier); positive values
increase it (smoother but blurrier). A common range for live action is `−2:−2`
to `0:0`. Animation may benefit from `1:1`.

---

### `--slices <integer>`

Force a specific number of slices per frame (rectangular slices).

| Type | Default | Range |
|------|---------|-------|
| integer | `0` (auto) | `0` – frame height in MBs |

Overridden by `--slice-max-size` or `--slice-max-mbs`. Set to `4` for
Blu-ray.

---

### `--slices-max <integer>`

Absolute maximum number of slices per frame.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` (unlimited) | `0` – frame height in MBs |

Caps slicing when `--slice-max-size` or `--slice-max-mbs` would otherwise
generate too many slices.

---

### `--slice-max-size <integer>`

Maximum size of each slice in bytes, including estimated NAL overhead.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` (unlimited) | any positive integer |

---

### `--slice-max-mbs <integer>`

Maximum number of macroblocks per slice.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` (unlimited) | any positive integer |

---

### `--slice-min-mbs <integer>`

Minimum number of macroblocks per slice.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` | any positive integer |

---

### `--tff`

*Type: boolean flag. Default: not set.*

Enable interlaced encoding with **top field first** field order.

---

### `--bff`

*Type: boolean flag. Default: not set.*

Enable interlaced encoding with **bottom field first** field order.

---

### `--constrained-intra`

*Type: boolean flag. Default: not set.*

Enable constrained intra prediction. Required for the base layer of SVC
encodes. Otherwise rarely needed.

---

### `--pulldown <string>`

Signal soft pulldown to change the frame rate using one of several preset
patterns. Requires constant frame rate input.

| Type | Default | Accepted |
|------|---------|----------|
| string | `none` | `none`, `22`, `32`, `64`, `double`, `triple`, `euro` |

Implies `--pic-struct`.

---

### `--fake-interlaced`

*Type: boolean flag. Default: not set.*

Flag the stream as interlaced but encode progressively. Makes it possible to
encode 25p and 30p Blu-Ray compliant streams. Ignored in interlaced mode.

---

### `--frame-packing <integer>`

Frame packing arrangement for stereoscopic 3D video.

| Type | Default | Range |
|------|---------|-------|
| integer | `-1` (not set) | `0` – `7` |

| Value | Arrangement |
|-------|-------------|
| `0` | Checkerboard — pixels alternate between L and R |
| `1` | Column alternation — L and R interlaced by column |
| `2` | Row alternation — L and R interlaced by row |
| `3` | Side by side — L on left, R on right |
| `4` | Top bottom — L on top, R on bottom |
| `5` | Frame alternation — one view per frame |
| `6` | Mono — 2D frame, no packing |
| `7` | Tile format — L on top-left, R split across |

---

## Rate Control

x264 offers three primary rate control modes. Only one can be active:

1. **CRF** (`--crf`) — Constant quality, variable bitrate. *Recommended for
   single-pass encoding.*
2. **CQP** (`--qp`) — Constant quantizer. Fixed quality per QP, no
   adaptation.
3. **ABR** (`--bitrate`) — Average bitrate targeting, typically used with
   two-pass.

If none is specified, CRF at 23.0 is used.

---

### `-q, --qp <integer>`

Force Constant QP mode. The specified QP applies to P-frames; I-frame and
B-frame QPs are derived via `--ipratio` and `--pbratio`.

| Type | Default | Range |
|------|---------|-------|
| integer | *not set* (CRF mode active) | `0` – `81` |

QP 0 enables mathematically lossless encoding (bypasses quantization). QPs
above 51 are only accessible in 10-bit mode and are extremely low quality.

---

### `-B, --bitrate <integer>`

Enable single-pass ABR (Average Bitrate) mode, targeting the specified average
bitrate.

| Type | Default | Range |
|------|---------|-------|
| integer (kbit/s) | *not set* | any positive integer |

Best used with two-pass encoding (`--pass 1` / `--pass 2`) for more accurate
bitrate allocation.

---

### `--crf <float>`

Constant Rate Factor — quality-based VBR. Targets a constant perceptual quality
rather than a specific bitrate or QP. CRF *n* targets approximately the same
quality as QP *n*, but uses fewer bits by reducing quality on complex or
high-motion frames where quality loss is less noticeable.

| Type | Default | Range |
|------|---------|-------|
| float | `23.0` | `-12.0` – `51.0` |

Lower values = higher quality and larger files. A change of ±6 roughly doubles
or halves the bitrate. Sane values for most content are `16` – `28`.

Negative CRF values (available in 10-bit mode) produce quality beyond QP 0 but
are not truly lossless — use `--qp 0` for that.

---

### `--rc-lookahead <integer>`

Number of frames to look ahead for frametype decisions (mb-tree and VBV
lookahead).

| Type | Default | Range |
|------|---------|-------|
| integer | `40` | `0` – `250` |

More frames give better rate control decisions but use more memory and increase
latency. For mb-tree, the effective value is `min(rc-lookahead, keyint)`. For
VBV, the effective value depends on `keyint`, `vbv-bufsize`, and `vbv-maxrate`.

---

### `--vbv-maxrate <integer>`

Maximum local bitrate, for VBV-constrained encoding.

| Type | Default | Range |
|------|---------|-------|
| integer (kbit/s) | `0` (disabled) | any positive integer |

Only effective when `--vbv-bufsize` is also set. VBV constrains the bitrate to
comply with decoder buffer models (e.g., for streaming or Blu-ray). VBV
reduces quality — only use when your target playback scenario requires it.

---

### `--vbv-bufsize <integer>`

Size of the VBV (Video Buffering Verifier) buffer.

| Type | Default | Range |
|------|---------|-------|
| integer (kbit) | `0` (disabled) | any positive integer |

Set together with `--vbv-maxrate`. For CRF mode, both must be specified to
enable VBV.

---

### `--vbv-init <float>`

Initial VBV buffer occupancy before playback begins.

| Type | Default | Range |
|------|---------|-------|
| float | `0.9` | `0.0` – `1.0` (fraction of `vbv-bufsize`) or kbit if `>1` |

---

### `--crf-max <float>`

Maximum ratefactor (minimum quality) allowed when using CRF with VBV.

| Type | Default | Range |
|------|---------|-------|
| float | *not set* | same range as `--crf` |

Prevents VBV from reducing quality below a threshold. **Warning:** may cause
VBV buffer underflows if set too aggressively.

---

### `--qpmin <integer>`

Minimum allowed QP. Prevents the encoder from using extremely high quality
(large) frames.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` | `0` – `81` |

---

### `--qpmax <integer>`

Maximum allowed QP. Prevents the encoder from dropping quality below a
threshold. The default of `81` effectively disables this limit. Values in the
`30`–`40` range are typically used to cap minimum quality.

| Type | Default | Range |
|------|---------|-------|
| integer | `81` | `0` – `81` |

---

### `--qpstep <integer>`

Maximum QP change between consecutive frames.

| Type | Default | Range |
|------|---------|-------|
| integer | `4` | any positive integer |

Limits abrupt quality shifts between frames. Lower values provide smoother
quality transitions but may make rate control less responsive.

---

### `--ratetol <float>`

ABR rate tolerance factor. In 1-pass ABR mode, this controls the allowed
deviation from the target average bitrate (in percent). In VBV mode, it affects
VBV strictness.

| Type | Default | Range |
|------|---------|-------|
| float | `1.0` | `0.01` – `inf` |

Higher values give the rate controller more freedom to allocate bits, which
helps with complex scenes near the end of the video. Set to `inf` to disable
overflow detection.

---

### `--ipratio <float>`

QP factor between I-frames and P-frames. Controls how much higher quality
I-frames get relative to P-frames.

| Type | Default | Range |
|------|---------|-------|
| float | `1.40` | any positive float |

Higher values increase I-frame quality at the cost of P/B-frame quality. Grain
tuning uses `1.1`. Typical range: `1.1` – `1.4`.

---

### `--pbratio <float>`

QP factor between P-frames and B-frames. Controls how much lower quality
B-frames get relative to P-frames.

| Type | Default | Range |
|------|---------|-------|
| float | `1.30` | any positive float |

Not used when mbtree is enabled (mbtree calculates the optimal value
automatically). Higher values decrease B-frame quality. Grain tuning uses
`1.1`.

---

### `--chroma-qp-offset <integer>`

QP difference between chroma and luma planes.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` | `-12` – `12` |

Negative values give more bits to chroma; positive values reduce chroma
quality. The H.264 spec applies an additional internal chroma QP offset
(`-2` for 4:2:0, `-6` for 4:4:4); this option adds on top of that.

---

### `-p, --pass <integer>`

Enable multi-pass rate control.

| Type | Default | Range |
|------|---------|-------|
| integer | *not set* | `1`, `2`, `3` |

| Value | Description |
|-------|-------------|
| `1` | First pass. Creates stats file. |
| `2` | Final pass. Reads stats file. Does not overwrite it. |
| `3` | Nth pass. Reads and overwrites stats file (for 3+ pass encoding). |

---

### `--stats <string>`

Filename for the two-pass statistics file.

| Type | Default |
|------|---------|
| string (filename) | `"x264_2pass.log"` |

---

### `--no-mbtree`

*Type: boolean flag. Default: not set (mbtree enabled).*

Disable macroblock-tree rate control. Mbtree tracks motion vector propagation
to allocate more bits to frequently-referenced blocks (e.g., static
backgrounds) and fewer bits to transient detail. Generally improves quality but
can be destructive with grainy content.

When enabled, `--pbratio` has no effect and `--qcomp` controls mbtree
strength (higher qcomp = weaker mbtree).

---

### `--qcomp <float>`

Quantizer curve compression factor. Controls bit distribution between simple
and complex scenes.

| Type | Default | Range |
|------|---------|-------|
| float | `0.60` | `0.0` – `1.0` |

- `0.0` → constant bitrate (all frames get equal bits regardless of complexity)
- `1.0` → constant quantizer (all frames get equal QP)

With mbtree, `qcomp` affects mbtree strength. Recommended: `0.60`–`0.70`
without mbtree; `0.70`–`0.85` with mbtree.

---

### `--cplxblur <float>`

Temporal blur applied to QP fluctuations **before** curve compression.

| Type | Default | Range |
|------|---------|-------|
| float | `20.0` | any non-negative float |

---

### `--qblur <float>`

Temporal blur applied to QP fluctuations **after** curve compression.

| Type | Default | Range |
|------|---------|-------|
| float | `0.5` | any non-negative float |

---

### `--zones <zone0>/<zone1>/...`

Override rate control and other settings for specific frame ranges. Zones are
separated by `/`. Each zone has the format:

```
<start>,<end>,<option>
```

where `<option>` is either:
- `q=<integer>` — force a specific QP, or
- `b=<float>` — bitrate multiplier (e.g., `b=1.5` = 150% bitrate)

If zones overlap, the later one takes precedence.

**Additional per-zone options** (comma-separated within a zone):

`ref`, `b-bias`, `scenecut`, `no-deblock`, `deblock`, `deadzone-intra`,
`deadzone-inter`, `direct`, `merange`, `nr`, `subme`, `trellis`,
`(no-)chroma-me`, `(no-)dct-decimate`, `(no-)fast-pskip`, `(no-)mixed-refs`,
`psy-rd`, `me`, `no-8x8dct`, `b-pyramid`

**Limitations:**
- `ref` cannot exceed the global `--ref` value
- `scenecut` cannot be toggled on/off, only varied
- `merange` cannot exceed the original value when using `esa`/`tesa`
- `subme` cannot be changed if originally set to `0`
- `me` cannot be set to `esa`/`tesa` if originally `dia`/`hex`/`umh`

*Example:*
```
--zones 0,1000,b=2/1001,2000,q=20,me=3,b-bias=-1000
```

---

### `--qpfile <string>`

Read per-frame QPs and frametypes from a text file.

| Type | Default |
|------|---------|
| string (filename) | *not set* |

**File format:** one line per frame:
```
framenumber frametype [QP]
```

- **framenumber** — 0-indexed frame number
- **frametype** — one of: `I` (IDR), `i` (non-IDR I-frame), `K` (I or i
  depending on `--open-gop`), `P`, `B` (ref), `b` (non-ref)
- **QP** — optional integer, clamped to `qpmin`/`qpmax`. Omit to let x264
  choose.

---

### `--lambda-file <string>`

Read custom lambda and lambda2 tables from a text file. This is inspired by the
x265 `--lambda-file` feature and is intended for experimentation with
rate-distortion tradeoffs.

| Type | Default |
|------|---------|
| string (filename) | *not set* |

The file must contain **82 float values** for `x264_lambda_tab` followed by
**82 float values** for `x264_lambda2_tab` (82 = QP_MAX_MAX + 1, covering QP 0
through 81).

- Commas are treated as whitespace.
- `#` starts a line comment.
- Lines must be less than 2048 bytes.
- `lambda_tab` values are clamped to `uint16_t` range (0–65535).
- `lambda2_tab` values are clamped to avoid overflow (max 134217727).

**Note:** Lambda tables are **process-global**. The override affects all
encoders running in the same process. Lower lambda values cause the encoder to
spend more bits on signaling (motion vectors, partitions) and fewer on residual.

---

## Analysis

### `-A, --partitions <string>`

Macroblock partition types to consider during analysis.

| Type | Default | Accepted |
|------|---------|----------|
| string (comma-separated) | `"p8x8,b8x8,i8x8,i4x4"` | `p8x8`, `p4x4`, `b8x8`, `i8x8`, `i4x4`, `none`, `all` |

- **I-frames:** `i8x8`, `i4x4`
- **P-frames:** `p8x8`, `p4x4` (p4x4 requires p8x8)
- **B-frames:** `b8x8`
- `i8x8` requires `--8x8dct`
- `all` and `none` are shorthand for enabling/disabling everything

More partitions improve compression but slow encoding. `p4x4` is rarely worth
the speed cost.

---

### `--direct <string>`

Direct MV prediction mode for B-frames.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"spatial"` | `none`, `spatial`, `temporal`, `auto` |

- `spatial` — predict motion vectors from neighboring blocks
- `temporal` — predict from co-located blocks in reference frames
- `auto` — select the best method per-frame (recommended for slower presets)

---

### `--no-weightb`

*Type: boolean flag. Default: not set (weighted B enabled).*

Disable weighted prediction for B-frames. Weighted prediction helps with fades
and lighting changes.

---

### `--weightp <integer>`

Weighted prediction for P-frames.

| Type | Default | Range |
|------|---------|-------|
| integer | `2` | `0` – `2` |

| Value | Description |
|-------|-------------|
| `0` | Disabled |
| `1` | Weighted refs — simple weighted prediction |
| `2` | Weighted refs + Duplicates — allows additional reference duplication for fades |

---

### `--me <string>`

Integer-pixel motion estimation algorithm.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"hex"` | `dia`, `hex`, `umh`, `esa`, `tesa` |

| Method | Description | Speed |
|--------|-------------|-------|
| `dia` | Diamond search, radius 1 | Fastest |
| `hex` | Hexagonal search, radius 2. Good general-purpose choice. | Fast |
| `umh` | Uneven multi-hexagon search. Better for HD/high-motion. | Medium |
| `esa` | Exhaustive search. Full search within `--merange`. | Slow |
| `tesa` | Hadamard exhaustive search. Slightly better than `esa`. | Slowest |

---

### `--merange <integer>`

Maximum motion vector search range in pixels.

| Type | Default | Range |
|------|---------|-------|
| integer | `16` | `4` – `16` (for `dia`/`hex`), `4` – `24`+ (for `umh`/`esa`/`tesa`) |

For `hex` and `dia`, clamped to 4–16. For `umh`, `esa`, and `tesa`, can be
increased for wider-range searches (useful for HD content). Larger values
significantly slow `umh`/`esa`/`tesa`.

---

### `--mvrange <integer>`

Maximum motion vector length in pixels.

| Type | Default | Range |
|------|---------|-------|
| integer | `-1` (auto, based on level) | level-dependent |

Auto values by level:

| Level | Max MV range |
|-------|-------------|
| 1/1b | 64 |
| 1.1–2.0 | 128 |
| 2.1–3.0 | 256 |
| 3.1+ | 512 |

When overriding, subtract 0.25 from these values (e.g., `--mvrange 127.75`).

---

### `--mvrange-thread <integer>`

Minimum buffer space between threads for motion vectors.

| Type | Default | Range |
|------|---------|-------|
| integer | `-1` (auto) | any integer |

Leave at default unless debugging threading issues.

---

### `-m, --subme <integer>`

Subpixel motion estimation and mode decision quality.

| Type | Default | Range |
|------|---------|-------|
| integer | `7` | `0` – `11` |

| Value | Description |
|-------|-------------|
| `0` | Fullpel only (not recommended) |
| `1` | SAD mode decision, one qpel iteration |
| `2` | SATD mode decision |
| `3`–`5` | Progressively more qpel refinement |
| `6` | RD mode decision for I/P-frames |
| `7` | RD mode decision for all frames |
| `8` | RD refinement for I/P-frames |
| `9` | RD refinement for all frames |
| `10` | QP-RD — requires `--trellis 2` and `--aq-mode > 0` |
| `11` | Full RD: disable all early terminations |

Values ≤1 enable a faster/lower-quality lookahead mode and poorer scenecut
decisions. RDO levels (6+) are significantly slower.

---

### `--psy-rd <float:float>`

Psychovisual optimization strength, specified as two colon-separated values.

| Type | Default |
|------|---------|
| float:float | `"1.0:0.0"` |

- **First value (psy-rdo):** Strength of psychovisual rate-distortion
  optimization. Distorts the frame slightly to preserve visual energy
  (sharpness). Requires `--subme >= 6`. Range: `0.0` – `~2.0`.
- **Second value (psy-trellis):** Strength of psychovisual trellis
  optimization. Requires `--trellis >= 1`. Experimental — may harm quality on
  animated content. Range: `0.0` – `~1.0`.

Higher psy-rdo helps sharper content; lower values for blurry/animated content.
Typical values: `0.6`–`1.1` for live action, `0.4`–`0.9` for animation.

---

### `--no-psy`

*Type: boolean flag. Default: not set (psy enabled).*

Disable all psychovisual optimizations. This will improve PSNR and SSIM metrics
but typically reduces perceived visual quality.

---

### `--no-mixed-refs`

*Type: boolean flag. Default: not set (mixed refs enabled).*

Don't decide references on a per-partition basis. Each macroblock uses a single
reference frame for all its partitions. Faster but slightly less efficient.

---

### `--no-chroma-me`

*Type: boolean flag. Default: not set (chroma ME enabled).*

Ignore chroma planes during motion estimation. Only use luma for ME decisions.

---

### `--no-8x8dct`

*Type: boolean flag. Default: not set (8x8dct enabled).*

Disable adaptive spatial transform size. When enabled, x264 can choose between
4×4 and 8×8 DCT transforms per macroblock. Disabling forces 4×4 only. Required
disabled for Baseline and Main profiles.

---

### `-t, --trellis <integer>`

Trellis rate-distortion quantization.

| Type | Default | Range |
|------|---------|-------|
| integer | `1` | `0` – `2` |

| Value | Description |
|-------|-------------|
| `0` | Disabled |
| `1` | Enabled only on the final encode of a macroblock |
| `2` | Enabled on all mode decisions (slow, used by `slow`+ presets) |

Trellis is incompatible with deadzones. Increases quality at moderate speed
cost.

---

### `--no-fast-pskip`

*Type: boolean flag. Default: not set (fast P-skip enabled).*

Disable early SKIP detection on P-frames. Very slightly increases quality at a
large speed cost.

---

### `--no-dct-decimate`

*Type: boolean flag. Default: not set (DCT decimation enabled).*

Disable DCT coefficient thresholding on P-frames. DCT decimation drops blocks
deemed "unnecessary" for improved coding efficiency with usually negligible
quality loss. Disabling helps preserve grain and fine detail.

---

### `--nr <integer>`

Noise reduction strength.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` (disabled) | `0` – `~1000` |

Performs fast, in-encoder noise reduction by estimating film noise and dropping
small details before quantization. Not as good as external NR filters but has
near-zero speed cost. Typical values: `100`–`1000` for denoising.

---

### `--deadzone-inter <integer>`

Inter-frame luma quantization deadzone size.

| Type | Default | Range |
|------|---------|-------|
| integer | `21` | `0` – `32` |

---

### `--deadzone-intra <integer>`

Intra-frame luma quantization deadzone size.

| Type | Default | Range |
|------|---------|-------|
| integer | `11` | `0` – `32` |

Deadzones control the level of fine detail that x264 arbitrarily drops without
attempting to preserve. Higher values drop more detail. Incompatible with
trellis.

---

### `--cqm <string>`

Custom quantization matrix preset.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"flat"` | `flat`, `jvt` |

`flat` uses uniform quantization; `jvt` uses the JVT-recommended matrices that
favor lower frequencies (may improve perceived quality at the cost of PSNR).

---

### `--cqmfile <string>`

Read custom quantization matrices from a JM-compatible text file.

| Type | Default |
|------|---------|
| string (filename) | *not set* |

Overrides `--cqm` and all `--cqm*` options.

---

### `--cqm4 <list>`

Set all 4×4 quant matrices. Takes a comma-separated list of **16 integers**.

---

### `--cqm8 <list>`

Set all 8×8 quant matrices. Takes a comma-separated list of **64 integers**.

---

### `--cqm4i, --cqm4p, --cqm8i, --cqm8p <list>`

Set both luma and chroma quant matrices for intra or inter, 4×4 or 8×8.

---

### `--cqm4iy, --cqm4ic, --cqm4py, --cqm4pc <list>`

Set individual quant matrices (intra/inter, luma/chroma, 4×4).

---

## Video Usability Info (Annex E)

VUI settings are metadata signals to the decoder/player. They do **not** affect
encoding quality or decisions — they are advisory hints. See `doc/vui.txt` for
details.

### `--overscan <string>`

Specify how to handle overscan.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"undef"` | `undef`, `show`, `crop` |

---

### `--videoformat <string>`

Indicate the original video format before digitizing.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"undef"` | `component`, `pal`, `ntsc`, `secam`, `mac`, `undef` |

---

### `--range <string>`

Specify color range (luma/chroma signal range).

| Type | Default | Accepted |
|------|---------|----------|
| string | `"auto"` | `auto`, `tv` (limited), `pc` (full) |

---

### `--colorprim <string>`

Color primaries for RGB conversion.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"undef"` | `undef`, `bt709`, `bt470m`, `bt470bg`, `smpte170m`, `smpte240m`, `film`, `bt2020`, `smpte428`, `smpte431`, `smpte432` |

---

### `--transfer <string>`

Opto-electronic transfer characteristics (gamma curve).

| Type | Default | Accepted |
|------|---------|----------|
| string | `"undef"` | `undef`, `bt709`, `bt470m`, `bt470bg`, `smpte170m`, `smpte240m`, `linear`, `log100`, `log316`, `iec61966-2-4`, `bt1361e`, `iec61966-2-1`, `bt2020-10`, `bt2020-12`, `smpte2084`, `smpte428`, `arib-std-b67` |

---

### `--colormatrix <string>`

Color matrix coefficients for YCbCr-to-RGB conversion.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"undef"` | `undef`, `bt709`, `fcc`, `bt470bg`, `smpte170m`, `smpte240m`, `GBR`, `YCgCo`, `bt2020nc`, `bt2020c`, `smpte2085`, `chroma-derived-nc`, `chroma-derived-c`, `ICtCp` |

---

### `--chromaloc <integer>`

Chroma sample location.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` | `0` – `5` |

---

### `--mastering-display <string>`

SMPTE ST 2086 mastering display color volume SEI.

| Type | Default |
|------|---------|
| string | *not set* |

Format: `G(x,y)B(x,y)R(x,y)WP(x,y)L(max,min)`

Primaries and white point are in units of 0.00002; luminance values in units of
0.0001 cd/m².

*Example (P3D65 1000-nit display):*
```
G(13250,34500)B(7500,3000)R(34000,16000)WP(15635,16450)L(10000000,1)
```

---

### `--cll <string>`

Content light level SEI (CEA 861.3).

| Type | Default |
|------|---------|
| string | *not set* |

Format: `max_content,max_frame_average`

*Example:* `--cll "1000,400"` (MaxCLL=1000, MaxFALL=400 cd/m²)

---

### `--alternative-transfer <string>`

Alternative transfer characteristics SEI.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"undef"` | Same values as `--transfer` |

---

### `--nal-hrd <string>`

Signal HRD (Hypothetical Reference Decoder) information. Requires
`--vbv-bufsize`.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"none"` | `none`, `vbr`, `cbr` |

`cbr` is not allowed in `.mp4` containers. `cbr` implies `--filler`.

---

### `--filler`

*Type: boolean flag. Default: not set.*

Force hard-CBR mode and insert filler NAL units to maintain constant bitrate.
Implied by `--nal-hrd cbr`.

---

### `--pic-struct`

*Type: boolean flag. Default: not set.*

Force `pic_struct` in Picture Timing SEI. Implied by `--pulldown` and
`--tff`/`--bff`.

---

### `--crop-rect <string>`

Add cropping information to the bitstream-level cropping rectangle.

| Type | Default |
|------|---------|
| string `left,top,right,bottom` | *not set* |

---

## Input/Output

### `-o, --output <string>`

Output file name. **Required.**

| Type | Default |
|------|---------|
| string (filename) | *none* |

Output format is determined by file extension (`.264`, `.mkv`, `.flv`, `.mp4`)
or by `--muxer`.

---

### `--muxer <string>`

Output container format.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"auto"` | `auto`, `raw`, `mkv`, `flv`, `mp4` |

`auto` selects based on output filename extension.

---

### `--demuxer <string>`

Input container format / demuxer.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"auto"` | `auto`, `raw`, `y4m`, `avs`, `lavf` |

`auto` selects based on input filename extension, then tries `lavf`.

---

### `--input-fmt <string>`

Input file format hint (for lavf demuxer). Useful when format cannot be
auto-detected (e.g., reading from a pipe).

| Type | Default |
|------|---------|
| string | *auto-detected* |

---

### `--input-csp <string>`

Input colorspace format for raw input. Available values depend on the demuxer
in use.

| Type | Default |
|------|---------|
| string | `"i420"` |

**Common values for `raw` demuxer:** `i400`, `i420`, `yv12`, `nv12`, `nv21`,
`i422`, `yv16`, `nv16`, `yuyv`, `uyvy`, `i444`, `yv24`, `bgr`, `bgra`, `rgb`

The `lavf` demuxer supports many additional pixel formats — run
`x264 --fullhelp` for the complete list.

---

### `--output-csp <string>`

Output (encoded) colorspace.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"i420"` | `i400`, `i420`, `i422`, `i444`, `rgb` |

---

### `--input-depth <integer>`

Bit depth of raw input video.

| Type | Default | Range |
|------|---------|-------|
| integer | `8` | `8` – `16` |

---

### `--output-depth <integer>`

Bit depth of encoded output.

| Type | Default | Range |
|------|---------|-------|
| integer | *auto (from input or build)* | `8`, `10` |

---

### `--input-range <string>`

Color range of input video.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"auto"` | `auto`, `tv`, `pc` |

---

### `--input-res <intxint>`

Resolution of raw input video.

| Type | Default |
|------|---------|
| `WIDTHxHEIGHT` | *auto-detected from container* |

Required for raw input. Example: `--input-res 1920x1080`

---

### `--index <string>`

Filename for input index file (used by the lavf demuxer).

| Type | Default |
|------|---------|
| string (filename) | *not set* |

---

### `--sar <width:height>`

Sample Aspect Ratio. Specifies the ratio of width to height of individual
pixels. Used together with frame dimensions to determine the Display Aspect
Ratio: `DAR = SAR × width/height`.

| Type | Default |
|------|---------|
| `width:height` | *not set* |

---

### `--fps <float|rational>`

Source frame rate.

| Type | Default |
|------|---------|
| float or rational (`num/den`) | *auto-detected or 25* |

Auto-detected from y4m, avs, lavf headers. Falls back to `25` for raw input.
Setting this implies `--force-cfr`.

*Examples:* `--fps 29.970`, `--fps 30000/1001`, `--fps 24`

---

### `--seek <integer>`

First frame to encode (0-indexed). Skips preceding frames.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` | `0` – *end of input* |

---

### `--frames <integer>`

Maximum number of frames to encode.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` (all) | any positive integer |

---

### `--level <string>`

Enforce an H.264 level (Annex A). The encoder clamps settings to comply with
the level's limits on bitrate, DPB size, MV range, etc.

| Type | Default | Accepted |
|------|---------|----------|
| string | `-1` (auto) | `1`, `1.1`, `1.2`, `1.3`, `2`, `2.1`, `2.2`, `3`, `3.1`, `3.2`, `4`, `4.1`, `4.2`, `5`, `5.1`, `5.2`, `6`, `6.1`, `6.2` |

For general hardware compatibility, `4.1` is a safe choice. Auto-detection
derives the level from the encode's actual parameters.

---

### `--bluray-compat`

*Type: boolean flag. Default: not set.*

Enable compatibility hacks for Blu-ray disc authoring. Adjusts slice structure,
VBV, and reference frame constraints.

---

### `--avcintra-class <integer>`

Use AVC-Intra compatibility mode for a specific class.

| Type | Default | Accepted |
|------|---------|----------|
| integer | *not set* | `50`, `100`, `200`, `300`, `480` |

---

### `--avcintra-flavor <string>`

AVC-Intra flavor (vendor compatibility).

| Type | Default | Accepted |
|------|---------|----------|
| string | `"panasonic"` | `panasonic`, `sony` |

---

### `--stitchable`

*Type: boolean flag. Default: not set.*

Disable content-adaptive SPS/PPS header optimization. Ensures that
independently-encoded segments can be concatenated without header conflicts.

---

### `-v, --verbose`

*Type: boolean flag. Default: not set.*

Print per-frame encoding statistics.

---

### `--no-progress`

*Type: boolean flag. Default: not set.*

Don't show the progress indicator during encoding.

---

### `--quiet`

*Type: boolean flag. Default: not set.*

Suppress all non-error output.

---

### `--log-level <string>`

Maximum logging verbosity.

| Type | Default | Accepted |
|------|---------|----------|
| string | `"info"` | `none`, `error`, `warning`, `info`, `debug` |

---

### `--psnr`

*Type: boolean flag. Default: not set.*

Compute and report PSNR (Peak Signal-to-Noise Ratio) at the end of encoding.
Minor speed cost.

---

### `--ssim`

*Type: boolean flag. Default: not set.*

Compute and report SSIM (Structural Similarity) at the end of encoding. Minor
speed cost.

---

### `--threads <integer>`

Number of encoding threads.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` (auto) | `0` – `128` |

Auto-detection uses approximately 1.5× the number of CPU cores. More threads
beyond this provides diminishing returns.

---

### `--lookahead-threads <integer>`

Number of threads for lookahead analysis.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` (auto) | `0` – *auto* |

---

### `--sliced-threads`

*Type: boolean flag. Default: not set.*

Use slice-based threading instead of frame-based threading. Adds no encoding
latency but produces lower quality and compression efficiency. The maximum
number of sliced threads is `min((height+15)/16/4, 128)`.

Recommended only for real-time streaming or scenarios where latency is
critical.

---

### `--thread-input`

*Type: boolean flag. Default: not set.*

Run Avisynth input in its own thread.

---

### `--sync-lookahead <integer>`

Number of buffer frames for threaded lookahead.

| Type | Default | Range |
|------|---------|-------|
| integer | `-1` (auto) | `-1` – `250` |

---

### `--pools <string>`

NUMA thread pool configuration (x265-compatible syntax).

| Type | Default |
|------|---------|
| string | *not set* (default threading) |

| Value | Meaning |
|-------|---------|
| `"+"` | All cores, all NUMA nodes (with affinity) |
| `"8,8"` | 8 threads per NUMA node |
| `"+,-"` | All cores on node 0 only |
| `"none"` or `"-"` | Disable thread pools |
| `"*"` or `""` | Default (all cores, no affinity) |

Requires libnuma on Linux for actual NUMA pinning. On other platforms, controls
only thread count.

---

### `--numa-pools <string>`

Alias for `--pools`.

---

### `--non-deterministic`

*Type: boolean flag. Default: not set.*

Allow non-deterministic optimizations in multithreaded mode. Slightly improves
quality at the cost of reproducibility.

---

### `--cpu-independent`

*Type: boolean flag. Default: not set.*

Ensure exact reproducibility across different CPU architectures by using
canonical algorithms instead of CPU-optimal ones.

---

### `--asm <integer>`

Override CPU capability detection with a specific CPU flags bitmask.

| Type | Default | Range |
|------|---------|-------|
| integer | *auto-detected* | CPU flag bitmask |

---

### `--no-asm`

*Type: boolean flag. Default: not set.*

Disable all CPU SIMD optimizations. Useful for debugging or reproducibility
testing.

---

### `--opencl`

*Type: boolean flag. Default: not set.*

Enable OpenCL-accelerated lookahead when available.

---

### `--opencl-clbin <string>`

Path to compiled OpenCL kernel cache file.

| Type | Default |
|------|---------|
| string (filename) | *not set* |

---

### `--opencl-device <integer>`

OpenCL device ordinal (to select which GPU to use).

| Type | Default | Range |
|------|---------|-------|
| integer | `0` | `0` – *number of devices−1* |

---

### `--dump-yuv <string>`

Save reconstructed frames to a YUV file for debugging.

| Type | Default |
|------|---------|
| string (filename) | *not set* |

---

### `--csv <string>`

Write per-frame encoding statistics to a CSV log file.

| Type | Default |
|------|---------|
| string (filename) | *not set* |

---

### `--sps-id <integer>`

Set SPS and PPS id numbers.

| Type | Default | Range |
|------|---------|-------|
| integer | `0` | `0` – `31` |

---

### `--aud`

*Type: boolean flag. Default: not set.*

Emit Access Unit Delimiter NAL units at the start of each access unit. Required
for Blu-ray compliance.

---

### `--force-cfr`

*Type: boolean flag. Default: not set.*

Force constant framerate timestamp generation, ignoring input timestamps.

---

### `--tcfile-in <string>`

Force timestamp generation from a timecode file.

| Type | Default |
|------|---------|
| string (filename) | *not set* |

---

### `--tcfile-out <string>`

Output a timecode v2 file from input timestamps.

| Type | Default |
|------|---------|
| string (filename) | *not set* |

---

### `--timebase <int/int | integer>`

Specify timebase for timestamp handling.

| Type | Default |
|------|---------|
| rational (`num/den`) or integer | *auto* |

When a rational is given, it sets the timebase numerator and denominator. When
a single integer is given, it sets the numerator (for timecode file input) or
denominator (for other input).

---

### `--dts-compress`

*Type: boolean flag. Default: not set.*

Eliminate initial decoding delay using a container DTS hack. Useful for live
streaming.

---

## Filtering

### `--vf, --video-filter <filter0>/<filter1>/...`

Apply video filters to the input before encoding. Filters are chained with `/`.
Options for each filter use `:option=value` syntax.

**Available filters:**

#### `crop:left,top,right,bottom`

Remove pixels from the edges of the frame.

#### `resize:[width,height][,sar][,fittobox][,csp][,method]`

Resize frames based on the given criteria:

- **resolution only** — resizes and adapts SAR to avoid stretching
- **sar only** — sets SAR and resizes to avoid stretching
- **resolution and sar** — resizes to given resolution and sets the SAR
- **fittobox** — resize based on constraints: `width`, `height`, `both`
- **csp** — convert colorspace. Format: `[name][:depth]`
  - Valid names: `i400`, `i420`, `yv12`, `nv12`, `nv21`, `i422`, `yv16`,
    `nv16`, `yuyv`, `uyvy`, `i444`, `yv24`, `bgr`, `bgra`, `rgb`
- **method** — resize algorithm (if libswscale is available)

# Julia Fractal — MPI + OpenMP

Animated Julia set renderer using MPI for frame distribution and OpenMP for pixel-level parallelism.
Renders 300 frames at 1920×1080 (configurable in `src/include/consts.h`) and saves as `animation.avi`.

> Default settings in the repo are kept low (400×300, 750 frames) for fast test runs.
> The benchmarks below were measured at 1920×1080, 300 frames, MAX_ITER=256.

---

## Quick Start

```bash
# Build
make

# Run with default 4 MPI processes
make run

# Custom process count
make run NP=8

# Custom process count + OpenMP threads
OMP_NUM_THREADS=4 make run NP=4
```

---

## Requirements

```bash
sudo apt install mpich libopenmpi-dev openmpi-bin
```

ffmpeg is fetched automatically by `make run` if not present.

---

## Folder Structure

```
HPP-Julia-Template/
├── main.cc                   # MPI init, frame distribution, gather, video export
├── src/
│   ├── frame.cc / frame.h    # Frame class — WIDTH×HEIGHT pixel buffer
│   ├── animation.cc / ...    # Animation class — vector of frames
│   └── include/
│       ├── consts.h          # FRAMES, WIDTH, HEIGHT, MAX_ITER, CYCLE_FRAMES
│       └── CImg.h            # Image/video library (bundled)
├── Makefile
└── README.md
```

---

## Parallelism Design

| Level            | Tool   | How                                                                 |
|------------------|--------|---------------------------------------------------------------------|
| Frame distribution | MPI  | Striped — node `i` renders frames `i, i+NP, i+2*NP, ...`           |
| Pixel rendering  | OpenMP | `#pragma omp parallel for schedule(static)` over x-columns per frame |
| Julia iteration  | —      | Not parallelisable — each step depends on the previous value of z   |

Striping is chosen over chunking because early frames are heavier to render (more pixels reach MAX_ITER at low zoom). Striping distributes heavy and light frames evenly across nodes automatically.

Frames are gathered using `MPI_Send` / `MPI_Recv` per frame rather than `MPI_Gatherv`, which caused crashes due to a bug in `animation::operator=` in the template.

---

## Performance Results

Platform: Intel i9-14900K, WSL2/Ubuntu 24, 1920×1080, 300 frames, MAX_ITER=256.
Baseline (NP=1, OMP=1): **16.98 s**

| Configuration       | NP  | OMP | Time (s) | Speedup | Efficiency |
|---------------------|-----|-----|----------|---------|------------|
| Baseline            | 1   | 1   | 16.98    | 1.00×   | 100%       |
| MPI                 | 2   | 1   | 12.49    | 1.36×   | 68%        |
| MPI                 | 4   | 1   | 10.78    | 1.58×   | 39%        |
| MPI                 | 6   | 1   | 10.34    | 1.64×   | 27%        |
| **MPI — optimum**   | **8** | **1** | **9.91** | **1.71×** | **21%** |
| MPI                 | 12  | 1   | 11.61    | 1.46×   | 12%        |
| MPI                 | 16  | 1   | 12.86    | 1.32×   | 8%         |
| MPI                 | 32  | 1   | 16.88    | 1.01×   | 3%         |
| OpenMP only         | 1   | 4   | 16.27    | 1.04×   | 26%        |
| MPI+OMP (unstable)  | 4   | 4   | ~595 s   | —       | —          |

**Sweet spot: NP=8, OMP=1.**

Above NP=8, MPI communication overhead (300 × Send/Recv) outweighs the compute gain and total time rises again. At NP=32 the runtime is nearly identical to serial. The MPI+OMP combination crashes or hangs under WSL2 due to `libgomp` stack limits — `MPI_THREAD_FUNNELED` helps but does not fully resolve it at NP=4 + OMP=4.

### Amdahl's Law

With S=1.71× at NP=8, the formula `S = 1 / (s + (1−s)/N)` gives **s ≈ 42% serial fraction**.
This is almost entirely `save_video()` running on root after gather.

Theoretical maximum speedup: **S_max = 1/0.42 ≈ 2.4×**, regardless of how many processes are added.

---

## Tuning the render

Edit `src/include/consts.h` to change resolution, frame count, and iteration depth:

```cpp
constexpr unsigned int MAX_ITER    = 256;   // Higher = more detail, slower
constexpr unsigned int FRAMES      = 300;   // Total frames in animation
constexpr unsigned int CYCLE_FRAMES = 300;  // Length of rotational repeat
constexpr unsigned int WIDTH       = 1920;  // Horizontal resolution
constexpr unsigned int HEIGHT      = 1080;  // Vertical resolution
```

Then rebuild with `make clean && make`.

---

## Reference

Based on the **HPP Julia assignment** — High Performance Programming, Hogeschool Utrecht, 2025.
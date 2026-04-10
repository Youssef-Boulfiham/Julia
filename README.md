# Julia Fractal — MPI + OpenMP

Animated Julia set renderer using **MPI** for frame distribution and **OpenMP** for pixel-level parallelism. Renders 750 frames at 400×300 and saves as `animation.avi`.

> 📺 Demo video coming soon

---

## Quick Start

```bash
# Build
make

# Run (default: 4 MPI processes)
make run

# Custom process count + OpenMP threads
OMP_NUM_THREADS=4 make run NP=8
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
├── main.cc          # Entry point — MPI init, frame distribution, gather, video export
├── frame.h/.cc      # Frame class — WIDTH×HEIGHT pixel buffer
├── animation.h/.cc  # Animation class — vector of frames
├── consts.h         # FRAMES, WIDTH, HEIGHT, MAX_ITER, CYCLE_FRAMES
├── CImg.h           # Image/video library (bundled)
├── Makefile
└── README.md
```

---

## Parallelism

| Level | Tool | How |
|---|---|---|
| Frame distribution | MPI | Striped — node `i` renders frames `i, i+size, i+2*size, ...` |
| Pixel rendering | OpenMP | `#pragma omp parallel for` over x-columns per frame |

---

## Reference

Based on the [HPP Julia assignment](https://github.com/AI-S2-2025-CD/HPP-Julia-Template) — High Performance Programming, 2025.
#include <cmath>
#include <complex>
#include <numbers>
#include <ostream>
#include <omp.h>
#include <iostream>
#include <mpi.h>

#include "consts.h"
#include "frame.h"
#include "animation.h"

#define cimg_display 0
#include "src/include/CImg.h"

using std::cout, std::endl;
using namespace std::literals::complex_literals;

inline constexpr pixel COLOURISE(double iter) {
  iter = fmod(4 - iter * 5 / MAX_ITER, 6);
  byte x = static_cast<byte>(255 * (1 - std::abs(fmod(iter, 2) - 1)));
  byte r, g, b;
  if      (             iter < 1) { r = 255; g =   x; b =   0; }
  else if (iter >= 1 && iter < 2) { r =   x; g = 255; b =   0; }
  else if (iter >= 2 && iter < 3) { r =   0; g = 255; b =   x; }
  else if (iter >= 3 && iter < 4) { r =   0; g =   x; b = 255; }
  else if (iter >= 4 && iter < 5) { r =   x; g =   0; b = 255; }
  else                            { r = 255; g =   0; b =   x; }
  return { r, g, b };
}

void renderFrame(frame &f, unsigned int t) {
  double a = 2 * std::numbers::pi * t / CYCLE_FRAMES;
  double r = 0.7885;
  std::complex<double> c = r * cos(a) + static_cast<std::complex<double>>(1i) * r * sin(a);

  double x_y_range = 2;
  double scale = 1.5 - 1.45 * log(1 + 9.0 * t / FRAMES) / log(10);

  #pragma omp parallel for schedule(static) num_threads(4)
  for (unsigned int x = 0; x < WIDTH; x++) {
    for (unsigned int y = 0; y < HEIGHT; y++) {
      double re = (2 * x_y_range * static_cast<double>(x) / WIDTH)  - (x_y_range * 3.0 / 4);
      double im = (2 * x_y_range * static_cast<double>(y) / HEIGHT) - x_y_range;
      re *= scale;
      im *= scale;

      double zr = re, zi = im;
      unsigned int iter = 0;
      while ((zr*zr + zi*zi) < (x_y_range * x_y_range) && iter < MAX_ITER) {
        double tmp = zr*zr - zi*zi + c.real();
        zi = 2*zr*zi + c.imag();
        zr = tmp;
        iter++;
      }
      pixel col = (iter == MAX_ITER) ? pixel{0, 0, 0} : COLOURISE((double)iter);
      f.set_colour(x, y, col);
    }
  }
}

int main(int argc, char *argv[]) {
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  MPI_Datatype mpi_frame;
  MPI_Type_contiguous(FRAME_SIZE, MPI_BYTE, &mpi_frame);
  MPI_Type_commit(&mpi_frame);

  // Root initialiseert de volledige animatie
  animation full;
  if (rank == 0) full.initialise(FRAMES);

  // Elke node rendert zijn frames en stuurt ze 1 voor 1 naar root
  // op de juiste positie via MPI_Send/Recv
  for (unsigned int f = 0; f < FRAMES; f++) {
    if ((int)(f % size) == rank) {
      // Deze node rendert frame f
      frame local_frame;
      renderFrame(local_frame, f);
      if (rank == 0) {
        // Root heeft het al lokaal, kopieer direct
        full[f] = local_frame;
      } else {
        // Stuur naar root
        MPI_Send(&local_frame, 1, mpi_frame, 0, f, MPI_COMM_WORLD);
      }
    } else if (rank == 0) {
      // Root ontvangt van de juiste node
      MPI_Recv(&full[f], 1, mpi_frame, f % size, f, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }

  if (rank == 0) {
    cout << "All frames gathered, saving video..." << endl;
    cimg_library::CImg<byte> img(WIDTH, HEIGHT, FRAMES, 3);
    cimg_forXYZ(img, x, y, z) {
      img(x,y,z,RED)   = full[z].get_channel(x, y, RED);
      img(x,y,z,GREEN) = full[z].get_channel(x, y, GREEN);
      img(x,y,z,BLUE)  = full[z].get_channel(x, y, BLUE);
    }
    img.save_video("animation.avi");
    cout << "Video saved: animation.avi" << endl;
  }

  MPI_Type_free(&mpi_frame);
  MPI_Finalize();
}

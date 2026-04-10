#include "frame.h"

void frame::set_colour(unsigned int x, unsigned int y, pixel const& c) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
    throw std::invalid_argument("Out of bounds!");
  } else {
    this->data[(x*HEIGHT+y)*3+RED] = c[RED];
    this->data[(x*HEIGHT+y)*3+GREEN] = c[GREEN];
    this->data[(x*HEIGHT+y)*3+BLUE] = c[BLUE];
  }
}

byte frame::get_channel(unsigned int x, unsigned int y, unsigned int c) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
    throw std::invalid_argument("Out of bounds!");
  } else if (c < 0 || c >= 3) {
    throw std::invalid_argument("Invalid channel!");
  } else {
    return this->data[(x*HEIGHT+y)*3+c];
  }
}

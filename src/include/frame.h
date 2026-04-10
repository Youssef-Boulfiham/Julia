#pragma once

#include <array>
#include <memory>
#include <vector>
#include <stdexcept>
#include "consts.h"

typedef unsigned char byte;
typedef std::array<byte, 3> pixel;
constexpr unsigned int FRAME_SIZE = WIDTH * HEIGHT * 3;

typedef unsigned int channel;
constexpr channel RED = 0;
constexpr channel GREEN = 1;
constexpr channel BLUE = 2;

class frame {
  private:
    byte data[FRAME_SIZE] {0};
  public:
    void set_colour(unsigned int x, unsigned int y, pixel const& c);
    byte get_channel(unsigned int x, unsigned int y, channel c); 
};

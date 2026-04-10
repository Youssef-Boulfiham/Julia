#pragma once

#include <memory>
#include <vector>
#include "frame.h"

class animation {
private:
  std::unique_ptr<std::vector<frame>> frames;
  bool initialised;
public:
  // Uninitialised animation, ensuring non-root copies of gather do not allocate space.
  animation();
  // Initialised animation, used for frames which needs to be initialised on every node.
  animation(unsigned int size);
  // Rule of three...
  ~animation();
  // Delayed initialisation, only used in root node.
  void initialise(unsigned int size);
  // Rule of three...
  animation& operator=(const animation& other);
  // Access to underlying data.
  frame* data();
  // Overloading [] operator for semantic ergonomics.
  frame& operator[](unsigned int index);
};


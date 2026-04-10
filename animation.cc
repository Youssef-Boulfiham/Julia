#include "animation.h"

animation::animation() {
  this->frames = nullptr;
  this->initialised = false;
}

animation::animation(unsigned int size) {
  this->initialise(size);
}

animation::~animation() {
  if (this->initialised)
    this->frames = nullptr;
}

void animation::initialise(unsigned int size) {
  this->frames = std::make_unique<std::vector<frame>>(size);
  this->initialised = true;
}

animation& animation::operator=(const animation& other) {
  if (this != &other) { // This won't work when self-assigning.

    // Allocate and fill new memory.
    std::unique_ptr<std::vector<frame>> new_frames = std::make_unique<std::vector<frame>>(other.frames->size());
    std::copy(other.frames.get(), other.frames.get() + other.frames->size(), new_frames.get());

    // Deallocate existing (to be overwritten) memory.
    this->frames = nullptr;

    // Map new memory to this.
    this->frames = std::move(new_frames);
    this->initialised = true;
  }
  return *this;
}

frame* animation::data() {
  if (this->initialised) // Only available if initialised - on root.
    return this->frames->data();
  else // Other nodes will call this function, but do not use the returned pointer, so we can get away with nullptr.
    return nullptr;
}

frame& animation::operator[](unsigned int index) {
  return (*this->frames)[index];
}

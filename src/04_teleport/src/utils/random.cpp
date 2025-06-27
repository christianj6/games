#include "random.h"
#include "raylib.h"

Vector3 get_random_world_position(std::optional<float> y) {
  // TODO: remove hard-coded max world position
  RandomNumberGenerator<int> random_world_position(5, 80);

  if (y.has_value()) {
  } else {
    y = random_world_position();
  }
  return Vector3{static_cast<float>(random_world_position()), y.value(),
                 static_cast<float>(random_world_position())};
}

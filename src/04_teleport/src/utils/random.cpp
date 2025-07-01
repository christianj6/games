#include "random.h"
#include "raylib.h"

Vector3 get_random_world_position(std::optional<float> y) {
  // TODO: remove hard-coded max world position
  RandomNumberGenerator<int> random_world_position(5, 80);

  if (y.has_value()) {
  } else {
    y = random_world_position();
  }
  return Vector3{static_cast<float>(random_world_position()),
                 static_cast<float>(y.value()),
                 static_cast<float>(random_world_position())};
}

Vector3 get_random_unobstructed_world_position(
    std::optional<float> y,
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &vector_space_data) {
  for (int i = 0; i < 5; i++) {
    Vector3 random_world_position = get_random_world_position(y);
    if (!vector_space_data[static_cast<int>(random_world_position.y)](
            static_cast<int>(random_world_position.x),
            static_cast<int>(random_world_position.z))) {
      return random_world_position;
    }
  }
  // If no unobstructed position found after 5 attempts, return position at
  // origin
  return Vector3{0.0f, y.value_or(0.0f), 0.0f};
}

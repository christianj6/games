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

Vector3 find_unobstructed_position_near(
    const Vector2 &center,
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &vector_space_data) {

  // Search in a 8x8 area around the center (16x16 total)
  const int search_radius = 8;

  // Create random number generators for position offsets
  RandomNumberGenerator<int> random_offset(-2, 2);

  // Store valid positions
  std::vector<Vector3> valid_positions;

  // Try positions in a spiral pattern from center outward
  for (int layer = 0; layer <= search_radius; layer++) {
    for (int dx = -layer; dx <= layer; dx++) {
      for (int dz = -layer; dz <= layer; dz++) {
        // Only check points on the current layer (manhattan distance = layer)
        if (abs(dx) + abs(dz) == layer) {
          int check_x = static_cast<int>(center.x) + dx;
          int check_z =
              static_cast<int>(center.y) + dz; // center.y is the z coordinate

          // Check if position is within bounds and unobstructed
          if (check_x >= 0 && check_z >= 0 &&
              check_x < vector_space_data[1].cols() &&
              check_z < vector_space_data[1].rows() &&
              !vector_space_data[1](check_x, check_z)) {

            // Add random offset to the valid position
            Vector3 pos{static_cast<float>(check_x) + random_offset(), 0.0f,
                        static_cast<float>(check_z) + random_offset()};
            valid_positions.push_back(pos);

            // Randomly return this position (20% chance)
            if (GetRandomValue(0, 4) == 0 && valid_positions.size() > 3) {
              return pos;
            }
          }
        }
      }
    }
  }

  // If we found any valid positions, return a random one
  if (!valid_positions.empty()) {
    return valid_positions[GetRandomValue(0, valid_positions.size() - 1)];
  }

  // If no unobstructed position found, return the center point
  return Vector3{center.x, 0.0f, center.y}; // center.y is the z coordinate
}

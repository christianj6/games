#include "pathfinder.h"
#include "fmt/core.h"

void Map::set_vector_space_data(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &vector_space_data) {
  this->vector_space_data = vector_space_data;
}

float Map::LeastCostEstimate(void *stateStart, void *stateEnd) {
  Node start = Node::FromState(stateStart);
  Node end = Node::FromState(stateEnd);

  // Manhattan distance
  float cost = abs(end.x - start.x) + abs(end.y - start.y);
  return cost;
}

void Map::AdjacentCost(void *state,
                       MP_VECTOR<micropather::StateCost> *adjacent) {
  Node current = Node::FromState(state);

  const int VOXEL_SIZE = 85;

  // Four possible moves: right, left, forward, back
  const int dx[] = {1, -1, 0, 0};
  const int dz[] = {0, 0, 1, -1};

  for (int i = 0; i < 4; ++i) {
    int newX = current.x + dx[i];
    int newZ = current.y + dz[i]; // y in Node represents z in world space

    // Check bounds against voxel space size
    if (newX >= 0 && newX < VOXEL_SIZE && newZ >= 0 && newZ < VOXEL_SIZE) {
      // Check if position is walkable and not adjacent to obstacles
      if (!vector_space_data[1](newX, newZ)) {
        // Check surrounding cells for obstacles
        bool near_obstacle = false;
        for (int dx = -1; dx <= 1; dx++) {
          for (int dz = -1; dz <= 1; dz++) {
            int checkX = newX + dx;
            int checkZ = newZ + dz;
            if (checkX >= 0 && checkX < VOXEL_SIZE && checkZ >= 0 &&
                checkZ < VOXEL_SIZE) {
              if (vector_space_data[1](checkX, checkZ)) {
                near_obstacle = true;
                break;
              }
            }
          }
          if (near_obstacle)
            break;
        }

        // Only add the position if it's not next to an obstacle
        if (!near_obstacle) {
          Node neighbor(newX, newZ);
          micropather::StateCost stateCost;
          stateCost.state = (void *)(size_t)neighbor.ToState();
          stateCost.cost = 1.0f;
          adjacent->push_back(stateCost);
        }
      }
    }
  }
}

void Map::PrintStateInfo(void *state) {
  // TODO
}

#include "world.h"
#include "fmt/core.h"
#include "mesh.h"
#include "raylib.h"
#include "raymath.h"
#include "utils/random.h"
#include <vector>

// Define node type with proper state handling for micropather
struct Node {
  int x;
  int y;
  Node(int _x, int _y) : x(_x), y(_y) {}

  // Convert node to state integer (for micropather internal handling)
  void *ToState() const { return (void *)(size_t)(x + y * 1000); }

  // Convert state integer back to node
  static Node FromState(void *state) {
    size_t val = (size_t)state;
    int y = (int)(val / 1000);
    int x = (int)(val % 1000);
    return Node(x, y);
  }

  bool IsValid() const { return x >= 0 && x < 1000 && y >= 0 && y < 1000; }
};

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
  fmt::print("Estimating cost from ({},{}) to ({},{}): {}\n", start.x, start.y,
             end.x, end.y, cost);
  return cost;
}

void Map::AdjacentCost(void *state,
                       MP_VECTOR<micropather::StateCost> *adjacent) {
  Node current = Node::FromState(state);

  fmt::print("Finding neighbors for ({},{})\n", current.x, current.y);
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
          fmt::print("  Added neighbor ({},{})\n", newX, newZ);
        }
      }
    }
  }
  fmt::print("Found {} neighbors\n", adjacent->size());
}

void Map::PrintStateInfo(void *state) {
  // TODO
}

World::World()
    : renderer(),
      voxel_space(VOXEL_SIZE,
                  Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>::Constant(
                      VOXEL_SIZE, VOXEL_SIZE, false)),
      map(), pather(nullptr) {
  build_voxel_space();
  // currently we split floor, columns, and ceiling into separate meshes
  build_voxel_space_meshes(std::vector<int>(
      {1,
       static_cast<int>(
           voxel_space.size() - 60 +
           1), // add the extra one because we turned off the ceiling
       static_cast<int>(voxel_space.size())}));

  configure_materials();
  renderer.configure_lighting();

  // Setup pathfinding after world construction is complete
  map.set_vector_space_data(voxel_space);
  pather = new micropather::MicroPather(&map, 250);
}

void World::update(float dt, Camera player_camera) {
  renderer.update(player_camera);
}

void World::draw() {
  // note: shifting mesh calculation here has a huge performance hit,
  // but would be necessary in some way if we want a dynamic game world
  // (minecraft) we can think about this, and relevance of instancing strategy,
  // for 05_explore
  for (auto &mesh : meshes) {
    DrawMesh(mesh, material_default, MatrixIdentity());
  }
  renderer.draw();
  // Simple pathfinding test in a small area
  bool pathTest = true;
  if (pathTest) {
    pather->Reset();
    micropather::MPVector<void *> path;
    float totalCost = 0;

    // Test pathfinding between two points in the voxel space
    int startX = 10, startZ = 10;
    int endX = 50, endZ = 50;

    // Only test if both positions are walkable (not walls)
    if (voxel_space[1](startX, startZ) == false &&
        voxel_space[1](endX, endZ) == false) {
      fmt::print("Both positions are walkable\n");
      Node startNode(startX, startZ);
      Node endNode(endX, endZ);

      if (!startNode.IsValid() || !endNode.IsValid()) {
        fmt::print("Invalid node coordinates!\n");
        return;
      }

      fmt::print("Created start node ({},{}) and end node ({},{})\n",
                 startNode.x, startNode.y, endNode.x, endNode.y);

      fmt::print("\nTesting path from ({},{}) to ({},{})\n", startX, startZ,
                 endX, endZ);

      void *startState = startNode.ToState();
      void *endState = endNode.ToState();
      int result = pather->Solve(startState, endState, &path, &totalCost);

      if (result == micropather::MicroPather::SOLVED) {
        fmt::print("Path found! Cost: {:.2f}\n", totalCost);

        // Draw start and end points
        DrawSphere({static_cast<float>(startX), 3.0f, startZ * 1.0f}, 2.0f,
                   GREEN);
        DrawSphere({static_cast<float>(endX), 3.0f, endZ * 1.0f}, 2.0f, RED);

        // Draw path
        for (unsigned i = 0; i < path.size() - 1; ++i) {
          Node current = Node::FromState(path[i]);
          Node next = Node::FromState(path[i + 1]);

          // Draw line segments between path points
          DrawLine3D({current.x * 1.0f, 3.0f, current.y * 1.0f},
                     {next.x * 1.0f, 3.0f, next.y * 1.0f}, YELLOW);

          // Draw small spheres at each path point
          DrawSphere({current.x * 1.0f, 3.0f, current.y * 1.0f}, 0.5f, BLUE);
        }
      } else {
        fmt::print("No path found! Error code: {}\n", result);
      }
    } else {
      fmt::print("Start or end position is blocked by a wall\n");
    }
  }
}

void World::build_voxel_space() {
  RandomNumberGenerator<float> random_voxel(0.0f, 1.0f);
  RandomNumberGenerator<int> random_height(1, 20);

  const int floor_y = 0;
  const int ceiling_y = VOXEL_SIZE - 60;

  for (int i = 0; i < VOXEL_SIZE; i++) {
    for (int j = 0; j < VOXEL_SIZE; j++) {
      // create floor and ceiling
      voxel_space[floor_y](i, j) = true;
      /*voxel_space[ceiling_y](i, j) = true;*/

      // randomly create columns from floor and ceiling
      if (random_voxel() < VOXEL_DENSITY) {
        for (int k = 0; k < random_height(); k++) {
          voxel_space[floor_y + k](i, j) = true;
          voxel_space[ceiling_y - k](i, j) = true;
        }
      }
    }
  }
}

void World::build_voxel_space_meshes(const std::vector<int> &layer_partitions) {
  // voxel space is built into meshes by layers
  int start = 0;
  for (auto &end : layer_partitions) {
    meshes.push_back(
        merge_voxels(slice_voxel_space(voxel_space, start, end), start));
    start = end;
  }
}

void World::configure_materials() {
  material_default = LoadMaterialDefault();
  material_default.shader = renderer.get_shader();
  material_default.maps[MATERIAL_MAP_DIFFUSE].color = BLACK;
}
World::~World() { delete pather; }

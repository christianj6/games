#pragma once
#include "ai/micropather.h"
#include "graphics/renderer.h"
#include "raylib.h"
#include <Eigen/Dense>
#include <vector>

// TODO: consider moving map to another module
class Map : public micropather::Graph {
private:
  std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
      vector_space_data;

public:
  void set_vector_space_data(
      const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
          &vector_space_data);
  /**
          Return the least possible cost between 2 states. For example, if your
     pathfinding is based on distance, this is simply the straight distance
     between 2 points on the map. If you pathfinding is based on minimum time,
     it is the minimal travel time between 2 points given the best possible
     terrain.
  */
  float LeastCostEstimate(void *stateStart, void *stateEnd);

  /**
          Return the exact cost from the given state to all its neighboring
     states. This may be called multiple times, or cached by the solver. It
     *must* return the same exact values for every call to MicroPather::Solve().
     It should generally be a simple, fast function with no callbacks into the
     pather.
  */
  void AdjacentCost(void *state, MP_VECTOR<micropather::StateCost> *adjacent);

  /**
          This function is only used in DEBUG mode - it dumps output to stdout.
     Since void* aren't really human readable, normally you print out some
     concise info (like "(1,2)") without an ending newline.
  */
  void PrintStateInfo(void *state);
};

class World {
public:
  World();
  ~World();
  void update(float, Camera);
  void draw();
  const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
  get_voxel_space_data() const {
    return voxel_space;
  }

private:
  // for some reason mesh rendering is limited to this size
  static constexpr int VOXEL_SIZE = 85;
  // this is a reasonable density; again mesh limitations prevent high density
  static constexpr float VOXEL_DENSITY = 0.055f;

  void build_voxel_space();
  void build_voxel_space_meshes(const std::vector<int> &);
  void configure_materials();

  std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> voxel_space;
  std::vector<Mesh> meshes;

  Material material_default;
  Renderer renderer;
  Map map;
  micropather::MicroPather *pather;
};

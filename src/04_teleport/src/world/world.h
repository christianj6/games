#pragma once
#include "graphics/renderer.h"
#include "raylib.h"
#include <Eigen/Dense>
#include <vector>

class World {
public:
  World();
  void update(float, Camera);
  void draw();

private:
  // for some reason mesh rendering is limited to this size
  static constexpr int VOXEL_SIZE = 85;
  // this is a reasonable density
  static constexpr float VOXEL_DENSITY = 0.055f;

  Renderer renderer;
  Matrix *transforms;
  int active_voxel_count;
  std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> voxel_space;

  void get_initial_world_state();
  void configure_materials();
  Mesh ground;
  Mesh merge_voxels(
      const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> &,
      int);
  Mesh columns;
  Material material_default;
};

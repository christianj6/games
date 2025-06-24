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
  static constexpr int VOXEL_SIZE = 80;
  static constexpr float VOXEL_DENSITY = 0.02f;

  Renderer renderer;
  Matrix *transforms;
  int active_voxel_count;
  std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> voxel_space;

  void get_initial_world_state();
  void configure_materials();
  void merge_voxels();
  Mesh merged_mesh;
  Material material_default;
};

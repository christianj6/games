#pragma once
#include "graphics/renderer.h"
#include "raylib.h"
#include <Eigen/Dense>

class World {
public:
  World();
  void update(float, Camera);
  void draw();

private:
  static constexpr int VOXEL_SIZE = 200;
  static constexpr float VOXEL_DENSITY = 0.001f; // 10% of voxels will be filled

  Renderer renderer;
  Matrix *transforms;
  int active_voxel_count;
  Eigen::Array<bool, Eigen::Dynamic, Eigen::Dynamic> voxel_space;

  void get_initial_world_state();
  void configure_materials();
  void merge_voxels();
  Mesh merged_mesh;
  Material material_default;
};

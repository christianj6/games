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
};

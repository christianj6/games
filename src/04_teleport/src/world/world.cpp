#include "world.h"
#include "fmt/core.h"
#include "mesh.h"
#include "raylib.h"
#include "raymath.h"
#include "utils/random.h"
#include <vector>

World::World()
    : renderer(),
      voxel_space(VOXEL_SIZE,
                  Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>::Constant(
                      VOXEL_SIZE, VOXEL_SIZE, false)) {
  build_voxel_space();
  // currently we split floor, columns, and ceiling into separate meshes
  build_voxel_space_meshes(
      std::vector<int>({1, static_cast<int>(voxel_space.size() - 60),
                        static_cast<int>(voxel_space.size())}));

  configure_materials();
  renderer.configure_lighting();
}

void World::update(float dt, Camera player_camera) {
  float player_camera_position[3] = {player_camera.position.x,
                                     player_camera.position.y,
                                     player_camera.position.z};
  renderer.update(player_camera_position);
}

void World::draw() {
  // note: shifting mesh calculation here has a huge performance hit,
  // but would be necessary in some way if we want a dynamic game world
  // (minecraft) we can think about this, and relevance of instancing strategy,
  // for 05_explore
  for (auto &mesh : meshes) {
    DrawMesh(mesh, material_default, MatrixIdentity());
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
      voxel_space[ceiling_y](i, j) = true;

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

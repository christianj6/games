#include "world.h"
#include "raylib.h"
#include <Eigen/Dense>
#include <random>

#define MAX_INSTANCES 10000

void World::get_initial_world_state() {
  // initialize 2d eigen array to represent the 3d voxel space
  voxel_space.resize(VOXEL_SIZE * VOXEL_SIZE, VOXEL_SIZE);
  voxel_space.setConstant(false);

  // random number generation
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  // randomly set some voxels to active
  active_voxel_count = 0;
  for (int x = 0; x < VOXEL_SIZE; x++) {
    for (int y = 0; y < VOXEL_SIZE; y++) {
      for (int z = 0; z < VOXEL_SIZE; z++) {
        if (dist(gen) < VOXEL_DENSITY) {
          voxel_space(x * VOXEL_SIZE + y, z) = true;
          active_voxel_count++;
        }
      }
    }
  }

  // allocate transform matrix
  transforms = (Matrix *)RL_CALLOC(active_voxel_count, sizeof(Matrix));

  const float scale = 1.0f;                        // Size of each voxel
  const float offset = -VOXEL_SIZE * scale / 2.0f; // Center the world

  // Create a matrix of all positions where voxels are active
  Eigen::MatrixXf positions(4, active_voxel_count);
  int current_voxel = 0;

  for (int x = 0; x < VOXEL_SIZE; x++) {
    for (int y = 0; y < VOXEL_SIZE; y++) {
      for (int z = 0; z < VOXEL_SIZE; z++) {
        if (voxel_space(x * VOXEL_SIZE + y, z)) {
          positions.col(current_voxel) << x * scale + offset,
              y * scale + offset, z * scale + offset, 1.0f;
          current_voxel++;
        }
      }
    }
  }

  // Create transformation matrix (just translation in this case)
  Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();

  // Apply transform to all positions at once
  Eigen::MatrixXf transformed = transform * positions;

  // Convert to array of raylib matrices
  for (int i = 0; i < active_voxel_count; i++) {
    transforms[i] = {
        transform(0, 0), transform(0, 1), transform(0, 2), transformed(0, i),
        transform(1, 0), transform(1, 1), transform(1, 2), transformed(1, i),
        transform(2, 0), transform(2, 1), transform(2, 2), transformed(2, i),
        transform(3, 0), transform(3, 1), transform(3, 2), transformed(3, i)};
  }
}

void World::configure_materials() {
  // instanced materials use the special shader
  material_instanced = LoadMaterialDefault();
  material_instanced.shader = renderer.get_shader();
  material_instanced.maps[MATERIAL_MAP_DIFFUSE].color = RED;

  // otherwise use a default material
  material_default = LoadMaterialDefault();
  material_default.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
}

World::World() : renderer() {
  get_initial_world_state();
  configure_materials();
  renderer.configure_lighting((Vector3){50.0f, 50.0f, 0.0f});

  // mesh used for building the voxel world
  mesh_cube = GenMeshCube(1.0f, 1.0f, 1.0f);
}

void World::update(float dt, Camera player_camera) {
  float player_camera_position[3] = {player_camera.position.x,
                                     player_camera.position.y,
                                     player_camera.position.z};
  renderer.update(player_camera_position);
}

void World::draw() {
  DrawMeshInstanced(mesh_cube, material_instanced, transforms,
                    active_voxel_count);
}

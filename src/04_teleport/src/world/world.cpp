#include "world.h"
#include "raylib.h"
#include <Eigen/Dense>
#include <random>

#define MAX_INSTANCES 10000

void World::get_initial_world_state() {
  transforms = (Matrix *)RL_CALLOC(MAX_INSTANCES, sizeof(Matrix));

  std::random_device rd;
  std::mt19937 gen(rd());
  const float radius = 40.0f;           // Radius of the circle
  const float height_variation = 10.0f; // How much height varies
  std::uniform_real_distribution<float> height_dist(-height_variation,
                                                    height_variation);
  std::uniform_real_distribution<float> angle_dist(0.0f, 360.0f);

  for (int i = 0; i < MAX_INSTANCES; i++) {
    // Calculate position on circle
    float angle = (static_cast<float>(i) / MAX_INSTANCES) * 2.0f * M_PI;
    float x = radius * std::cos(angle);
    float z = radius * std::sin(angle);
    float y = height_dist(gen); // Random height variation

    // Create translation matrix using Eigen
    Eigen::Vector3f translation(x, y, z);
    Eigen::Translation3f trans(translation);

    // Create random rotation matrix using Eigen
    Eigen::Vector3f axis(1.0f, 1.0f, 1.0f); // Rotate around diagonal axis
    axis.normalize();
    Eigen::AngleAxisf rot(angle, axis);

    // Combine transformations
    Eigen::Affine3f transform = trans * rot;

    // Convert Eigen matrix to raylib Matrix
    Eigen::Matrix4f m = transform.matrix();
    transforms[i] = {m(0, 0), m(0, 1), m(0, 2), m(0, 3), m(1, 0), m(1, 1),
                     m(1, 2), m(1, 3), m(2, 0), m(2, 1), m(2, 2), m(2, 3),
                     m(3, 0), m(3, 1), m(3, 2), m(3, 3)};
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
  DrawMeshInstanced(mesh_cube, material_instanced, transforms, MAX_INSTANCES);
  // TODO: draw other things
}

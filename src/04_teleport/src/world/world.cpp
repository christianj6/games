#include "world.h"
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>

#define MAX_INSTANCES 10000

/*#include <Eigen/Dense>*/
// TODO: replace raymath with eigen for matrix stuff

void World::get_initial_world_state() {
  transforms = (Matrix *)RL_CALLOC(MAX_INSTANCES, sizeof(Matrix));

  for (int i = 0; i < MAX_INSTANCES; i++) {
    Matrix translation = MatrixTranslate((float)GetRandomValue(-50, 50),
                                         (float)GetRandomValue(-50, 50),
                                         (float)GetRandomValue(-50, 50));
    Vector3 axis = Vector3Normalize((Vector3){(float)GetRandomValue(0, 360),
                                              (float)GetRandomValue(0, 360),
                                              (float)GetRandomValue(0, 360)});
    float angle = (float)GetRandomValue(0, 180) * DEG2RAD;
    Matrix rotation = MatrixRotate(axis, angle);

    transforms[i] = MatrixMultiply(rotation, translation);
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

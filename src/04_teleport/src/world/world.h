// TODO: slightly larger game world, made possible through voxel-based rendering
// TODO: pseudo voxel world: implement a world state with Matrix, abstractions
// update this
// TODO: rely on eigen for Matrix operations
// TODO: culling out of scope for now; just reproduce the world from shoot w new
// pipeline
// TODO: rely on raylib's mesh instancing to draw them; a reasonable start
// TODO: out of scope: world layout regularly changes like a shifting labyrinth
//

#pragma once
#include "graphics/renderer.h"
#include "raylib.h"

class World {
public:
  World();
  void update(float, Camera);
  void draw();
  Matrix *get_transforms() { return transforms; }

private:
  Renderer renderer;
  Matrix *transforms;
  void get_initial_world_state();
  void configure_materials();
  Mesh mesh_cube;
  Material material_instanced;
  Material material_default;
};

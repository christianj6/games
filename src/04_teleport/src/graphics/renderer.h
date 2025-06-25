#pragma once
#include "raylib.h"
#include "rlights.h"

/*
 * encapsulates rendering utilities like shaders, lighting, etc.
 * used by world.h
 */
class Renderer {
public:
  Renderer();
  void update(Camera camera);
  void draw();
  Shader get_shader() { return shader; }
  void configure_lighting();

private:
  Shader shader;
  Light moving_light;
  Light player_light;
  float moving_light_angle = 0.0f;

  // Red sphere lights
  static const int NUM_SPHERES = 1;
  Vector3 sphere_positions[NUM_SPHERES] = {{42.5f, 21.0f, 42.5f}};
  Light sphere_lights[NUM_SPHERES];
};

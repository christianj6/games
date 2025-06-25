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
  Shader get_shader() { return shader; }
  void configure_lighting();

private:
  Shader shader;
  Light moving_light;
  Light player_light;
  float moving_light_angle = 0.0f;
};

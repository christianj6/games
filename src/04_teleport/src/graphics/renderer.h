#pragma once
#include "raylib.h"

/*
 * encapsulates rendering utilities like shaders, lighting, etc.
 * used by world.h
 */
class Renderer {
public:
  Renderer();
  void update(float[3]);
  Shader get_shader() { return shader; }
  void configure_lighting(Vector3);

private:
  Shader shader;
};

#pragma once
#include "raylib.h"
#include "rlights.h"


class Renderer {
public:
  Renderer();
  void update(Camera camera);
  Shader get_shader() { return shader_; }

private:
  Shader shader_;
};

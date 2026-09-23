#pragma once
#include "raylib.h"

class Renderer {
public:
  Renderer();
  Shader get_shader() const { return shader_; }

private:
  Shader shader_;
};

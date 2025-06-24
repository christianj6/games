#include "raylib.h"
#pragma once

class Game {
public:
  Game();
  void update();
  void draw();

private:
  Camera camera;
  Shader shader;
  Mesh cube;
  Material mat_instances;
  Material mat_default;
  Matrix *transforms;
};

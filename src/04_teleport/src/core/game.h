#include "hud.h"
#include "objects/player.h"
#include "raylib.h"
#pragma once

class Game {
public:
  Game();
  void update();
  void draw();

private:
  Hud hud;
  Player player;
  //
  Shader shader;
  Mesh cube;
  Material mat_instances;
  Material mat_default;
  Matrix *transforms;
};

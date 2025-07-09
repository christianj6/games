#pragma once
#include "raylib.h"

class Chunk {
public:
  Chunk();
  void load();
  void unload();
  void draw();
  bool loaded = false;

private:
  Vector2 position;
};

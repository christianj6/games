#pragma once
#include "raylib.h"

class Chunk {
public:
  Chunk();
  void load();
  void unload();

private:
  bool loaded;
  Vector2 position;
};

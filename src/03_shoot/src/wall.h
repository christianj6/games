#pragma once
#include "raylib.h"


class Wall {
  public:
    Wall(float, Vector3, Color, float);
    void draw();

  private:
    Vector3 position;
    float height;
    float length;
    Color color;
};

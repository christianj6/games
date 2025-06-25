#include "raylib.h"

class Player {
public:
  Player();
  void update(float);
  Vector3 get_position() { return camera.position; }
  Camera get_camera() { return camera; }

private:
  Vector3 position;
  Camera camera;
  const float MOVE_SPEED = 10.0f;
  const float MOUSE_SENSITIVITY = 0.003f;
  void handle_input();
  void jump();
  void blink();
};

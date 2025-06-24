// TODO: teleportation mechanic allows player to blink forwards and on top of
// obstacles
// TODO: jump mechanic which pairs with teleportation

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
};

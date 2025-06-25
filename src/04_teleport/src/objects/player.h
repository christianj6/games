#include "Eigen/Dense"
#include "raylib.h"

class Player {
public:
  Player();
  void update(
      float,
      const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> &);
  Vector3 get_position() { return camera.position; }
  Camera get_camera() { return camera; }

private:
  Vector3 position;
  Camera camera;
  const float MOVE_SPEED = 10.0f;
  const float MOUSE_SENSITIVITY = 0.003f;
  Vector3 try_move();
  void move_camera();
  void jump();
  void blink();
};

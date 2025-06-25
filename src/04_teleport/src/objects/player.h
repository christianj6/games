#include "Eigen/Dense"
#include "movement/controller.h"
#include "raylib.h"
#include <memory>

class Player {
public:
  Player();
  Player(std::unique_ptr<IMovemementController>);
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
  const float JUMP_FORCE = 14.0f;
  const float GRAVITY = -20.0f;
  float vertical_velocity = 0.0f;
  bool is_grounded = true;
  std::unique_ptr<IMovemementController> movement_controller;
  Vector3 try_move();
  void setup_camera();
  void move_camera();
  void jump();
  void blink();
};

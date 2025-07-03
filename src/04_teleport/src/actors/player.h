#include "Eigen/Dense"
#include "movement/controller.h"
#include "raylib.h"
#include <memory>

struct PlayerAction {
  bool attack = false;
  bool blink = false;
};

class Player {
public:
  Player();
  Player(std::unique_ptr<IMovemementController>);
  PlayerAction update(
      float,
      const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> &);
  void draw(
      const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> &);
  Vector3 get_position() { return camera.position; }
  Camera get_camera() { return camera; }
  void decrease_health(int amount) { health -= amount; }
  void reset_health() { health = 100; }
  int check_health() { return health; }
  void reset_position();
  void set_movement_controller(
      std::unique_ptr<IMovemementController> new_controller) {
    movement_controller = std::move(new_controller);
  }

private:
  Vector3 position;
  Camera camera;
  const float PLAY_AREA_SIZE = 85.0f;
  const float MOVE_SPEED = 10.0f;
  const float MOUSE_SENSITIVITY = 0.003f;
  const float JUMP_FORCE = 14.0f;
  const float GRAVITY = -20.0f;
  float vertical_velocity = 0.0f;
  int jumps_remaining = 2;
  bool is_blinking = false;
  Vector3 blink_target;
  const int MAX_JUMPS = 2;
  std::unique_ptr<IMovemementController> movement_controller;
  Vector3 try_move();
  void setup_camera();
  void move_camera();
  void jump();
  void blink();
  bool attack();
  int health = 100;
};

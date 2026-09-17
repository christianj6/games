#pragma once
#include "game.h"

enum class ApplicationState { MAIN_MENU, GAME, QUIT };

class App {
public:
  bool run(bool);
  bool is_game_running() const;
  void toggle_pause(); // web: ESC-exits-pointer-lock is delivered as a pause
private:
  ApplicationState current_state = ApplicationState::MAIN_MENU;
  std::unique_ptr<Game> game_; // built lazily on first ENTER
  void main_menu();
};

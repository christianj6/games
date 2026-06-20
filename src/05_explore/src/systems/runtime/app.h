#pragma once
#include "game.h"

enum class ApplicationState { MAIN_MENU, GAME, QUIT };

class App {
public:
  bool run(bool);

private:
  ApplicationState current_state = ApplicationState::GAME; // TODO: change back to MAIN_MENU
  Game game;
  void main_menu();
};

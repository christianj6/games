#pragma once
#include <vector>
#include <memory>

#include "hud.h"
#include "map/world.h"

#include "actors/actor.h"
#include "actors/player.h"

enum class GameState {RUNNING, PAUSED, WIN, LOSE};

struct GameInfo {

};

struct Blackboard {

};

class Game {
  public:
    Game();
    ~Game();
    GameInfo run(bool);
  private:
    GameState current_state;
    GameInfo update();
    void draw();

    Hud hud;
    World world;
    Player player;
    std::vector<std::unique_ptr<Actor>> actors;
    Blackboard blackboard;
};

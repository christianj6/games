#include "hud.h"
#include "raylib.h"
#include <fmt/core.h>

void Hud::draw(Blackboard blackboard) {
  DrawFPS(20, 40);
  DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 2, BLACK);
  DrawText(fmt::format("{}, {}, {}", blackboard.current_player_position.x,
                       blackboard.current_player_position.y,
                       blackboard.current_player_position.z)
               .c_str(),
           20, 60, 20, BLACK);
}

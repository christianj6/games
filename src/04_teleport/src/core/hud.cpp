#include "hud.h"
#include "fmt/core.h"
#include "raylib.h"
#include <string>

Hud::Hud(bool debug_mode_enabled) : debug_mode_enabled(debug_mode_enabled) {}

void Hud::draw(Vector3 player_position) {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  // reticle
  DrawCircle(screenWidth / 2, screenHeight / 2, 2, WHITE);

  if (debug_mode_enabled) {
    DrawRectangle(10, 10, 300, 133, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(10, 10, 300, 133, BLUE);

    Vector3 &v = player_position;
    std::string player_position_string =
        fmt::format("x={:.2f}, y={:.2f}, z={:.2f}", v.x, v.y, v.z);

    DrawFPS(15, 15);
    DrawText(player_position_string.c_str(), 15, 40, 20, RED);
  }
}

void Hud::update(float dt) {
  // TODO: consider removing the debug_mode sink, leaving only this toggle
  if (IsKeyPressed(KEY_I)) {
    debug_mode_enabled = !debug_mode_enabled;
  }
}

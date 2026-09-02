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

  // Quest tracker
  std::string quest_str;
  switch (blackboard.quest.state) {
  case QuestState::COLLECTING:
    quest_str = fmt::format("Collect shards: {} / {}",
                            blackboard.quest.collected,
                            blackboard.quest.required);
    break;
  case QuestState::TURN_IN:
    quest_str = "Return to your friend!";
    break;
  case QuestState::COMPLETE:
    quest_str = "Quest complete!";
    break;
  }
  if (!quest_str.empty())
    DrawText(quest_str.c_str(), 20, 85, 22, DARKBLUE);

  // Health bar (damage sources arrive in Phase 2)
  int hw = 200, hh = 18, hx = 20, hy = GetScreenHeight() - 38;
  DrawRectangle(hx, hy, hw, hh, {40, 40, 40, 200});
  float frac = blackboard.player_health / 100.0f;
  if (frac < 0.0f) frac = 0.0f;
  if (frac > 1.0f) frac = 1.0f;
  DrawRectangle(hx + 2, hy + 2, (int)((hw - 4) * frac), hh - 4, RED);

  // Interaction prompt
  if (blackboard.friend_nearby && blackboard.quest.state == QuestState::TURN_IN) {
    const char *prompt = "[E] Talk to friend";
    int w = MeasureText(prompt, 24);
    DrawText(prompt, GetScreenWidth() / 2 - w / 2,
             (int)(GetScreenHeight() * 0.72f), 24, BLACK);
  }
}

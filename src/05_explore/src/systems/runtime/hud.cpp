#include "hud.h"
#include "raylib.h"
#include <algorithm>
#include <fmt/core.h>

namespace {
constexpr int kFpsX = 20;
constexpr int kFpsY = 40;
constexpr int kCrosshairRadius = 2;
constexpr int kHudX = 20;
constexpr int kDebugY = 60;
constexpr int kDebugFontSize = 20;
constexpr int kQuestY = 85;
constexpr int kQuestFontSize = 22;
constexpr int kBarWidth = 200;
constexpr int kBarHeight = 18;
constexpr int kBarX = 20;
constexpr int kBarBottomInset = 38;
constexpr int kBarInset = 2;
constexpr Color kBarBackColor = {40, 40, 40, 200};
constexpr int kVignetteMaxAlpha = 120;
constexpr int kVignetteThickness = 60;
constexpr int kActionFontSize = 22;
constexpr int kPromptFontSize = 24;
constexpr float kPromptScreenFraction = 0.72f;
// 0.5 (centre) + 44 px at 1080p: 0.5 + 44 / 1080 = 0.54074 — same position as
// the old fixed offset on desktop, but scales with short browser canvases.
constexpr float kActionPromptScreenFraction = 0.5408f;
} // namespace

void Hud::draw(const Blackboard &blackboard) {
  DrawFPS(kFpsX, kFpsY);
  DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, kCrosshairRadius,
             BLACK);
  DrawText(fmt::format("{}, {}, {}", blackboard.current_player_position.x,
                       blackboard.current_player_position.y,
                       blackboard.current_player_position.z)
               .c_str(),
           kHudX, kDebugY, kDebugFontSize, BLACK);

  // Quest tracker
  std::string quest_str;
  switch (blackboard.quest.state) {
  case QuestState::COLLECTING:
    quest_str =
        fmt::format("Collect shards: {} / {}", blackboard.quest.collected,
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
    DrawText(quest_str.c_str(), kHudX, kQuestY, kQuestFontSize, DARKBLUE);

  // Health bar (damage sources arrive in Phase 2)
  int hy = GetScreenHeight() - kBarBottomInset;
  DrawRectangle(kBarX, hy, kBarWidth, kBarHeight, kBarBackColor);
  float frac =
      std::clamp(blackboard.player_health / kMaxPlayerHealth, 0.0f, 1.0f);
  DrawRectangle(kBarX + kBarInset, hy + kBarInset,
                (int)((kBarWidth - 2 * kBarInset) * frac),
                kBarHeight - 2 * kBarInset, RED);

  // Damage vignette
  if (blackboard.damage_flash > 0.0f) {
    Color c = RED;
    c.a = (unsigned char)(blackboard.damage_flash * kVignetteMaxAlpha);
    int w = GetScreenWidth(), h = GetScreenHeight();
    DrawRectangle(0, 0, w, kVignetteThickness, c);
    DrawRectangle(0, h - kVignetteThickness, w, kVignetteThickness, c);
    DrawRectangle(0, 0, kVignetteThickness, h, c);
    DrawRectangle(w - kVignetteThickness, 0, kVignetteThickness, h, c);
  }

  // Contextual takedown / strike prompt
  if (blackboard.takedown_available || blackboard.attack_available) {
    const char *action =
        blackboard.takedown_available ? "[LMB] Takedown" : "[LMB] Strike";
    int aw = MeasureText(action, kActionFontSize);
    DrawText(action, GetScreenWidth() / 2 - aw / 2,
             (int)(GetScreenHeight() * kActionPromptScreenFraction),
             kActionFontSize, blackboard.takedown_available ? SKYBLUE : MAROON);
  }

  if (blackboard.friend_nearby &&
      blackboard.quest.state == QuestState::TURN_IN) {
    const char *prompt = "[E] Talk to friend";
    int w = MeasureText(prompt, kPromptFontSize);
    DrawText(prompt, GetScreenWidth() / 2 - w / 2,
             (int)(GetScreenHeight() * kPromptScreenFraction), kPromptFontSize,
             BLACK);
  }
}

#pragma once
#include "raylib.h"
#include <array>

enum class Sfx {
  Blink, Anchor, Recall, Pickup, TurnIn, Win, Lose, Shot, Hit, Kill,
  Takedown, Revive, Alert, Step, Count
};

// Process-wide audio: device, loaded sounds, ambient wind loop.
// Singleton because it must outlive individual Game instances (restart flow).
class Audio {
public:
  static Audio &get();

  void init();         // device + loads; idempotent
  void stop_ambient(); // stop the wind loop (menu / shutdown)
  void play(Sfx id, float volume = 1.0f);
  // Volume attenuates by distance from the listener; pan by the source's
  // position relative to the listener's right vector (pseudo spatial).
  void play_at(Sfx id, Vector3 listener, Vector3 source, Vector3 right,
               float max_dist = 60.0f, float volume = 1.0f);
  void update(); // pump the ambient music stream

private:
  Audio() = default;
  std::array<Sound, (size_t)Sfx::Count> sounds_{};
  Music wind_{};
  bool initialized_ = false;
};

#include "audio.h"
#include "raymath.h"

Audio &Audio::get() {
  static Audio instance;
  return instance;
}

void Audio::init() {
  if (initialized_)
    return;
  InitAudioDevice();
  if (!IsAudioDeviceReady())
    return;

  static const char *names[] = {"blink",   "anchor", "recall", "pickup",
                                "turnin",  "win",    "lose",   "shot",
                                "hit",     "kill",   "takedown", "revive",
                                "alert",   "step"};
  for (int i = 0; i < (int)Sfx::Count; ++i)
    sounds_[i] = LoadSound(TextFormat("audio/%s.wav", names[i]));

  wind_ = LoadMusicStream("audio/wind.wav");
  wind_.looping = true;
  SetMusicVolume(wind_, 0.08f);
  initialized_ = true;
}

void Audio::stop_ambient() {
  if (initialized_)
    StopMusicStream(wind_);
}

void Audio::play(Sfx id, float volume) {
  if (!initialized_)
    return;
  Sound &s = sounds_[(int)id];
  SetSoundVolume(s, volume);
  PlaySound(s);
}

void Audio::play_at(Sfx id, Vector3 listener, Vector3 source, Vector3 right,
                    float max_dist, float volume) {
  Vector3 offset = Vector3Subtract(source, listener);
  float d = Vector3Length(offset);
  float att = 1.0f - d / max_dist;
  if (att <= 0.0f)
    return;
  Vector3 dir = d > 0.001f ? Vector3Scale(offset, 1.0f / d)
                           : Vector3{0.0f, 0.0f, 1.0f};
  // raylib pan: -1.0 left, 0.0 center, 1.0 right
  float pan = Vector3DotProduct(dir, right);
  Sound &s = sounds_[(int)id];
  SetSoundVolume(s, volume * att);
  SetSoundPan(s, fmaxf(-1.0f, fminf(1.0f, pan)));
  PlaySound(s);
}

void Audio::update() {
  if (!initialized_)
    return;
  UpdateMusicStream(wind_);
  if (!IsMusicStreamPlaying(wind_))
    PlayMusicStream(wind_);
}

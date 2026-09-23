#include "renderer.h"
#include "utils/resources.h"

#ifdef PLATFORM_WEB
#define GLSL_VERSION 300
#else
#define GLSL_VERSION 330
#endif

namespace {
// NOTE: the shader divides ambient by 10, so 2.5 => effective 0.25.
// The old 0.2 left unlit faces at 0.02 — effectively black.
constexpr float kAmbient[4] = {2.5f, 2.5f, 2.4f, 1.0f};

// Key "sun": directional, warm, from the south-east sky. Directional lights
// hit every chunk regardless of position, so no part of the world is
// ambient-only anymore.
constexpr Vector3 kSunDirection{-0.45f, -0.85f, -0.25f};
constexpr Color kSunColor{255, 246, 220, 255};

// Fill: cool light from the opposite azimuth so no face orientation ends up
// fully unlit.
constexpr Vector3 kFillDirection{0.55f, -0.35f, -0.40f};
constexpr Color kFillColor{135, 150, 185, 255};

// Purple point accent parked at the corner shared by chunks (0,0) and (1,1),
// not above the home chunk's centre (16, 16) — mood lighting only.
constexpr Vector3 kAccentPosition{32.0f, 14.0f, 32.0f};
constexpr Vector3 kAccentTarget{32.0f, 0.0f, 32.0f};
constexpr Color kAccentColor{112, 36, 138, 255};

// Upload a light straight into the lighting shader's lights[] array.
// Bypasses rlights.h CreateLight, whose internal static counter silently
// drops lights once a second Game (and shader) is constructed on restart.
void set_light(Shader shader, int index, int type, Vector3 position,
               Vector3 target, Color color) {
  int enabled = 1;
  SetShaderValue(
      shader,
      GetShaderLocation(shader, TextFormat("lights[%i].enabled", index)),
      &enabled, SHADER_UNIFORM_INT);
  SetShaderValue(
      shader, GetShaderLocation(shader, TextFormat("lights[%i].type", index)),
      &type, SHADER_UNIFORM_INT);
  float pos[3] = {position.x, position.y, position.z};
  SetShaderValue(
      shader,
      GetShaderLocation(shader, TextFormat("lights[%i].position", index)), pos,
      SHADER_UNIFORM_VEC3);
  float tgt[3] = {target.x, target.y, target.z};
  SetShaderValue(
      shader, GetShaderLocation(shader, TextFormat("lights[%i].target", index)),
      tgt, SHADER_UNIFORM_VEC3);
  float col[4] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f,
                  color.a / 255.0f};
  SetShaderValue(
      shader, GetShaderLocation(shader, TextFormat("lights[%i].color", index)),
      col, SHADER_UNIFORM_VEC4);
}
} // namespace

Renderer::Renderer() {
  SearchAndSetResourceDir("resources");
  shader_ = LoadShader(TextFormat("shaders/glsl%i/lighting.vs", GLSL_VERSION),
                       TextFormat("shaders/glsl%i/lighting.fs", GLSL_VERSION));

  // Get shader_ locations
  shader_.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader_, "mvp");
  shader_.locs[SHADER_LOC_MATRIX_MODEL] =
      GetShaderLocation(shader_, "matModel");
  shader_.locs[SHADER_LOC_MATRIX_NORMAL] =
      GetShaderLocation(shader_, "matNormal");

  int ambientLoc = GetShaderLocation(shader_, "ambient");
  SetShaderValue(shader_, ambientLoc, kAmbient, SHADER_UNIFORM_VEC4);

  set_light(shader_, 0, 0 /*directional*/, Vector3{0.0f, 0.0f, 0.0f},
            kSunDirection, kSunColor);

  set_light(shader_, 1, 0 /*directional*/, Vector3{0.0f, 0.0f, 0.0f},
            kFillDirection, kFillColor);

  set_light(shader_, 2, 1 /*point*/, kAccentPosition, kAccentTarget,
            kAccentColor);
}

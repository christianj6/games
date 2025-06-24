#include "renderer.h"
#include "utils/resource_dir.h"

#define RLIGHTS_IMPLEMENTATION
#include "raymath.h"
#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

Renderer::Renderer() {
  SearchAndSetResourceDir("resources");
  shader = LoadShader(TextFormat("shaders/glsl%i/lighting.vs", GLSL_VERSION),
                      TextFormat("shaders/glsl%i/lighting.fs", GLSL_VERSION));

  // Get shader locations
  shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
  shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
  shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
  shader.locs[SHADER_LOC_MATRIX_NORMAL] =
      GetShaderLocation(shader, "matNormal");

  // Set ambient light
  int ambientLoc = GetShaderLocation(shader, "ambient");
  SetShaderValue(shader, ambientLoc, (float[4]){0.2f, 0.2f, 0.2f, 1.0f},
                 SHADER_UNIFORM_VEC4);
}

void Renderer::update(float player_camera_position[3]) {
  SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW],
                 player_camera_position, SHADER_UNIFORM_VEC3);
}

void Renderer::configure_lighting() {
  // TODO: extract light positions to world
  // Main directional light from top-front
  Light light1 = CreateLight(LIGHT_DIRECTIONAL, (Vector3){1.0f, 1.0f, -1.0f},
                             Vector3Zero(), WHITE, shader);

  // Warm fill light from left
  Light light2 =
      CreateLight(LIGHT_DIRECTIONAL, (Vector3){-2.0f, 0.5f, 0.0f},
                  Vector3Zero(), (Color){255, 200, 150, 255}, shader);

  // Cool rim light from back-right
  Light light3 =
      CreateLight(LIGHT_DIRECTIONAL, (Vector3){1.0f, 0.5f, 2.0f}, Vector3Zero(),
                  (Color){150, 180, 255, 255}, shader);

  UpdateLightValues(shader, light1);
  UpdateLightValues(shader, light2);
  UpdateLightValues(shader, light3);
}

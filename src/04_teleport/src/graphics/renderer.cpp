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
  // Main ambient directional light from above
  Light light1 =
      CreateLight(LIGHT_DIRECTIONAL, (Vector3){0.0f, 1.0f, 0.0f}, Vector3Zero(),
                  (Color){180, 180, 180, 255}, shader);

  // Point lights spread around the space
  Light light2 = CreateLight(LIGHT_POINT, (Vector3){50.0f, 5.0f, 50.0f},
                             (Vector3){0.0f, 0.0f, 0.0f},
                             (Color){255, 100, 100, 255}, shader);

  Light light3 = CreateLight(LIGHT_POINT, (Vector3){-50.0f, 5.0f, -50.0f},
                             (Vector3){0.0f, 0.0f, 0.0f},
                             (Color){100, 255, 100, 255}, shader);

  Light light4 = CreateLight(LIGHT_POINT, (Vector3){50.0f, 5.0f, -50.0f},
                             (Vector3){0.0f, 0.0f, 0.0f},
                             (Color){100, 100, 255, 255}, shader);

  Light light5 = CreateLight(LIGHT_POINT, (Vector3){-50.0f, 5.0f, 50.0f},
                             (Vector3){0.0f, 0.0f, 0.0f},
                             (Color){255, 255, 100, 255}, shader);

  Light light6 = CreateLight(LIGHT_POINT, (Vector3){0.0f, 5.0f, 0.0f},
                             (Vector3){0.0f, 0.0f, 0.0f},
                             (Color){255, 100, 255, 255}, shader);

  UpdateLightValues(shader, light1);
  UpdateLightValues(shader, light2);
  UpdateLightValues(shader, light3);
  UpdateLightValues(shader, light4);
  UpdateLightValues(shader, light5);
  UpdateLightValues(shader, light6);
}

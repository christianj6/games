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
  shader = LoadShader(
      TextFormat("shaders/glsl%i/lighting_instancing.vs", GLSL_VERSION),
      TextFormat("shaders/glsl%i/lighting.fs", GLSL_VERSION));

  shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
  shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

  int ambientLoc = GetShaderLocation(shader, "ambient");
  SetShaderValue(shader, ambientLoc, (float[4]){0.2f, 0.2f, 0.2f, 1.0f},
                 SHADER_UNIFORM_VEC4);
}

void Renderer::update(float player_camera_position[3]) {
  SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW],
                 player_camera_position, SHADER_UNIFORM_VEC3);
}

void Renderer::configure_lighting(Vector3 light_position) {
  CreateLight(LIGHT_DIRECTIONAL, light_position, Vector3Zero(), WHITE, shader);
}

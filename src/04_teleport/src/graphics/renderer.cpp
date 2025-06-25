#include "renderer.h"
#include "raymath.h"
#include "utils/resource_dir.h"

#define RLIGHTS_IMPLEMENTATION
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

void Renderer::update(Camera camera) {
  float position[3] = {camera.position.x, camera.position.y, camera.position.z};
  SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], position,
                 SHADER_UNIFORM_VEC3);
}

void Renderer::draw() {
  // Draw red spheres
  for (int i = 0; i < NUM_SPHERES; i++) {
    DrawSphereEx(sphere_positions[i], 1.0f, 8, 8, RED);
  }
}

void Renderer::configure_lighting() {
  const float AREA_SIZE = 85.0f;
  const float CORNER_HEIGHT = 0.0f;

  // Initialize red sphere lights
  for (int i = 0; i < NUM_SPHERES; i++) {
    sphere_lights[i] =
        CreateLight(LIGHT_POINT, sphere_positions[i], Vector3Zero(),
                    (Color){255, 50, 50, 255}, shader);
  }

  CreateLight(LIGHT_POINT, (Vector3){0, CORNER_HEIGHT, 0}, Vector3Zero(),
              DARKPURPLE, shader);

  CreateLight(LIGHT_POINT, (Vector3){AREA_SIZE, CORNER_HEIGHT, AREA_SIZE},
              Vector3Zero(), DARKPURPLE, shader);

  CreateLight(LIGHT_POINT, (Vector3){0, CORNER_HEIGHT, AREA_SIZE},
              Vector3Zero(), DARKPURPLE, shader);

  CreateLight(LIGHT_POINT, (Vector3){AREA_SIZE, CORNER_HEIGHT, 0},
              Vector3Zero(), DARKPURPLE, shader);
}

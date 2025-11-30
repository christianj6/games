#include "renderer.h"
#include "utils/resources.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#define GLSL_VERSION 330

Renderer::Renderer() {
  SearchAndSetResourceDir("resources");
  shader_ = LoadShader(
      TextFormat("shaders/glsl%i/lighting.vs", GLSL_VERSION),
      TextFormat("shaders/glsl%i/lighting.fs", GLSL_VERSION)
  );

  // Get shader_ locations
  shader_.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader_, "mvp");
  shader_.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader_, "viewPos");
  shader_.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader_, "matModel");
  shader_.locs[SHADER_LOC_MATRIX_NORMAL] = GetShaderLocation(shader_, "matNormal");

  // Set ambient light
  int ambientLoc = GetShaderLocation(shader_, "ambient");
  float shader__values[4] = {0.2f, 0.2f, 0.2f, 1.0f};
  SetShaderValue(shader_, ambientLoc, shader__values, SHADER_UNIFORM_VEC4);
}

void Renderer::update(Camera camera) {
  float position[3] = {camera.position.x, camera.position.y, camera.position.z};
  SetShaderValue(shader_, shader_.locs[SHADER_LOC_VECTOR_VIEW], position, SHADER_UNIFORM_VEC3);
}

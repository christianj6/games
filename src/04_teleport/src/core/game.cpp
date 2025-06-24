#include "raylib.h"
#include "raymath.h"

#include "game.h"
#include "utils/resource_dir.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include <stdlib.h> // Required for: calloc(), free()

/*#include <Eigen/Dense>*/

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

#define MAX_INSTANCES 10000
Game::Game() {
  // Define the camera to look into our 3d world
  camera.position = (Vector3){-125.0f, 125.0f, -125.0f}; // Camera position
  camera.target = (Vector3){0.0f, 0.0f, 0.0f}; // Camera looking at point
  camera.up =
      (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 45.0f;             // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE; // Camera projection type

  // Define mesh to be instanced
  cube = GenMeshCube(1.0f, 1.0f, 1.0f);

  // Define transforms to be uploaded to GPU for instances
  transforms = (Matrix *)RL_CALLOC(
      MAX_INSTANCES,
      sizeof(Matrix)); // Pre-multiplied transformations passed to rlgl

  // Translate and rotate cubes randomly
  for (int i = 0; i < MAX_INSTANCES; i++) {
    Matrix translation = MatrixTranslate((float)GetRandomValue(-50, 50),
                                         (float)GetRandomValue(-50, 50),
                                         (float)GetRandomValue(-50, 50));
    Vector3 axis = Vector3Normalize((Vector3){(float)GetRandomValue(0, 360),
                                              (float)GetRandomValue(0, 360),
                                              (float)GetRandomValue(0, 360)});
    float angle = (float)GetRandomValue(0, 180) * DEG2RAD;
    Matrix rotation = MatrixRotate(axis, angle);

    transforms[i] = MatrixMultiply(rotation, translation);
  }
  SearchAndSetResourceDir("resources");

  // Load lighting shader
  shader = LoadShader(
      TextFormat("shaders/glsl%i/lighting_instancing.vs", GLSL_VERSION),
      TextFormat("shaders/glsl%i/lighting.fs", GLSL_VERSION));
  // Get shader locations
  shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
  shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

  // Set shader value: ambient light level
  int ambientLoc = GetShaderLocation(shader, "ambient");
  SetShaderValue(shader, ambientLoc, (float[4]){0.2f, 0.2f, 0.2f, 1.0f},
                 SHADER_UNIFORM_VEC4);

  // Create one light
  CreateLight(LIGHT_DIRECTIONAL, (Vector3){50.0f, 50.0f, 0.0f}, Vector3Zero(),
              WHITE, shader);

  // NOTE: We are assigning the intancing shader to material.shader
  // to be used on mesh drawing with DrawMeshInstanced()
  mat_instances = LoadMaterialDefault();
  mat_instances.shader = shader;
  mat_instances.maps[MATERIAL_MAP_DIFFUSE].color = RED;

  // Load default material (using raylib intenral default shader) for
  // non-instanced mesh drawing WARNING: Default shader enables vertex color
  // attribute BUT GenMeshCube() does not generate vertex colors, so, when
  // drawing the color attribute is disabled and a default color value is
  // provided as input for thevertex attribute
  mat_default = LoadMaterialDefault();
  mat_default.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
}
void Game::update() {
  UpdateCamera(&camera, CAMERA_ORBITAL);

  // Update the light shader with the camera view position
  float cameraPos[3] = {camera.position.x, camera.position.y,
                        camera.position.z};
  SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos,
                 SHADER_UNIFORM_VEC3);
}

void Game::draw() {
  BeginDrawing();

  ClearBackground(RAYWHITE);

  BeginMode3D(camera);

  // Draw cube mesh with default material (BLUE)
  DrawMesh(cube, mat_default, MatrixTranslate(-10.0f, 0.0f, 0.0f));

  // Draw meshes instanced using material containing instancing shader (RED +
  // lighting), transforms[] for the instances should be provided, they are
  // dynamically updated in GPU every frame, so we can animate the different
  // mesh instances
  DrawMeshInstanced(cube, mat_instances, transforms, MAX_INSTANCES);

  // Draw cube mesh with default material (BLUE)
  DrawMesh(cube, mat_default, MatrixTranslate(10.0f, 0.0f, 0.0f));

  EndMode3D();

  DrawFPS(GetScreenWidth() / 2, GetScreenHeight() / 2);

  EndDrawing();
}

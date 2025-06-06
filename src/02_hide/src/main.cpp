#include "player.h"
#include "raylib.h"
#include "world.h"

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Hide");

  World world;
  world.add_object(std::make_unique<Player>(Vector2{640, 400}));

  while (!WindowShouldClose()) { 
    float dt = GetFrameTime();   

    BeginDrawing();         
    ClearBackground(BLACK); 

    world.update(dt); 
    world.draw();    

    EndDrawing(); 
  }

  CloseWindow(); 
  return 0;     
}

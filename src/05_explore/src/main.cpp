// TODO: nonfunctional goals
/*
 * clean architecture
 * better performance than teleport
 * reduce invest in additional tooling, in order to
 * focus more on core game implementation
 * make it silly; have fun with it
 */

// TODO: functional goals
/*
 * larger game world through mesh chunking
 * limited use of kenney assets
 * sounds
 * limited communication from NPCs, anticipating brownies
 */

// TODO: game loop
/*
 * "tower defense with light exploration"
 * NPCs roam around a specified area
 * player can "chat" with NPCs in a limited, scripted way
 * enemies attack in waves according to a timer
 * between waves, NPCs periodically request assets which are used to attack
 * enemies player must explore the open world to find the assets, while avoiding
 * enemies assets can be used to create traps, or NPCs throw/shoot them player
 * can attack enemies by getting close but it is very risky NPCs can heal the
 * player between waves if all NPCs die you lose if make it through all the
 * waves you win
 */

// TODO: expanded teleportation
/*
 * right-click and hold to teleport as before
 * quick tapping right-click teleports back to previous location
 * left-click while holding places a spot for teleportation
 * teleportation depends on mana which is dropped by slain enemies
 */

// TODO: feedback from teleport
/*
 * too dark
 * movement does not feel nice
 * health bar is better than percentage
 */

// TODO: notes
/*
 * consider renaming to "defend"
 */
#include "raylib.h"

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  SetTargetFPS(60);
  InitWindow(400, 200, "Explore");
  DisableCursor();

  // TODO: app run call

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();
  }
  CloseWindow();

  return 0;
}

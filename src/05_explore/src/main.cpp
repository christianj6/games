// TODO: world and movement
/*
 * simplified voxel mesh rendering strategy
 * world is generally structured the same (pillars)
 * pillars are larger and better placed so it is easier to jump between them
 * player camera with improved game feel
 * teleportation mechanic from teleport
 * world chunking based on player position
 * home chunk defined in loaded json; has a more "specific" shape
 * feedback: too dark, movement does not feel nice
 * opti
 * checkpoint: larger game world with chunking and nice player movement
 */

// TODO: friends and home
/*
 * start working with meshes to make the home area look a bit more interesting
 * friend actors with limited animations and behavior trees
 * use mixamo and blender for a simple animation workflow
 * friend ai can be puppeteered by director ai
 * player can talk to friends with a simple text-based system
 * checkpoint: friends roam around the nicer looking home area
 */

// TODO: quests
/*
 * director ai can trigger friends to request items
 * if friend needs something they get an exclamation mark above their head
 * if player talks with friend while in this state, they will give the player a
 * quest requested item is placed in the world and shown in the hud player can
 * search the open world for the item, collect it, and bring it back checkpoint:
 * friends periodically request items, quests are managed, and player can
 * complete
 */

// TODO: enemies
/*
 * enemy actors with limited animations and behavior trees
 * enemy behavior tree for stealth gameplay
 * enemies can be made by the director to just stay in a 'guard' state
 * enemy detection works with a timer shown on the enemy like dishonored
 * enemies spawn around quest items to add challenge
 * player can kill enemies by getting close and clicking a kill button
 * enemies can kill player by shooting; game over state
 * feedback: player health bar
 * checkpoint: enemies added to world to make quests more interesting /
 * challenging
 */

// TODO: finish game loop
/*
 * when you have completed a quest for all friends, you win
 * simple narrative using text communication system
 * playtest and balance, refine
 * checkpoint: core game loop works and is kinda fun
 */

// TODO: polish
/*
 * add sounds
 * friends are interesting and fun
 * enemies are challenging and interesting
 * world is interesting to explore
 * game is performant
 * code is clean
 * checkpoint: game is finished
 * ensure ready for kenney game jam
 * stretch: figure out wasm build
 * stretch: expand teleportation mechanics
 */

#include "raylib.h"
#include "systems/runtime/app.h"

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

  // Initialize with a windowed mode first
  InitWindow(1920, 1080, "Explore");

  // Then toggle to fullscreen - more reliable on macOS than FLAG_FULLSCREEN_MODE
  ToggleFullscreen();
  DisableCursor();

  App app;
  while (app.run(false))
    ;
  CloseWindow();

  return 0;
}

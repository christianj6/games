// TODO: polish check up
/*
 * pillars should be lit from multiple sides when you turn around to look at them it looks janky
 * generally the blink mechanics need thorough play-testing to identify pain points
 * maybe increase the landing squash
 * some blink scenarios can still be a bit tricky so a few more tweaks are welcome
 */

// TODO: friends and home
/*
 * start working with meshes to make the home area look a bit more interesting
 * friend actors with limited animations and behavior trees
 * friends wander around the home chunk, not running into things, and e.g.
 * randomly interact with some objects use mixamo and blender for a simple
 * animation workflow friend ai can be puppeteered by director ai player can
 * talk to friends with a simple text-based system checkpoint: friends roam
 * around the nicer looking home area
 */

// TODO: quests
/*
 * director ai can trigger friends to request items
 * if friend needs something they get an exclamation mark above their head
 * if player interacts with friend while in this state, they will give the
 * player a quest requested item is placed in the world and shown in the hud
 * player can search the open world for the item, collect it, and bring it back
 * checkpoint: friends periodically request items, quests are managed, and
 * player can complete these quests by getting items in the world
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
 * core idea: you go out and get stuff to satisfy the friends,
 * with the goal of ultimately getting all the stuff. when you go out,
 * narrative: you are getting ingredients for a recipe
 * you engage in mini stealth/combat encounters with enemies
 * when you have completed a quest for all friends, you win
 * simple narrative using text communication system
 * playtest and balance, refine
 * checkpoint: core game loop works and is kinda fun
 */

// TODO: polish
/*
 * add sounds
 * camera and movement: better game feel
 * feedback: too dark, movement does not feel nice
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

// TODO: clean up
/*
* change back to main menu starting in app.h
*/

#include "raylib.h"
#include "systems/runtime/app.h"

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

  // Initialize with a windowed mode first
  InitWindow(1920, 1080, "Explore");

  // Then toggle to fullscreen - more reliable on macOS than
  // FLAG_FULLSCREEN_MODE
  ToggleFullscreen();
  DisableCursor();

  App app;
  while (app.run(false))
    ;
  CloseWindow();

  return 0;
}

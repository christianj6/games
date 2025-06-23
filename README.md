### Games
Practicing C++ with Raylib.

***

#### Building Projects
To build any of the projects in the src/ directory, simply ```cd``` into that project directory and run the following:

For Premake:
```
cd build
./premake5.osx gmake2

cd ..
make
```

For CMake (ensure CMake and Ninja are installed first):
```
mkdir -p build && cd build 
cmake .. -G Ninja
ninja
```

***

#### 00_jump
Simple sidescrolling jumper. First experiences with Raylib, C++ syntax, and working with classes.
Build System: Premake

***

#### 01_collect
Top-down coin collecting game with an enemy that chases the player. Getting more comfortable with C++ and Raylib.
Build System: Premake

***

#### 02_hide
Top-down game where player must hide from a searching enemy. Working with pointers and better game abstractions.
Build System: Premake

***

#### 03_shoot
First-person shooter with patrolling enemies. Continuing to refine experience with game abstractions. Change build system.
Build System: CMake

***

#### 04_teleport
Larger game world with teleportation mechanic. Polishing game development and C++ capabilities, project organization, and Web Assembly build for browser support.
Build System: CMake

***

#### 05_explore
Even larger game world with exploration mechanics and more elaborate game state management. Continuing to polish "professional" C++ abilities and application of best practices.
Build System: CMake

***

#### 06_mingle
Experimenting with NPC interactions via llama.cpp and multithreading. Expanding skills with performance and code organization while scoping larger project.
Build System: CMake

***

#### To Do 
- [x] Jump
- [x] Collect
- [x] Hide
- [x] Shoot
- [ ] Teleport
- [ ] Explore
- [ ] Mingle
- [ ] Add pictures of each game to readme; instruction card to each game, etc.
- [ ] Tidy, finish, plan larger project

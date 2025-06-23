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

Note: Builds from 04_teleport and beyond require vcpkg located in the project superior directory.

***

#### 00_jump
Simple sidescrolling jumper. First experiences with Raylib, C++ syntax, and working with classes.

Build System: Premake

![img](./img/00_jump.png)

***

#### 01_collect
Top-down coin collecting game with an enemy that chases the player. Getting more comfortable with C++ and Raylib.

Build System: Premake

![img](./img/01_collect.png)

***

#### 02_hide
Top-down game where player must hide from a searching enemy. Working with pointers and better game abstractions.

Build System: Premake

![img](./img/02_hide.png)

***

#### 03_shoot
First-person shooter with patrolling enemies. Continuing to refine experience with game abstractions. Change build system.

Build System: CMake

![img](./img/03_shoot.png)

***

#### 04_teleport
Larger game world with teleportation mechanic. Polishing game development and C++ capabilities, project organization, and Web Assembly build for browser support.
Working with additional packages and development tools.

Build System: CMake

***

#### 05_explore
Even larger game world with exploration mechanics and more elaborate game state management. Continuing to polish "professional" C++ abilities and application of best practices. 
Itch.io publish and preparation for a first game jam.

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
- [ ] Tidy, finish, plan larger project
    - [ ] Add instruction card to each game

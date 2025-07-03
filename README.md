### Games
Practicing C++ with Raylib.

***

#### Dependencies
Unix
```
clang-format
cmake 
ninja-build
pkg-config
zip
libx11-dev libxcursor-dev libxrandr-dev libxi-dev libgl1-mesa-dev libxinerama-dev libglu1-mesa-dev
```

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

Build System: Premake, Make

![img](./img/00_jump.png)

***

#### 01_collect
Top-down coin collecting game with an enemy that chases the player. Getting more comfortable with C++ and Raylib.

Build System: Premake, Make

![img](./img/01_collect.png)

***

#### 02_hide
Top-down game where player must hide from a searching enemy. Working with pointers and better game abstractions.

Build System: Premake, Make

![img](./img/02_hide.png)

***

#### 03_shoot
First-person shooter with patrolling enemies. Continuing to refine experience with game abstractions. Change build system.

Build System: CMake, Ninja

![img](./img/03_shoot.png)

***

#### 04_teleport
Larger game world with teleportation mechanic. Refining C++ capabilities, game programming, and project organization.
Working with additional packages and development tools. First experiences with shaders, meshes, and lighting.

Build System: vcpkg, CMake, Ninja

![img](./img/04_teleport.png)

***

#### 05_explore
Even larger game world with exploration mechanics and more elaborate game state management. Continuing to polish "professional" C++ abilities and application of best practices. 
Itch.io publish and preparation for a first game jam.

Build System: vcpkg, CMake, Ninja

***

#### 06_mingle
Experimenting with NPC interactions via llama.cpp and multithreading. Expanding skills with performance and code organization while scoping larger project.

Build System: vcpkg, CMake, Ninja

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

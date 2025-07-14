### Games
Practicing C++ with Raylib.

***

#### Installing Dependencies
Unix
```
sudo apt install clang-format cmake ninja-build pkg-config zip
```

Windows 
```
winget install Ninja-build.Ninja
```
[Visual Studio C++ Build Tools](https://visualstudio.microsoft.com/downloads/)

***

#### Building Projects
To build any of the projects in the src/ directory, run the ```build.sh``` script of that project from the project root dir; for example:
```
bash ./src/02_hide/build.sh
```

Later projects also support builds on Windows with a .bat file:
```
.\src\04_teleport\build.bat
```

Additional Points
- Builds from 04_teleport and beyond require vcpkg located in the project superior directory. For more information on setting up vcpkg, check [this resource](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell).
- Earlier projects also support builds on MacOS, as they are based on the [Raylib Quickstart](https://github.com/raylib-extras/raylib-quickstart).

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

#### 06_jam
Kenney Jam 2025.

Build System: vcpkg, CMake, Ninja

***

#### To Do 
- [x] Jump
- [x] Collect
- [x] Hide
- [x] Shoot
- [x] Teleport
- [ ] Explore
- [ ] Jam
- [ ] Checkpoint, tidy, publish.
- [ ] Mingle
- [ ] Defend

***

#### Rules 
- Each new project must be better than the previous.
- Game jams are allowed to be less polished.

***

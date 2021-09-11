AirForce3D Engine (https://github.com/Sheph/af3d)
=========================

![bsd license][img_license]

[img_license]: https://img.shields.io/badge/license-BSD-lightgrey.svg

+ [1. About](#1-about)
+ [2. Features](#2-features)
+ [3. Status](#3-status)
+ [4. Build for windows](#4-build-for-windows)
+ [5. Build for linux](#5-build-for-linux)
+ [6. Special keys](#6-special-keys)

### 1. About
-------------------

AirForce3D is a C++ game engine and built-in level editor:

<img src="https://github.com/Sheph/af3d/blob/master/screenshot2.jpg?raw=true"/><img src="https://github.com/Sheph/af3d/blob/master/screenshot3.jpg?raw=true"/><img src="https://github.com/Sheph/af3d/blob/master/screenshot4.jpg?raw=true"/><img src="https://github.com/Sheph/af3d/blob/master/screenshot5.jpg?raw=true"/>

Check out short AirForce3D engine showreel on youtube:

[![AirForce3D engine showreel](https://github.com/Sheph/af3d/blob/master/screenshot1.jpg)](https://www.youtube.com/watch?v=qPNcl_PSg14)

### 2. Features
-------------------

* Core functionaity
  * Modern C++ (C++11) code base
  * AClass / AObject / AProperty - meta object system with reflection
  * 3D models: FBX
  * Textures: PNG, TGA, HDR, DDS, etc.
  * Instancing: ability to embed a saved scene into another scene
  * Automatic 3D model importer: ablity to split single FBX file into series of scene objects, meshes, etc.
  * Windows and Linux support
* Graphics
  * OpenGL 4.3 renderer
  * Variety of shading models: Unlit, Classic blinn-phong, Modern PBR
  * Anti-aliasing modes: FXAA, TAA
  * Post-processing: Tone mapping, Bloom, Motion Blur, SSAO
  * Clustered-forward shading using compute shaders
  * Lights: Point, Directional, Spot
  * Dynamic shadows: CSM
  * Global illumination: light probes / reflection probes. Spherical and Box models
  * Skyboxes
  * Multiple cameras, multiple render targets
* Lua scripting
  * Ability to create, inspect, modify the scene, object, lights, physics, etc.
  * Easily extensible with new bindings, thanks to luabind
  * LuaJIT support
* Bullet physics integration
  * Supports static, kinematic and dynamic bodies
  * Collision shapes: box, capsule, cone, cylinder, plane, sphere, static mesh, convex mesh
  * Joint types: point to point, hinge, cone twist, slider, 6dof
  * Custom collision filters, collision matrix
  * Sensors
  * Scripting integration
  * Ability to set mass, friction, restitution, dampings, gravity, etc.
  * Debug draw
* Built-in editor
  * Command history, undo / redo stack
  * Transform, rotate, scale modifiers
  * Manipulation modes: scene objects, visuals, lights, collisions, joints
  * Entity creation, removal, duplication, etc.
  * Play mode: test your scene without leaving the editor
  * Extensible via AClass / AObject meta object system

### 3. Status
-------------------

This was originally my hobby project, mostly for learning purposes in the area of 3D graphics and engines. It's still very
incomplete, it lacks a lot of functionality and it's not well-optimized yet, but it's getting there :)

Note! Currently this project is on hold as I don't have enough free time to work on it. But hopefully I'll be able
to get back to it someday...

### 4. Build for windows
-------------------

#### Install LFS

This project uses Git Large Files extension. You need to install lfs if it is not already installed.
Do:

<pre>
git lfs install
</pre>

And make sure all large files are downloaded.

#### Install CMake

Download latest stable (not RC) .exe installer at http://www.cmake.org/download/

Run it, choose "Add CMake to the system PATH for all users", accept other defaults.

#### Install Visual Studio Community 2019

#### Generate visual studio solution

Open cmd shell (i.e. win key + R, type "cmd"), inside the shell run:

<pre>
cmake_Win32_VS2019.bat
</pre>

If everything was done right, you'll see CMake generating the solution and output:

<pre>
-- Build files have been written to: C:/Projects/af3d/build_Win32_VS2019
</pre>

#### Build and run the intro scene

Open visual studio if you haven't done so already, open
C:\Projects\af3d\build_Win32_VS2019\AF3D.sln solution.

* Set "af3d" project as active project, i.e. right-click "af3d" project and select "Set as StartUp project"
* Open "af3d" project properties, i.e. right-click "af3d" project and select "Properties"
* Set "Configuration" to "All Configurations"
* Go to "Configuration Properties -> Debugging" and change "Working Directory" to $(ProjectDir)..\out\bin\\$(Configuration)
* Press "Ok"

Now you can build and run the engine in different configurations, i.e. to build in release mode
choose "Release" in the topmost combox box and press "ctrl + F5"

Once engine is built you can also run it outside of IDE by simply
running "C:\Projects\af3d\build_Win32_VS2019\out\bin\Release\af3d.exe"

Note! The engine is painfully slow in debug mode, better run it in release.

#### Run in editor mode

By default af3d.exe will run intro.af3 scene, if you want to run the editor, you'll need
to run it like this:

<pre>
af3d.exe intro.af3 1
</pre>

If you want to run some other scene, do this:

<pre>
af3d.exe probes.af3
</pre>

### 5. Build for linux
-------------------

#### Install LFS

This project uses Git Large Files extension. You need to install lfs if it is not already installed.
Do:

<pre>
git lfs install
</pre>

And make sure all large files are downloaded.

#### Compile and run

Install prerequisites:

<pre>
sudo apt-get install cmake libx11-dev libxxf86vm-dev libopenal-dev
</pre>

Then:

<pre>
./cmake_i386_release.sh
cd ../af3d-release
make -j4
</pre>

Then:

<pre>
cd ./out/bin
./af3d
</pre>

#### Run in editor mode

By default ./af3d will run intro.af3 scene, if you want to run the editor, you'll need
to run it like this:

<pre>
./af3d intro.af3 1
</pre>

If you want to run some other scene, do this:

<pre>
./af3d probes.af3
</pre>

### 6. Special keys
-------------------

There're a couple of special keys which you can press in editor or in game mode:

* **P** - Toggles physics debug draw
* **K** - Toggles game debug draw
* **M** - Toggles slowmo mode

You can use **W/S/A/D** to move around, **SPACE/C** - to move up / down, **SHIFT** key to move faster.

**I** in editor mode opens action menu.

There're hints for the rest of the hot keys in the editor.

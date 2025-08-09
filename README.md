2d (and 3d) graphics blueprint

![demo](res/gifs/demo_video.gif)

- I wanted to make a 2d pixel art game, and then wanted to make an engine, and decided that just having a well organized blueprint that is expanded upon, on a per game basis, is easier and seems a little more manageable/worthwhile.
- After finishing the 2d pixel art focused system, I wanted to explore 3d rendering. I made another repo that was simply exploring how the different matrix operations work in a 3d space, and it seemed like it could be added to this, even though the 3d aspect is somewhat out of the scope/focus of the initial blueprint goals.

This project attempts to fulfill the primary needs to start making a 2d game (or very simply 3d game). It is a system that can be improved on a per-game basis, focusing on 2D games with OpenGL rendering, basic entity systems, physics, and animation.
- Started with GLFW + glad, then I gave up and switched to only using SDL because it was more complicated than I thought and I just wanted some easy rendering, without shaders and all. Eventually I realized that SDL is not ideal for some of the features I wanted, so I went back to GLFW + glad, learned the basics of shaders, and it seems to have been very worthwhile and not as complicated as I had thought.
- After finishing the 2d rendering and exploring the conventions for 3d work in opengl, I found it interesting enough to want to include into this repo, as an "all-in-one" kickstarter for basic game development.

features:
- OpenGL-based orthographic and perspective rendering, trying to follow the standard opengl graphics pipeline
  - Batch rendering system for sprite sheets and frames
- Basic physics engine: static bodies, kinematic and normal bodies, collisions, collision layers, no tunnelling
- Basic animation system for the sprite frames
- Basic input handling and key bind configuration
- Font rendering system with custom font sprite sheet
- Very basic audio capabilities
- Basic math library that mirrors linmath. I initially started with linmath and found it amazing, though I wanted to provide my own functions that have row-major matrices that I derived and understand thoroughly.

some directories:
- examples/: these are sample games and examples of how this system can be used
  - both 2d and 3d examples here
- c-lib/: utility libraries (dynamic list, logging, etc.)
- shaders/: GLSL shader files
- lib/: external libraries (GLAD, linmath, miniaudio)
- res/: game assets (sprite sheets)

`make` to build
`make GAME=dir` for specific game src files within `examples/dir`

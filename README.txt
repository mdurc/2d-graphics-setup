2d graphics blueprint
- I wanted to make a 2d pixel art game, and then wanted to make an engine, and now have decided that just having a well organized blueprint that is expanded upon, on a per game basis, is easier and seems a little more manageable/worthwhile.

This project provides a complete foundation for building 2D games with modern OpenGL rendering, entity systems, physics, and animation.
- Started with GLFW + glad, then I gave up and switched to only using SDL because it was more complicated than I thought and I just wanted some easy rendering, without shaders and all. Eventually I realized that SDL is not ideal for some of the features I wanted, so I went back to GLFW + glad, learned the basics of shaders, and it seems to have been very worthwhile and not as complicated as I had thought.

features:
- OpenGL-based rendering
  - Batch rendering system for sprite sheets and frames
- Basic physics engine: static bodies, kinematic and normal bodies, collisions, collision layers, no tunnelling
- Basic animation system for the sprite frames
- Basic input handling and key bind configuration
- Font rendering system with custom font sprite sheet
- Very basic audio capabilities

some directories:
- game/: this is where the game specific game loop and implementation would go, with all of the entity creation, etc.
- c-lib/: utility libraries (dynamic list, logging, etc.)
- shaders/: GLSL shader files
- lib/: external libraries (GLAD, linmath)
- res/: game assets (sprite sheets)

`make` to build

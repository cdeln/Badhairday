# Having a bad hair day?
```
             |||||||
         ____|||||||||__
    ----/               \-------
 ------/                \--~~~--
-------|    O       O   |--------
    ---|        >       |------
       \    ________    /
        \              /
        ^^^^^^^^^^^^^^^^
```    
This is a graphics project for the course [Advanced Computer Graphics](https://computer-graphics.se/TSBK03/)
held by [Ingemar Ragnemalm](https://computer-graphics.se/TSBK03/) at Linköping University, Sweden during 2017.
The project explores simulation of light scattering in hair, for detailed information read the [report](BadHairDay.pdf) (in Swedish).
The project is built using [MicroGlut](http://www.ragnemalm.se/lightweight/aboutmicroglut.html).

During the course of the project we got really tired of OpenGL runtime errors,
so we developed a library for compiling GLSL shaders.
The result became a (far from complete) statically compiled graphics pipeline GTL (Graphics Template Library)
together with a (ugly hack) shader compilator SBT (Shader Build Tool).
GTL provides means of setting attributes in shaders directly in the client code,
and getting static compilation errors, if for example an attribute does not exist on a shader or is of wrong type.
This can be done after that the shader code has been compiled into a C++ class with SBT. 

## Project structure
* common (MicroGLUT)
* examples (Some example code we based the rest of the code on)
* gtl (GTL library)
* lib (Common files for this project)
* slask (old unused/broken files, but kept for reference)
* src (main source files)
    - tesselation_bunny.cpp (Base implementation of the tesselation)
    - voxelized_tess_bunny.cpp (Base implementation of the final solution to this project)
    - voxelized_tess_bunny_optimized.cpp (Optimized version of the above)
    - integer_voxelized_tess_bunny.cpp (Implementation based on integer arithmetic, which enables atomic operations on the GPU)
    - integer_voxelized_tess_bunny_optimized.cpp (Optimized version of the above)
* shaders (.prog shader source code)
* models (.obj model files)
* textures (Self-explaining)

## SBT and GTL
* sbt.py, builds a (potentially *mixed*, with *.prog* exttension) shader source file into a C++ GTL class
* sbt_make.py, generates a makefile based on a yaml configuration file listing shader source code to be compiled

## Dependencies
* OpenGL (version >= 4.5)

## Installation (Debian)
* `git submodule init`
* `git submodule update`
* `apt-get install libgl1-mesa-dev libglm-dev libxt-dev`
* `make`

## Running
* Check out the various executables in the `build` directory

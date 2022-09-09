# FieldView

This repository contains the source code for FieldView, a scientific vector field visualisation tool, developed for our paper *FieldView: An interactive software tool for exploration of three-dimensional vector fields*.

:warning: **The application requires an NVIDIA RTX graphics card for the mesh shader support.**

## Installation

To build the software you need to install a C++ compiler and CMake, which can be done the easiest way by using the system package manager.

Then, clone the repository to a local directory on your system by issuing the following command:
```
git clone https://github.com/pnwkw/field_view
```
It will fetch the contents of the repository.

If you want to run the program on a Linux machine running in text-mode (i.e. without a graphical interface such as *Gnome* or *KDE*) you need to change the OpenGL provider used
by the program from GLFW3 to EGL.

This can be done by commenting out the `set(USE_GLFW_CONTEXT true)` and uncommenting `set(USE_EGL_CONTEXT true)` in the `CMakeLists.txt` (lines 7 and 8):
```
# EGL can be used to execute on systems with no graphics (text mode), otherwise GLFW
#set(USE_GLFW_CONTEXT true)
set(USE_EGL_CONTEXT true)
```

If using Windows or Linux with a graphical interface installed, then the default GLFW3 can be used and no additional changes are needed.

The next step is to create a build directory with:
```
cmake -B build
```
CMake will generate build scripts for you. To compile the program, type:
```
cmake -B build -S .
```

## Running the application

First, move to the directory where the executable resides:
```
cd build
```
### Configuration
In the `build/data` directory an example `config.json` was automatically created. Here some options can be specified
which are unavailable in the GUI:

* `deviceID`: The index of the GPU in your machine that you want to use (not needed if GLFW used)
* `randomSeed`: The seed for generation of space points
### Running
Execute the application with the following command:
```
./field_view
```

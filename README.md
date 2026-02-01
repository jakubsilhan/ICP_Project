# ICP project
A semestral project for the ICP course at the Technical University of Liberec

Authors:

  - Jakub Šilhán
  - Hynek V. Svobodný
  

## About
This project is an interactive OpenGL application written in C++. For image processing and camera capture, OpenCV is used.

The application's main scene involves shooting various floating objects, with the player being able to move around within it.

In addition to this main scene, it is possible to compile the app with options to start with a different entry point, resulting in showing some previous or partially unrelated assignments, instead.

### Features
The main application has the following features:

  - Camera tracking with OpenCV, with red color and face recognition
  - The tracker running in another thread, using a pool and synced deques for passing data between threads
  - Requires OpenGL 4.6 and the Core profile and uses DSA for loading data (drawback: not being able to run on macOS)
  - A toggle for VSync, antialiasing and fullscreen vs window mode
  - Screenshot with path selection using tinyfiledialogs
  - 2D GUI layered over the scene using ImGui
  - Camera image with recognized objects visible as part of the GUI overlay
  - Processing window events in both the UI and the scene
  - Scene composed of textured or single-color objects, using modular architecture
  - Object file loader and simple mesh generator functions as two means of generating models
  - Camera being able to move around within the scene
  - Bounding boxes of objects and raycasting, allowing the player to shoot the objects
  - Background music, sound effect for shooting
  - 3D spatial sound effect when locating ("pinging") an object to shoot, coming from the object's position
  - Sccene locking based on data from the tracker

## Build and run instructions
The build an run on Kubuntu 24.04 LTS:

  1. Install packages:

         sudo apt install curl ninja-build libglm-dev libglm-doc nlohmann-json3-dev libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config git automake gcc g++ make cmake perl libopencv-dev libglfw3-dev libglew-dev glew-utils

  2. Set up vcpkg by following [the vcpkg tutorial at Microsoft Learn](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash).

     > [!NOTE]
     > Make sure you leave the `vcpkg` directory after setting vcpkg up,
     > so you don't do the following steps in the `vcpkg` directory.

  3. Clone the repository and change current directory to it:

         git clone https://github.com/jakubsilhan/ICP_Project.git
         cd ICP_Project

  4. Configure the build:
   
         cmake --preset default

  5. Build the executable:

         cmake --build build

  6. Run the executable:

         build/icp

The app displays the available actions using its GUI.

> [!NOTE]
> When taking screenshots, the file save dialog API does not select the file mask on Kubuntu 24.04,
> whereas on later versions it works correctly. Please choose the `.png` filename extension when naming the screenshot.


### Building and running with other entry points
There are also other entry points available, specified by CMake preset used.
The following presets are available:

  - `Shooter` - the default shooter game (the default)
  - `Raster` - a simple raster processing app (video encoder)
  - `Viewer` - the default app but with `ViewerScene` instead of `ShooterScene` (the functionality out of the scene is the same like `GLAPP_SHOOTER`)
  - `TrackApp` - a simple camera tracker app using OpenCV
  - `ThreadTrackApp` - a threaded camera tracker app using OpenCV + an OpenGL window with triangle

To build and run with other entry points:

  1. Make sure you have gone through the general build and run steps 1 through 3.

  2. Configure the build:

         cmake --preset <preset>

     where `preset` is the chosen preset.

  5. Build the executable:

         cmake --build build

  4. Run the executable:

         build/icp

> [!NOTE]
> In order to revert to the default entry point after building a different one, you need to follow the above steps with `preset` set to `Shooter`, otherwise the last mode stays active.

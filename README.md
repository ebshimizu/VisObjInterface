# Visual Objectives Interface

## Builds

Scene folders must be placed in the same directory as the executable file in order to load properly.
Images are loaded via relative path by default. You can edit the `.rig.json` files
to point to the proper path if the relative pathing fails. The property to look for is
`arnold_cache.cache_options.path` (at the bottom of each file).

App and All Scenes: [Win x64](https://github.com/ebshimizu/VisObjInterface/releases/download/v0.17.2/VisObjInterface_win32_x64.zip)

Scenes included: 
* Light Lab - Black Scrim
* Light Lab - Living Room
* Light Lab - Garden
* ETC Century

## Using the app

Memory requirements are steep (the app loads and decompresses EXR files in memory). I recommend
at least 16GB RAM.

Rendering occurs manually if using the slider controls. Press the `r` key to trigger a render after manipulating lights.

This is a research interface, so some functions in the app may not work correctly or may cause a crash.
Using the default settings should be safe. Load a scene from the file menu, then load a set of images
to use as visual objectives by selecting a folder containing the JPG or PNG files of interest.

If you are loading a new scene, it's recommended that you restart the app. Not everything gets cleared out
properly.

## Building from Source

The Visual Objectives interface depends on [Lumiverse](https://github.com/ebshimizu/Lumiverse)
for providing lighting control and rendering support, and [JUCE](https://github.com/WeAreROLI/JUCE)
for UI components. The interface has only been built for Windows systems, and has not been tested on MacOS
or *NIX systems.

### Building the Dependencies

**Lumiverse**

The Visual Objectives interface is compatible with the current version of [Lumiverse](https://github.com/ebshimizu/Lumiverse). 
In order to properly build the interface, Lumiverse must be configured properly.
At minimum, the build configuration should enable the `LumiverseCore_INCLUDE_CACHING_ARNOLD` option.
This includes a basic HDR rendering system with the Lumiverse lighting control options. If you are attempting
to control a real-world stage, you will need to enable other build options as needed by your control system.
Additional Lumiverse documentation can be found at [lumiverse.cs.cmu.edu](http://lumiverse.cs.cmu.edu).

Including this particular build option requires a few additional dependencies, including
zlib, libpng, and OpenEXR. Lumiverse includes the compatible versions of zlib and libpng
in the repository.
If you are building the project on Windows, OpenEXR v2.2.0 is recommended (2.3.0
broke the CMake config). If building OpenEXR with cmake on Windows, use the "build static libraries" option.

**Default Library Paths**

The following is a list of where the Visual Studio 2017 solution in the repository
is looking for files. If you have all of the dependencies built and in these folders,
you should be able to build the included solution file without modifications.
If a build fails to a missing library, you may need to adjust the search paths in the
Visual Studio solution file.

**Headers**
* Lumiverse: `C:/Program Files/Lumiverse/include`
* libpng: `C:/Program Files/libpng/include`
* zlib: `C:/Program Files/zlib/include`
* openexr (the current build file uses the Debug folder as the include location)
  * Debug: `C:/Program Files/openexr/Debug/include`
  * Release: `C:/Program Files/openexr/Release/include`

**Libraries**
* Lumiverse
    * Debug: `C:/Program Files/Lumiverse/Debug/lib`
    * Release: `C:/Program Files/Lumiverse/Release/lib`
* libpng: `C:/Program Files/libpng/lib`
* zlib: `C:/Program Files/zlib/lib`
* ilmbase (part of OpenEXR)
    * Debug: `C:/Program Files/ilmbase/Debug/lib`
    * Release: `C:/Program Files/ilmbase/Release/lib`
* openexr
    * Debug: `C:/Program Files/openexr/Debug/lib`
    * Release: `C:/Program Files/openexr/Release/lib`

**JUCE**

The Visual Objectives interface is built against JUCE v5.4.1. It is likely compatible with
minor revisions in the 5.x series of releases. Copies of the module code are provided, so JUCE shouldn't
need to be re-configured.
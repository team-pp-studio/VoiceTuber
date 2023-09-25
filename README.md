VoiceTuber Software Design Document
===================================

# You are on the `main` branch, which means the save format is unstable, any update may break your save, so make small experimental projects.

1\. Introduction
----------------

This design document outlines the features and functionality of
VoiceTuber, a lightweight software application designed for
PNGTubers. VoiceTuber streamlines the content creation process by
emphasizing the use of only a microphone (and sometimes even no
microphone), eliminating the need for additional hardware such as
webcams or tracking devices. The software aims to provide an engaging
and user-friendly experience for PNGTubers, making it easy for
creators to produce dynamic and interactive content with minimal setup
and equipment.

2\. Features and Functionality
------------------------------

### 2.1. Core Features

- [x]  Viseme detection for lip-syncing
- [x]  Sprite-based animation
- [x]  Rudimentary physics for realistic movement
- [ ]  Custom hotkeys for triggering expressions and gestures
- [ ]  Wake-word detection using Pocketsphinx

### 2.2. Text-to-Speech Integration

-  [x] Integration with Azure TTS for text-to-speech functionality
    - [ ]  Step-by-step guide for users to obtain and set up an API key
- [ ]  TTS to voice Twitch chat messages
- [ ]  Alternative for VTubers to type instead of talking, using TTS for vocalization

### 2.3. Streaming Platform Integration

- [x]  Twitch chat integration for seamless viewer interaction
- [ ]  Interactive features using Twitch bits or reward points, such as throwing objects at the model


### 2.4. Mouse Tracking for Eye and Body Movements
- [x]  Avatar's eyes follow the user's cursor for a more lifelike and responsive experience
- [ ]  Post-processing pass and body morphing based on mouse position, creating an effect where the body of the model follows the mouse


3\. Open-Source and Lightweight Approach
----------------------------------------

- [x]  Released under an MIT license
- [x]  Donation button for financial support through GitHub
- [x]  Focus on maintaining a lightweight application without compromising features

4\. User Interface and Customization
------------------------------------

### 4.1. Avatar Customization

- [x] Integrated designer within the application
- [x] Blender-style editing for 2D avatar customization
- [ ] Addition of gadgets and a toolbar for more traditional editing
    options

5\. Dependencies
----------------

* **Dear ImGui** - A lightweight and efficient Immediate Mode Graphical User Interface library for creating simple and functional graphical interfaces.
* **GLM** - A C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specification, providing matrix operations and other essential functionality.
* **Pocketsphinx** - A speech recognition library that converts spoken language into text.
* **SDL2** - A cross-platform development library that provides low-level access to audio, keyboard, mouse, and display functions, as well as window management.
* **cpptoml** - A header-only library for parsing [TOML][toml] configuration files.
* **json** - JSON parser library
* **libcurl** - HTTP client library
* **libuv** - A cross-platform that provides support for asynchronous I/O based on event loops
* **log** - Small logging library to simplify debugging and monitoring of application processes.
* **sdlpp** - A compact C++ wrapper around SDL2, streamlining its integration and usage in C++ applications.
* **ser** - A lightweight and efficient serialization/deserialization library for C++
* **stb** - A collection of single-file public domain libraries, specifically used in this project for decoding and encoding images in various formats.

6\. Build Instructions
-----------------------

**Windows/Visual Studio:**

* Ensure you have the *Desktop development with C++* workload and the *C++ CMake tools for Windows* individual component installed.
* Export PocketSphinx to your Conan cache by running `conan export ./recipes/pocketsphinx --version 5.0.1`.
* Install required dependencies by running `conan install ./ --build missing -s build_type=Release`.
* Now open the project directory in Visual Studio and it will automatically configure your project.

After locally installing, you will need to copy the dependencies DLLs into the installation directory.
You can add `-d full_deploy` to the `conan install` command to copy them outside the Conan cache and then manually copy them to the installation location.
Links: 
Cmake: https://cmake.org/download/
Conan: https://conan.io/downloads

**Linux:**

Building with Conan:

* Ensure you have Conan, CMake and GCC installed.
    * Conan might not be available in your distribution repositories, but it's also available through pip.
* Export PocketSphinx to your conan cache by running `conan export ./recipes/pocketsphinx --version 5.0.1`.
* Install required dependencies: `conan install ./ --build missing -s build_type=Release`
* Build the binary: `cmake --build --preset conan-release`
* And install locally: `cmake --install build/Release --prefix ./local_install/`

You can also add the location of the shared libraries to the `LD_LIBRARY_PATH` environment variable.

Building with coddle:

* Install dependencies
```
sudo apt-get install -y clang pkg-config libsdl2-dev libuv1-dev git cmake
```
* Clone the app
```
git clone --recurse-submodules https://github.com/team-pp-studio/VoiceTuber.git
```

* Build Pocketsphinx
```
cd VoiceTuber/3rd-party/pocketsphinx
cmake -S . -B build
cmake --build build
cmake --build build --target install
cd ../../..
```
Last `cmake` command you may need to run with `sudo`.

* Clone and compile the build tool `coddle`
```
git clone https://github.com/coddle-cpp/coddle.git && cd coddle && ./build.sh
```
* Install `coddle`
```
sudo ./deploy.sh
cd ..
```
* Build VoiceTuber
```
cd VoiceTuber/src && coddle
```
* Run the application
```
../VoiceTuber
```

7\. TODO
--------
Top Priority
 - [ ] the app crashes on Twitch chat with unstable Internet
 - [x] implement mouth based on individual images instead of the sprite sheet
 - [x] implement blink based on individual images


General Priority
- [ ] remember directory in open/save dialog boxes
- [ ] search for files in the dialog box

- [ ] Transition to/Add Softbody Physics

- [ ] (feature) support for transparency (Transparency for OBS so users will not need a green/blue background and can use all colors in the PNG model)
- [ ] twitch extension (triggers for bits)
- [ ] TikTok companion app
- [ ] outdoor streaming from the phone with PNGTuber overlay
- [ ] stream directly from VoiceTuber

Feedback
  
- [ ] Vale don't like tha they can select what mic is being used
- [ ] Don't expect the placement of stuff (UI UX)
   - [ ] Swap description and value
   - [ ] Placement of bouncer options confusing
- [ ] Really wants drag and drop files
- 
- [ ] Don't like the separation of windows.thinks that should be an option

- [ ] better compatibility for multiple windows, what I mean by this
      is, if I move the voice tuber application to another window, the
      eye tracking still follows the mouth but only around the first
      window and it ends up just looking to the bottom left if I move
      it to my second monitor (left monitor) (see screenshots, I know
      it doesn't capture my mouse so it just highlighted the location
      box instead to show where my mouse is and the task bar shows
      what monitor I'm on)

- [ ] different variations of bounce, I like the bounce but I really
      only want the bounce once when I start talking and resetting
      when I stop talking, hell it'd also be nice if I could customize
      it so that if I was talking fast or excited or going high
      pitched then there'd be more bounce frequency and otherwise less
      bounce frequency again going based off what veadotube does is it
      just has a minimum microphone range where if the input from the
      microphone is above that then you're talking about if it's below
      that then you're not talking
- [ ] also physics breaks if you go too high with the values, luckily
      it can be reset by unchecking and re-checking physics but again,
      tooltip of some sort would be nice
- [ ] Some sort of micro training, voice data to detect
      expressions. It’s more than just weak words. It’s a voice
      inflection. my guess the implementation would be when you’re
      done with the stream you could go back and clip sections where
      you’re happy, sad or mad, and then compile them together, and
      that will train a little thingymabobber to understand
      inflection, 10 tones, and be able to signal changes

Completed
- [x] (bug) there are some transparency issues
- [x] (bug) after pressing d, make it g
- [x] double-click to enable editing
- [x] (bug) pressing "Open" if nothing is selected crashes
- [x] (bug) App crashes after deleting a mouth and clicking on another mouth.
- [x] add "add" buttons in the add menu 
- [x] (bug) crash if an image is deleted from the project
- [x] (bug) after transparency fix was made, bouncer stopped working.
- [x] (Feature) App works when minimized.
- [x] (bug) The app is crashing if it is not started from the directory where the executable is located.
- [x] (bug) changing the disk does not work
- [x] (bug) .. is confusing
- [x] (feature) undo
- [x] drag&drop nodes in the hierarchy (REALLY WANTS CLICK AND DRAG)
- [x] drag controls for move with gizmos (REALLY WANTS CLICK AND DRAG)
- [x] Don't expect the placement of stuff (UI UX)
   - [x] Swap description and value
   - [x] Placement of bouncer options confusing
- [x] (feature) each node can have different amount of bounce
- [X] They REALLY expected That value bars were able to be clicked and dragged
- [X] REALLY WANTS CLICK AND DRAG - covered in TODO
- [X] Drag and drop files into the software - covered in TODO
- [x] Really wants drag and drop files
- [x] highlight the chat if it is disconnected from the Internet
 - [x] allow limiting of frame rate
 - [x] better compatibility for multiple windows
 - [x] rotation slider
 - [x] Hotkeys information
 - [x] Physics breaks - cannot reproduce
 - [x] have it so you can enter a value for rotation instead of a
      slider... or at least a mouse tooltip that says "hold ctrl to
      enter value"
 - [X] We don't clear draws - known issue and according to internet it is Windows issue
 - [x] Azure TTS
- [x] (feature) Easing the bounce with a filter, maybe add some 2nd-order filter
      to have overshooting
- [x] Have a splash dialog similar as Blender one (Easier initial Project navigation)

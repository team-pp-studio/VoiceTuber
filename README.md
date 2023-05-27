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

-  [ ] Integration with Azure TTS for text-to-speech functionality
    - [ ]  Step-by-step guide for users to obtain and set up an API key
- [ ]  TTS to voice Twitch chat messages
- [ ]  Alternative for VTubers to type instead of talking, using TTS for vocalization

### 2.3. Streaming Platform Integration

- [x]  Twitch chat integration for seamless viewer interaction
- [ ]  Interactive features using Twitch bits or reward points, such as throwing objects at the model

### 2.4. Expression and Gesture Presets

- [ ]  Library of pre-set expressions and gestures (e.g., surprise, anger, joy)
- [ ]  Custom hotkeys and wake words for triggering presets

### 2.5. Scene Transitions and Effects

- [ ]  Library of pre-made scene transitions and effects
- [ ]  Customizable transitions for a professional and polished presentation

### 2.6. Mouse Tracking for Eye and Body Movements
- [x]  Avatar's eyes follow the user's cursor for a more lifelike and responsive experience
- [ ]  Post-processing pass and body morphing based on mouse position, creating an effect where the body of the model follows the mouse

### 2.7. Audio Analysis for Animation Triggers

- [ ]  Audio amplitude analysis to use different sets of mouth shapes based on the loudness or quietness of the VTuber's speech
- [ ]  Pitch analysis for detecting excitement or screaming

### 2.8. Voice Anonymization

- [ ]  Speech-to-text (STT) conversion to transcribe the streamer's voice into text
- [ ]  Text-to-speech (TTS) system to vocalize the transcribed text with a synthetic voice, concealing the streamer's real voice
- [ ]  Various synthetic voice options for customization and personalization

By incorporating the voice anonymization feature, you can cater to a
wider audience of streamers, including those who are concerned about
privacy or prefer not to reveal their actual voice. This added
functionality can make your PNGTuber software even more versatile and
appealing to a diverse range of users.

3\. Open-Source and Lightweight Approach
----------------------------------------

- [x]  Released under an MIT license
- [ ]  Donation button for financial support through GitHub
- [ ]  Focus on maintaining a lightweight application without compromising features

4\. User Interface and Customization
------------------------------------

### 4.1. Avatar Customization

- [x] Integrated designer within the application
- [x] Blender-style editing for 2D avatar customization
- [ ] Addition of gadgets and a toolbar for more traditional editing
    options

5\. Monetization Strategy
-------------------------

### 5.1. Donations

- [ ] Open-source software with a donation button on GitHub for
    financial support from users who appreciate the application

### 5.2. Marketplace Platform

- [ ] Create a marketplace platform where users can upload, sell, and
    purchase custom models, accessories, and other assets for the
    PNGTuber software
- [ ] Generate revenue through commission fees on sales made through the
    platform
- [ ] Facilitate a thriving community of content creators and artists,
    contributing to the overall success and popularity of the software

By integrating a marketplace platform, you'll not only be providing a
means of monetization, but also encouraging a community of creators to
contribute to and benefit from the software. This approach allows you
to support the development and maintenance of the software while still
keeping it open-source and accessible to users.

6\. Conclusion
--------------

This design document provides an overview of the key features and
functionality for a lightweight and user-friendly PNGTuber software
application. By focusing on unique features and maintaining a
lightweight design, the software aims to differentiate itself from
competitors and provide a versatile solution for content creators. As
development progresses, user feedback and market trends should be
considered to further refine and improve the application.

7\. Dependencies
----------------

* **Dear ImGui** - A lightweight and efficient Immediate Mode Graphical User Interface library for creating simple and functional graphical interfaces.
* **GLM** - A C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specification, providing matrix operations and other essential functionality.
* **Pocketsphinx** - A speech recognition library that converts spoken language into text.
* **SDL2** - A cross-platform development library that provides low-level access to audio, keyboard, mouse, and display functions, as well as window management.
* **libuv** - A cross-platform that provides support for asynchronous I/O based on event loops
* **cpptoml** - A header-only library for parsing [TOML][toml] configuration files.
* **log** - Small logging library to simplify debugging and monitoring of application processes.
* **sdlpp** - A compact C++ wrapper around SDL2, streamlining its integration and usage in C++ applications.
* **ser** - A lightweight and efficient serialization/deserialization library for C++
* **stb** - A collection of single-file public domain libraries, specifically used in this project for decoding and encoding images in various formats.

8\. TODO
--------
Top Priority
 - [ ] better compatibility for multiple windows
 - [x] rotation slider
 - [ ] Hotkeys infomation
 - [ ] Physics breaks

General Priority
- [ ] remember directory in open/save dialog boxes
- [ ] search for files in the dialog box
- [ ] Have a spalsh dialog simular as Blender one (Easier initial Project navigtion)
- [ ]  Transition to/Add Softbody Phyics
- [x] Azure TTS
- [ ] (feature) automatically create and export sprite sheets based on 2 images
- [x] (feature) Easing the bounce with a filter, maybe add some 2nd-order filter
      to have overshooting
- [ ] (feature) support for transparency (Transparency for OBS so users will not need a green/blue background and can use all colors in the PNG model)
- [ ] twitch extention (triggers for bits)
- [ ] TikTok companion app
- [ ] outdoor streaming from the phone wiht PNGTuber overlay
- [ ] stream directly from VoiceTuber

Feedback
- [X] We don't clear draws - known issue and according to internet it is Windows issue
   
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
- [x] have it so you can enter a value for rotation instead of a
      slider... or at least a mouse tooltip that says "hold ctrl to
      enter value"
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
- [x] (bug) after transpancy fix was made, bouncer stopped working.
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
- [X] Drag and drop files into the software - coverd in TODO
- [x] Really wants drag and drop files
      
9\. Unorgonized Ideas
---------------------

- [ ] Puppeteer interface with keyboard and mouse
- [ ] Puppeteer interfacing with the game controller

Channel Point/Bit/Chat Ideas.

Vtuber/chat tts affected
1. Outfit change/hair style change (Vtuber)
1.A outfit/change on chat tts
1.B change vtubers/chat tts color (like a multiple or screen rgb/hex code.
2. Mouth fall off
3. Throw/fall objects at vtuber/chat
4. Part of model grows or shrinks

Minigames
1. Gacha outfit/asset (secret accessories to unlock via gacha)
1.A claw machine (Credit idea to ValeofHearts)
2. Take care of chat tts (tamagotchi)
3. Chat duel? 
4. Guess the amount (how much is in the jar)
5.* FISHING MINIGAME (I have no idea, but fishing minigame)
6.* COOKING/BAKING MINIGAME (Wichit loves cooking minigames.)

extension necessary ideas
* - items from previous list that may or may not benefit from this

1. Draw the streamer a asset or shirt decal in browser. (will be approved by the streamer before being uploaded.)
2. Move the vtuber or chat tts anywhere on the screen.

10\. Build Instructions
-----------------------

### Windows
* Clone the source code using GitHub Desktop
   * You can install Github Desktop from: <https://desktop.github.com/>
* Build Pocketsphinx
   * Install CMake from: <https://cmake.org/download/>
   * In the cloned project, locate the Pocketsphinx copy at `VoiceTuber/3rd-party/pocketsphinx`. Run the following commands in `cmd`:
```
cmake -S . -B build
cmake --build build
cmake --build build --target install -DCMAKE_INSTALL_PREFIX=bin
```
* Build libuv
    * Assume CMake is installed from the Pocketsphinx sections
    * In the cloned project, locate the libuv copy at `VoiceTuber/3rd-party/libuv`. Run the following commands in `cmd`:
```
$ mkdir -p build
$ (cd build && cmake .. -DBUILD_TESTING=ON) # generate project with tests
$ cmake --build build                       # add `-j <n>` with cmake >= 3.12
```
*
    * This steps will build Debug version of libuv, open generated `.sln` file from `VoiceTuber/3rd-party/libuv/libuv.sln` in Visual Studio
    * Switch Solution Configuration to Release
    * Build Solution with `Build/Build Solution` menu
* Build the rest of the project
    * Open `VoiceTuber.sln` in Visual Studio 2022
    * Use the menu option `Build/Build Solution`
### Linux
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
cd VoiceTuber && coddle
```
* Run the application
```
./VoiceTuber
```


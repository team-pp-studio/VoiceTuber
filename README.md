VoiceTuber Software Design Document
===================================

# You are on the `main` branch, which means the save format is unstable, so make small experimental projects; any update may break your save.

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

- [ ]  Twitch chat integration for seamless viewer interaction
- [ ]  Interactive features using Twitch bits or reward points, such as throwing objects at the model

### 2.4. Expression and Gesture Presets

- [ ]  Library of pre-set expressions and gestures (e.g., surprise, anger, joy)
- [ ]  Custom hotkeys and wake words for triggering presets

### 2.5. Scene Transitions and Effects

- [ ]  Library of pre-made scene transitions and effects
- [ ]  Customizable transitions for a professional and polished presentation

### 2.6. Mouse Tracking for Eye and Body Movements
- [ ]  Avatar's eyes follow the user's cursor for a more lifelike and responsive experience
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
* **SDL2_net** - A cross-platform networking library designed to work with SDL2
* **log** - Small logging library to simplify debugging and monitoring of application processes.
* **sdlpp** - A compact C++ wrapper around SDL2, streamlining its integration and usage in C++ applications.
* **ser** - A lightweight and efficient serialization/deserialization library for C++
* **stb** - A collection of single-file public domain libraries, specifically used in this project for decoding and encoding images in various formats.

8\. TODO

- [x] (bug) App crashes after deleting a mouth and clicking on another
      mouth.
- [ ] (feature) Easing the bounce with a filter, maybe add some 2nd-order filter
      to have overshooting
- [ ] (feature) undo
- [ ] remember directory in open/save dialog boxes
- [ ] search for files in the dialog box
- [ ] add "add" buttons in the add menu

11\. Unorgonized Ideas

- [ ] Puppeteer interface with keyboard and mouse
- [ ] Puppeteer interfacing with the game controller

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
* Build the rest of the project
    * Open `VoiceTuber.sln` in Visual Studio 2022
    * Use the menu option `Build/Build Solution`
### Linux
* Install dependencies
```
sudo apt-get install -y clang pkg-config libsdl2-dev libsdl2-net-dev git cmake
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


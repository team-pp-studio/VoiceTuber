PNGTuber Software Design Document
=================================

1\. Introduction
----------------

This design document outlines the features and functionality of a
lightweight PNGTuber software application. The software aims to
provide an engaging and user-friendly experience for content creators,
enabling them to create dynamic and interactive content without the
need for additional hardware such as webcams or tracking devices.

2\. Features and Functionality
------------------------------

### 2.1. Core Features

-   Viseme detection for lip-syncing
-   Sprite-based animation
-   Rudimentary physics for realistic movement
-   Custom hotkeys for triggering expressions and gestures
-   Wake-word detection using Pocketsphinx

### 2.2. Text-to-Speech Integration

-   Integration with Azure TTS for text-to-speech functionality
-   Step-by-step guide for users to obtain and set up an API key
-   TTS to voice Twitch chat messages
-   Alternative for VTubers to type instead of talking, using TTS for vocalization

### 2.3. Streaming Platform Integration

-   Twitch chat integration for seamless viewer interaction
-   Interactive features using Twitch bits or reward points, such as throwing objects at the model

### 2.4. Expression and Gesture Presets

-   Library of pre-set expressions and gestures (e.g., surprise, anger, joy)
-   Custom hotkeys and wake words for triggering presets

### 2.5. Scene Transitions and Effects

-   Library of pre-made scene transitions and effects
-   Customizable transitions for a professional and polished presentation

### 2.6. Mouse Tracking for Eye Movements

-   Avatar's eyes follow the user's cursor for a more lifelike and responsive experience

### 2.7. Audio Analysis for Animation Triggers

-   Audio amplitude analysis to use different sets of mouth shapes based on the loudness or quietness of the VTuber's speech
-   Pitch analysis for detecting excitement or screaming

3\. Open-Source and Lightweight Approach
----------------------------------------

-   Released under an MIT license
-   Donation button for financial support through GitHub
-   Focus on maintaining a lightweight application without compromising features

4\. User Interface and Customization
------------------------------------

### 4.1. Avatar Customization

-   Integrated designer within the application
-   Blender-style editing for 2D avatar customization
-   Addition of gadgets and a toolbar for more traditional editing options

5\. Conclusion
--------------

This design document provides an overview of the key features and
functionality for a lightweight and user-friendly PNGTuber software
application. By focusing on unique features and maintaining a
lightweight design, the software aims to differentiate itself from
competitors and provide a versatile solution for content creators. As
development progresses, user feedback and market trends should be
considered to further refine and improve the application.

## Dependencies

* **Dear ImGui** - A lightweight and efficient Immediate Mode Graphical User Interface library for creating simple and functional graphical interfaces.
* **GLM** - A C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specification, providing matrix operations and other essential functionality.
* **Pocketsphinx** - A speech recognition library that converts spoken language into text.
* **SDL2** - A cross-platform development library that provides low-level access to audio, keyboard, mouse, and display functions, as well as window management.
* **log** - Small logging library to simplify debugging and monitoring of application processes.
* **sdlpp** - A compact C++ wrapper around SDL2, streamlining its integration and usage in C++ applications.
* **ser** - serialization/deserialization library
* **stb** - A collection of single-file public domain libraries, specifically used in this project for decoding and encoding images in various formats.

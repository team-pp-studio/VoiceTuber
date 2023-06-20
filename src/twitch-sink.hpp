#pragma once
#include <glm/vec3.hpp>
#include <string>

class TwitchSink
{
public:
  virtual ~TwitchSink() = default;

  struct Msg
  {
    std::string displayName;
    std::string msg;
    glm::vec3 color = glm::vec3{0.f, 0.f, 0.f};
    bool isFirst = false;
    bool isMod = false;
    int subscriber = -1;
  };
  virtual auto onMsg(Msg) -> void = 0;
};

// color
//
// tag badge-info=subscriber/56
// tag badges=broadcaster/1,subscriber/0
// tag client-nonce=3e16f95e935d90392559499d43f923f9
// tag color=#FF69B4
// tag display-name=Mika314
// tag emotes=
// tag first-msg=0
// tag flags=
// tag id=c3d25169-9ed8-4c86-88df-ac25988c2f36
// tag mod=0
// tag returning-chatter=0
// tag room-id=224389503
// tag subscriber=1
// tag tmi-sent-ts=1681529715041
// tag turbo=0
// tag user-id=224389503
// tag user-type=
// tag user-type=
// source mika314!mika314@mika314.tmi.twitch.tv
// command PRIVMSG
// parameter #mika314
// parameter zzz
// msg PING :tmi.twitch.tv
// source no-source
// command PING
// parameter tmi.twitch.tv
// msg PING :tmi.twitch.tv
// source no-source

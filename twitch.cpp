#ifdef WIN32
#define NOMINMAX
#endif
#include "twitch.hpp"
#include <algorithm>
#include <log/log.hpp>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{
  const char *RPL_WELCOME = "001";
  //  const char *RPL_YOURHOST = "002";
  //  const char *RPL_CREATED = "003";
  //  const char *RPL_MYINFO = "004";
  //  const char *RPL_ISUPPORT = "005";
  //  const char *RPL_BOUNCE = "010";
  //  const char *RPL_UMODEIS = "221";
  //  const char *RPL_LUSERCLIENT = "251";
  //  const char *RPL_LUSEROP = "252";
  //  const char *RPL_LUSERUNKNOWN = "253";
  //  const char *RPL_LUSERCHANNELS = "254";
  //  const char *RPL_LUSERME = "255";
  //  const char *RPL_ADMINME = "256";
  //  const char *RPL_ADMINLOC1 = "257";
  //  const char *RPL_ADMINLOC2 = "258";
  //  const char *RPL_ADMINEMAIL = "259";
  //  const char *RPL_TRYAGAIN = "263";
  //  const char *RPL_LOCALUSERS = "265";
  //  const char *RPL_GLOBALUSERS = "266";
  //  const char *RPL_WHOISCERTFP = "276";
  //  const char *RPL_NONE = "300";
  //  const char *RPL_AWAY = "301";
  //  const char *RPL_USERHOST = "302";
  //  const char *RPL_UNAWAY = "305";
  //  const char *RPL_NOWAWAY = "306";
  //  const char *RPL_WHOREPLY = "352";
  //  const char *RPL_ENDOFWHO = "315";
  //  const char *RPL_WHOISREGNICK = "307";
  //  const char *RPL_WHOISUSER = "311";
  //  const char *RPL_WHOISSERVER = "312";
  //  const char *RPL_WHOISOPERATOR = "313";
  //  const char *RPL_WHOWASUSER = "314";
  //  const char *RPL_WHOISIDLE = "317";
  //  const char *RPL_ENDOFWHOIS = "318";
  //  const char *RPL_WHOISCHANNELS = "319";
  //  const char *RPL_WHOISSPECIAL = "320";
  //  const char *RPL_LISTSTART = "321";
  //  const char *RPL_LIST = "322";
  //  const char *RPL_LISTEND = "323";
  //  const char *RPL_CHANNELMODEIS = "324";
  //  const char *RPL_CREATIONTIME = "329";
  //  const char *RPL_WHOISACCOUNT = "330";
  //  const char *RPL_NOTOPIC = "331";
  //  const char *RPL_TOPIC = "332";
  //  const char *RPL_TOPICWHOTIME = "333";
  //  const char *RPL_INVITELIST = "336";
  //  const char *RPL_ENDOFINVITELIST = "337";
  //  const char *RPL_WHOISACTUALLY = "338";
  //  const char *RPL_INVITING = "341";
  //  const char *RPL_INVEXLIST = "346";
  //  const char *RPL_ENDOFINVEXLIST = "347";
  //  const char *RPL_EXCEPTLIST = "348";
  //  const char *RPL_ENDOFEXCEPTLIST = "349";
  //  const char *RPL_VERSION = "351";
  //  const char *RPL_NAMREPLY = "353";
  //  const char *RPL_ENDOFNAMES = "366";
  //  const char *RPL_LINKS = "364";
  //  const char *RPL_ENDOFLINKS = "365";
  //  const char *RPL_BANLIST = "367";
  //  const char *RPL_ENDOFBANLIST = "368";
  //  const char *RPL_ENDOFWHOWAS = "369";
  //  const char *RPL_INFO = "371";
  //  const char *RPL_ENDOFINFO = "374";
  //  const char *RPL_MOTDSTART = "375";
  //  const char *RPL_MOTD = "372";
  //  const char *RPL_ENDOFMOTD = "376";
  //  const char *RPL_WHOISHOST = "378";
  //  const char *RPL_WHOISMODES = "379";
  //  const char *RPL_YOUREOPER = "381";
  //  const char *RPL_REHASHING = "382";
  //  const char *RPL_TIME = "391";
  //  const char *ERR_UNKNOWNERROR = "400";
  //  const char *ERR_NOSUCHNICK = "401";
  //  const char *ERR_NOSUCHSERVER = "402";
  //  const char *ERR_NOSUCHCHANNEL = "403";
  //  const char *ERR_CANNOTSENDTOCHAN = "404";
  //  const char *ERR_TOOMANYCHANNELS = "405";
  //  const char *ERR_WASNOSUCHNICK = "406";
  //  const char *ERR_NOORIGIN = "409";
  //  const char *ERR_INPUTTOOLONG = "417";
  //  const char *ERR_UNKNOWNCOMMAND = "421";
  //  const char *ERR_NOMOTD = "422";
  //  const char *ERR_ERRONEUSNICKNAME = "432";
  //  const char *ERR_NICKNAMEINUSE = "433";
  //  const char *ERR_USERNOTINCHANNEL = "441";
  //  const char *ERR_NOTONCHANNEL = "442";
  //  const char *ERR_USERONCHANNEL = "443";
  //  const char *ERR_NOTREGISTERED = "451";
  //  const char *ERR_NEEDMOREPARAMS = "461";
  //  const char *ERR_ALREADYREGISTERED = "462";
  //  const char *ERR_PASSWDMISMATCH = "464";
  //  const char *ERR_YOUREBANNEDCREEP = "465";
  //  const char *ERR_CHANNELISFULL = "471";
  //  const char *ERR_UNKNOWNMODE = "472";
  //  const char *ERR_INVITEONLYCHAN = "473";
  //  const char *ERR_BANNEDFROMCHAN = "474";
  //  const char *ERR_BADCHANNELKEY = "475";
  //  const char *ERR_BADCHANMASK = "476";
  //  const char *ERR_NOPRIVILEGES = "481";
  //  const char *ERR_CHANOPRIVSNEEDED = "482";
  //  const char *ERR_CANTKILLSERVER = "483";
  //  const char *ERR_NOOPERHOST = "491";
  //  const char *ERR_UMODEUNKNOWNFLAG = "501";
  //  const char *ERR_USERSDONTMATCH = "502";
  //  const char *ERR_HELPNOTFOUND = "524";
  //  const char *ERR_INVALIDKEY = "525";
  //  const char *RPL_STARTTLS = "670";
  //  const char *RPL_WHOISSECURE = "671";
  //  const char *ERR_STARTTLS = "691";
  //  const char *ERR_INVALIDMODEPARAM = "696";
  //  const char *RPL_HELPSTART = "704";
  //  const char *RPL_HELPTXT = "705";
  //  const char *RPL_ENDOFHELP = "706";
  //  const char *ERR_NOPRIVS = "723";
  //  const char *RPL_LOGGEDIN = "900";
  //  const char *RPL_LOGGEDOUT = "901";
  //  const char *ERR_NICKLOCKED = "902";
  //  const char *RPL_SASLSUCCESS = "903";
  //  const char *ERR_SASLFAIL = "904";
  //  const char *ERR_SASLTOOLONG = "905";
  //  const char *ERR_SASLABORTED = "906";
  //  const char *ERR_SASLALREADY = "907";
  //  const char *RPL_SASLMECHS = "908";
} // namespace

static const char *server = "irc.chat.twitch.tv";
static const char *port = "6667";

Twitch::Twitch(class Uv &uv, std::string aUser, std::string aKey, std::string aChannel)
  : uv(uv),
    user(std::move(aUser)),
    key(std::move(aKey)),
    channel(std::move(aChannel)),
    retry(uv.getTimer())
{
  init();
}

auto Twitch::init() -> void
{
  LOG("Init:", server, ":", port, "user", user, "channel", channel);
  state = State::connecting;
  retry.stop();

  auto s = uv.get().connect(server, port, [this](int status, Tcp aTcp) {
    if (status < 0)
    {
      LOG(__func__, "error:", uv_err_name(status));
      initiateRetry();
      return;
    }
    tcp = std::move(aTcp);
    sendPassNickUser();
  });
  if (s < 0)
  {
    LOG(__func__, "error:", uv_err_name(s));
    initiateRetry();
    return;
  }
}

auto Twitch::sendPassNickUser() -> void
{
  std::ostringstream msg;
  msg << "PASS " << key << "\r\n";
  msg << "NICK " << user << "\r\n";
  msg << "USER nobody unknown unknown :noname\r\n";
  auto s = tcp.write(msg.str(), [this](int status) {
    if (status < 0)
    {
      LOG(__func__, "error:", uv_err_name(status));
      initiateRetry();
      return;
    }
    readStart();
  });
  if (s < 0)
  {
    LOG(__func__, "error:", uv_err_name(s));
    initiateRetry();
    return;
  }
}

auto Twitch::readStart() -> void
{
  auto s = tcp.readStart([this](int status, std::string msg) {
    if (status < 0)
    {
      LOG(__func__, "error:", uv_err_name(status));
      initiateRetry();
    }
    buf.insert(std::end(buf), std::begin(msg), std::end(msg));
    parseMsg();
  });
  if (s < 0)
  {
    LOG(__func__, "error:", uv_err_name(s));
    initiateRetry();
    return;
  }
}

auto Twitch::parseMsg() -> void
{
  for (;;)
  {
    auto it = std::find(std::begin(buf), std::end(buf), '\n');
    if (it == std::end(buf) || it == std::begin(buf) || *(it - 1) != '\r')
      return;
    auto msg = std::string{std::begin(buf), it - 1};
    buf.erase(std::begin(buf), it + 1);
    LOG("msg", msg);
    auto tags = std::vector<std::pair<std::string, std::string>>{};
    auto source = std::optional<std::string>{};
    auto command = std::string{};
    auto parameters = std::vector<std::string>{};
    size_t nextSpace = 0;
    for (size_t index = 0; index < msg.size(); index = nextSpace + 1)
    {
      nextSpace = msg.find(' ', index);
      if (nextSpace == std::string::npos)
        nextSpace = msg.size();

      const auto token = msg.substr(index, nextSpace - index);
      if (token.empty())
        continue;
      if (token.front() == '@' && tags.empty() && !source && command.empty())
      {
        auto tagsSs = std::istringstream{token.substr(1)};
        auto tag = std::string{};
        while (tagsSs)
        {
          std::getline(tagsSs, tag, ';');
          const auto pos = tag.find('=');
          tags.push_back({tag.substr(0, pos), tag.substr(pos + 1)});
        }
      }
      else if (token.front() == ':' && !source && command.empty())
        source = token.substr(1);
      else if (command.empty())
        command = token;
      else
      {
        if (token.front() == ':')
        {
          parameters.push_back(msg.substr(index + 1));
          break;
        }
        parameters.push_back(token);
      }
    }
    if (command == "PRIVMSG")
    {
      if (parameters.size() < 2)
      {
        LOG("Expected 2 parameters on PRIVMSG");
        return;
      }
      auto privMsg = parameters[1];
      auto displayName = std::string{"noname"};
      auto color = glm::vec3{0.f, 0.f, 0.f};
      auto isFirst = false;
      auto isMod = false;
      auto subscriber = -1;
      for (const auto &tag : tags)
      {
        if (tag.first == "display-name")
        {
          displayName = tag.second;
          continue;
        }
        if (tag.first == "color")
        {
          color = [](const std::string &hexString) {
            if (hexString.size() != 7 || hexString[0] != '#')
            {
              return glm::vec3{0.f, 0.f, 0.f};
            }

            int r, g, b;
            std::istringstream(hexString.substr(1, 2)) >> std::hex >> r;
            std::istringstream(hexString.substr(3, 2)) >> std::hex >> g;
            std::istringstream(hexString.substr(5, 2)) >> std::hex >> b;

            return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
          }(tag.second);
          continue;
        }
        if (tag.first == "first-msg")
        {
          isFirst = tag.second == "1";
          continue;
        }
        if (tag.first == "mod")
        {
          isMod = tag.second == "1";
          continue;
        }
        if (tag.first == "subscriber")
        {
          subscriber = atoi(tag.second.c_str());
          continue;
        }
      }
      for (auto sink : sinks)
        sink.get().onMsg({displayName, privMsg, color, isFirst, isMod, subscriber});
      return;
    }

    if (command == RPL_WELCOME)
      onWelcome();
    else if (command == "PING")
    {
      if (parameters.empty())
      {
        LOG("Parameters on PING message are empty");
        return;
      }
      onPing(parameters[0]);
    }
  }
}

auto Twitch::onPing(const std::string &val) -> void
{
  std::ostringstream sendMsg;
  sendMsg << "PONG " << val << "\r\n";
  auto s = tcp.write(sendMsg.str(), [this](int status) {
    if (status < 0)
    {
      LOG(__func__, "error:", uv_err_name(status));
      initiateRetry();
      return;
    }
  });
  if (s < 0)
  {
    LOG(__func__, "error:", uv_err_name(s));
    initiateRetry();
    return;
  }
}

auto Twitch::onWelcome() -> void
{
  std::ostringstream msg;
  msg << "CAP REQ :twitch.tv/tags\r\n"
      << "CAP REQ :twitch.tv/tags twitch.tv/commands\r\n"
      << "JOIN #" << channel << "\r\n";
  auto s = tcp.write(msg.str(), [this](int status) {
    if (status < 0)
    {
      LOG(__func__, "error:", uv_err_name(status));
      initiateRetry();
      return;
    }
  });
  if (s < 0)
  {
    LOG(__func__, "error:", uv_err_name(s));
    initiateRetry();
    return;
  }
  LOG("Connected to twitch");
  state = State::connected;
  initRetry = 1000;
}

auto Twitch::reg(TwitchSink &v) -> void
{
  sinks.push_back(v);
}

auto Twitch::unreg(TwitchSink &v) -> void
{
  sinks.erase(
    std::remove_if(std::begin(sinks), std::end(sinks), [&](const auto &x) { return &x.get() == &v; }),
    std::end(sinks));
}

auto Twitch::initiateRetry() -> void
{
  state = State::connecting;
  retry.start([this]() { init(); }, initRetry);
  LOG("Retry in ", initRetry);
  initRetry = std::min(32000, initRetry * 2);
}

auto Twitch::updateUserKey(const std::string &aUser, const std::string &aKey) -> void
{
  if (user == aUser && key == aKey)
    return;
  user = aUser;
  key = aKey;
  init();
}

auto Twitch::isConnected() const -> bool
{
  return state == State::connected;
}

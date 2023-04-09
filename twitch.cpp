#include "twitch.hpp"
#include <algorithm>
#include <log/log.hpp>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{
  class NetInitializer
  {
  public:
    NetInitializer()
    {
      const auto s = SDLNet_Init();
      if (s < 0)
      {
        std::ostringstream ss;
        ss << "Network initializetion failed: " << s;
        throw std::runtime_error(ss.str());
      }
    }
    ~NetInitializer() { SDLNet_Quit(); }
    static auto init() -> void { static NetInitializer inst; }
  };

  const char *RPL_WELCOME = "001";
  const char *RPL_YOURHOST = "002";
  const char *RPL_CREATED = "003";
  const char *RPL_MYINFO = "004";
  const char *RPL_ISUPPORT = "005";
  const char *RPL_BOUNCE = "010";
  const char *RPL_UMODEIS = "221";
  const char *RPL_LUSERCLIENT = "251";
  const char *RPL_LUSEROP = "252";
  const char *RPL_LUSERUNKNOWN = "253";
  const char *RPL_LUSERCHANNELS = "254";
  const char *RPL_LUSERME = "255";
  const char *RPL_ADMINME = "256";
  const char *RPL_ADMINLOC1 = "257";
  const char *RPL_ADMINLOC2 = "258";
  const char *RPL_ADMINEMAIL = "259";
  const char *RPL_TRYAGAIN = "263";
  const char *RPL_LOCALUSERS = "265";
  const char *RPL_GLOBALUSERS = "266";
  const char *RPL_WHOISCERTFP = "276";
  const char *RPL_NONE = "300";
  const char *RPL_AWAY = "301";
  const char *RPL_USERHOST = "302";
  const char *RPL_UNAWAY = "305";
  const char *RPL_NOWAWAY = "306";
  const char *RPL_WHOREPLY = "352";
  const char *RPL_ENDOFWHO = "315";
  const char *RPL_WHOISREGNICK = "307";
  const char *RPL_WHOISUSER = "311";
  const char *RPL_WHOISSERVER = "312";
  const char *RPL_WHOISOPERATOR = "313";
  const char *RPL_WHOWASUSER = "314";
  const char *RPL_WHOISIDLE = "317";
  const char *RPL_ENDOFWHOIS = "318";
  const char *RPL_WHOISCHANNELS = "319";
  const char *RPL_WHOISSPECIAL = "320";
  const char *RPL_LISTSTART = "321";
  const char *RPL_LIST = "322";
  const char *RPL_LISTEND = "323";
  const char *RPL_CHANNELMODEIS = "324";
  const char *RPL_CREATIONTIME = "329";
  const char *RPL_WHOISACCOUNT = "330";
  const char *RPL_NOTOPIC = "331";
  const char *RPL_TOPIC = "332";
  const char *RPL_TOPICWHOTIME = "333";
  const char *RPL_INVITELIST = "336";
  const char *RPL_ENDOFINVITELIST = "337";
  const char *RPL_WHOISACTUALLY = "338";
  const char *RPL_INVITING = "341";
  const char *RPL_INVEXLIST = "346";
  const char *RPL_ENDOFINVEXLIST = "347";
  const char *RPL_EXCEPTLIST = "348";
  const char *RPL_ENDOFEXCEPTLIST = "349";
  const char *RPL_VERSION = "351";
  const char *RPL_NAMREPLY = "353";
  const char *RPL_ENDOFNAMES = "366";
  const char *RPL_LINKS = "364";
  const char *RPL_ENDOFLINKS = "365";
  const char *RPL_BANLIST = "367";
  const char *RPL_ENDOFBANLIST = "368";
  const char *RPL_ENDOFWHOWAS = "369";
  const char *RPL_INFO = "371";
  const char *RPL_ENDOFINFO = "374";
  const char *RPL_MOTDSTART = "375";
  const char *RPL_MOTD = "372";
  const char *RPL_ENDOFMOTD = "376";
  const char *RPL_WHOISHOST = "378";
  const char *RPL_WHOISMODES = "379";
  const char *RPL_YOUREOPER = "381";
  const char *RPL_REHASHING = "382";
  const char *RPL_TIME = "391";
  const char *ERR_UNKNOWNERROR = "400";
  const char *ERR_NOSUCHNICK = "401";
  const char *ERR_NOSUCHSERVER = "402";
  const char *ERR_NOSUCHCHANNEL = "403";
  const char *ERR_CANNOTSENDTOCHAN = "404";
  const char *ERR_TOOMANYCHANNELS = "405";
  const char *ERR_WASNOSUCHNICK = "406";
  const char *ERR_NOORIGIN = "409";
  const char *ERR_INPUTTOOLONG = "417";
  const char *ERR_UNKNOWNCOMMAND = "421";
  const char *ERR_NOMOTD = "422";
  const char *ERR_ERRONEUSNICKNAME = "432";
  const char *ERR_NICKNAMEINUSE = "433";
  const char *ERR_USERNOTINCHANNEL = "441";
  const char *ERR_NOTONCHANNEL = "442";
  const char *ERR_USERONCHANNEL = "443";
  const char *ERR_NOTREGISTERED = "451";
  const char *ERR_NEEDMOREPARAMS = "461";
  const char *ERR_ALREADYREGISTERED = "462";
  const char *ERR_PASSWDMISMATCH = "464";
  const char *ERR_YOUREBANNEDCREEP = "465";
  const char *ERR_CHANNELISFULL = "471";
  const char *ERR_UNKNOWNMODE = "472";
  const char *ERR_INVITEONLYCHAN = "473";
  const char *ERR_BANNEDFROMCHAN = "474";
  const char *ERR_BADCHANNELKEY = "475";
  const char *ERR_BADCHANMASK = "476";
  const char *ERR_NOPRIVILEGES = "481";
  const char *ERR_CHANOPRIVSNEEDED = "482";
  const char *ERR_CANTKILLSERVER = "483";
  const char *ERR_NOOPERHOST = "491";
  const char *ERR_UMODEUNKNOWNFLAG = "501";
  const char *ERR_USERSDONTMATCH = "502";
  const char *ERR_HELPNOTFOUND = "524";
  const char *ERR_INVALIDKEY = "525";
  const char *RPL_STARTTLS = "670";
  const char *RPL_WHOISSECURE = "671";
  const char *ERR_STARTTLS = "691";
  const char *ERR_INVALIDMODEPARAM = "696";
  const char *RPL_HELPSTART = "704";
  const char *RPL_HELPTXT = "705";
  const char *RPL_ENDOFHELP = "706";
  const char *ERR_NOPRIVS = "723";
  const char *RPL_LOGGEDIN = "900";
  const char *RPL_LOGGEDOUT = "901";
  const char *ERR_NICKLOCKED = "902";
  const char *RPL_SASLSUCCESS = "903";
  const char *ERR_SASLFAIL = "904";
  const char *ERR_SASLTOOLONG = "905";
  const char *ERR_SASLABORTED = "906";
  const char *ERR_SASLALREADY = "907";
  const char *RPL_SASLMECHS = "908";
} // namespace

static const char *server = "irc.chat.twitch.tv";
static const int port = 6667;

Twitch::Twitch(std::string aKey, std::string aUser, std::string aChannel)
  : key(std::move(aKey)), user(std::move(aUser)), channel(std::move(aChannel))
{
  NetInitializer::init();
  IPaddress ip;
  if (SDLNet_ResolveHost(&ip, server, port) < 0)
  {
    std::ostringstream ss;
    ss << "SDLNet_ResolveHost() failed: " << SDLNet_GetError();
    LOG(ss.str());
    throw std::runtime_error(ss.str());
  }

  socket = SDLNet_TCP_Open(&ip);
  if (!socket)
  {
    std::ostringstream ss;
    ss << "SDLNet_TCP_Open() failed: " << SDLNet_GetError();
    LOG(ss.str());
    throw std::runtime_error(ss.str());
  }

  {
    std::ostringstream msg;
    msg << "PASS " << key << "\r\n";
    const auto sz = static_cast<int>(msg.str().size());
    if (SDLNet_TCP_Send(socket, msg.str().c_str(), sz) < sz)
    {
      std::ostringstream ss;
      ss << "SDLNet_TCP_Send() failed: " << SDLNet_GetError() << std::endl;
      LOG(ss.str());
      SDLNet_TCP_Close(socket);
      socket = nullptr;
      throw std::runtime_error(ss.str());
    }
  }

  {
    std::ostringstream msg;
    msg << "NICK " << user << "\r\n";
    const auto sz = static_cast<int>(msg.str().size());
    if (SDLNet_TCP_Send(socket, msg.str().c_str(), sz) < sz)
    {
      std::ostringstream ss;
      ss << "SDLNet_TCP_Send() failed: " << SDLNet_GetError();
      LOG(ss.str());
      SDLNet_TCP_Close(socket);
      socket = nullptr;
      throw std::runtime_error(ss.str());
    }
  }

  {
    const char *msg = "USER nobody unknown unknown :noname\r\n";
    if (SDLNet_TCP_Send(socket, msg, strlen(msg)) < static_cast<int>(strlen(msg)))
    {
      std::ostringstream ss;
      ss << "SDLNet_TCP_Send() failed: " << SDLNet_GetError();
      LOG(ss.str());
      SDLNet_TCP_Close(socket);
      socket = nullptr;
      throw std::runtime_error(ss.str());
    }
  }
  socketSet = SDLNet_AllocSocketSet(1);
  SDLNet_TCP_AddSocket(socketSet, socket);
}

Twitch::~Twitch()
{
  if (socketSet)
    SDLNet_FreeSocketSet(socketSet);
  if (socket)
    SDLNet_TCP_Close(socket);
}

auto Twitch::tick() -> void
{
  auto numReady = SDLNet_CheckSockets(socketSet, 0);
  if (numReady < 0)
  {
    std::ostringstream ss;
    ss << "SDLNet_CheckSockets() failed: " << SDLNet_GetError();
    LOG(ss.str());
    throw std::runtime_error(ss.str());
  }
  if (!SDLNet_SocketReady(socket))
    return;
  char buffer[1024];
  const auto bytesReceived = SDLNet_TCP_Recv(socket, buffer, sizeof(buffer));
  if (bytesReceived <= 0)
  {
    std::ostringstream ss;
    ss << "SDLNet_TCP_Recv() failed: \"" << SDLNet_GetError() << "\" bytesReceive: " << bytesReceived;
    LOG(ss.str());
    throw std::runtime_error(ss.str());
  }
  buf.insert(std::end(buf), buffer, buffer + bytesReceived);
  for (;;)
  {
    auto it = std::find(std::begin(buf), std::end(buf), '\n');
    if (it == std::end(buf) || it == std::begin(buf) || *(it - 1) != '\r')
      return;
    auto msg = std::string{std::begin(buf), it - 1};
    buf.erase(std::begin(buf), it + 1);
    LOG("msg", msg);
    auto tags = std::vector<std::string>{};
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
        tags.push_back(token.substr(1));
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
    for (const auto &tag : tags)
      LOG("tag", tag);
    LOG("source", source ? *source : "no-source");
    LOG("command", command);
    for (const auto &parameter : parameters)
      LOG("parameter", parameter);
    if (command == RPL_WELCOME)
    {
      const char *msg = "CAP REQ :twitch.tv/tags\r\n";
      if (SDLNet_TCP_Send(socket, msg, strlen(msg)) < static_cast<int>(strlen(msg)))
      {
        std::ostringstream ss;
        ss << "SDLNet_TCP_Send() failed: " << SDLNet_GetError();
        LOG(ss.str());
        SDLNet_TCP_Close(socket);
        socket = nullptr;
        throw std::runtime_error(ss.str());
      }
      {
        const char *msg = "CAP REQ :twitch.tv/tags twitch.tv/commands\r\n";
        if (SDLNet_TCP_Send(socket, msg, strlen(msg)) < static_cast<int>(strlen(msg)))
        {
          std::ostringstream ss;
          ss << "SDLNet_TCP_Send() failed: " << SDLNet_GetError();
          LOG(ss.str());
          SDLNet_TCP_Close(socket);
          socket = nullptr;
          throw std::runtime_error(ss.str());
        }
      }

      {
        std::ostringstream msg;
        msg << "JOIN #" << channel << "\r\n";
        const auto sz = static_cast<int>(msg.str().size());
        if (SDLNet_TCP_Send(socket, msg.str().c_str(), sz) < sz)
        {
          std::ostringstream ss;
          ss << "SDLNet_TCP_Send() failed: " << SDLNet_GetError();
          LOG(ss.str());
          SDLNet_TCP_Close(socket);
          socket = nullptr;
          throw std::runtime_error(ss.str());
        }
      }
    }
  }
}

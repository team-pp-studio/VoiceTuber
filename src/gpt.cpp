#include "gpt.hpp"
#include "http-client.hpp"
#include "uv.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <json/json.hpp>
#include <log/log.hpp>
#include <numeric>
#include <sstream>

Gpt::Gpt(uv::Uv &uv, std::string aToken, HttpClient &aHttpClient)
  : alive(std::make_shared<int>()),
    timer(uv.createTimer()),
    token(std::move(aToken)),
    httpClient(aHttpClient),
    lastReply(std::chrono::high_resolution_clock::now())
{
}

static auto esc(const std::string &str) -> std::string
{
  std::string result;
  for (auto c : str)
  {
    switch (c)
    {
    case '"': result += "\\\""; break;
    case '\\': result += "\\\\"; break;
    case '\b': result += "\\b"; break;
    case '\f': result += "\\f"; break;
    case '\n': result += "\\n"; break;
    case '\r': result += "\\r"; break;
    case '\t': result += "\\t"; break;
    default: result += c; break;
    }
  }
  return result;
}

static auto stripWhiteSpaces(std::string v) -> std::string
{
  v.erase(v.begin(), std::find_if(v.begin(), v.end(), [](auto c) { return !std::isspace(c); }));
  v.erase(std::find_if(v.rbegin(), v.rend(), [](auto c) { return !std::isspace(c); }).base(), v.end());
  return v;
}

static auto stripHangingSentences(std::string v) -> std::string
{
  const auto posDot = v.rfind('.');
  const auto posExclamation = v.rfind('!');
  const auto posQuestion = v.rfind('?');

  auto pos = std::string::npos;
  for (std::size_t p : {posDot, posExclamation, posQuestion})
    if (p != std::string::npos && (pos == std::string::npos || p > pos))
      pos = p;

  return (pos != std::string::npos) ? v.substr(0, pos + 1) : v;
}

auto Gpt::prompt(std::string name, std::string p, Cb cb) -> void
{
  p = stripWhiteSpaces(p);
  queuedMsgs.emplace_back(std::make_pair(Msg{std::move(name), std::move(p)}, std::move(cb)));
  using namespace std::chrono_literals;
  if (state == State::waiting || std::chrono::high_resolution_clock::now() < lastReply + 3s)
    return;
  process();
}

auto Gpt::process() -> void
{
  if (state == State::waiting)
    return;
  if (queuedMsgs.empty())
    return;
  state = State::waiting;
  std::ostringstream ss;
  const auto embedName = (rand() % 5 == 0) || msgs.empty();
  ss << R"({
    "model": "text-curie-001",
    "prompt": ")";
  ss << esc(systemPrompt_);
  for (const auto &msg : msgs)
    ss << esc("\n| ") << esc(msg.name) << ": " << esc(msg.msg);
  for (const auto &msg : queuedMsgs)
  {
    ss << esc("\n| ") << esc(msg.first.name) << ": " << esc(msg.first.msg);
    msgs.emplace_back(std::move(msg.first));
  }

  if (embedName)
    ss << R"(\n| )" << cohost_ << R"(:)";
  else
    ss << R"(\n|)";
  ss
    << R"(", "temperature": 1, "max_tokens": 24, "top_p": 1.0, "frequency_penalty": 0.5, "presence_penalty": 0.6, "stop": ["\n| "]})";

  httpClient.get().post("https://api.openai.com/v1/completions",
                        ss.str(),
                        [embedName,
                         qMsgs = std::move(queuedMsgs),
                         jsonPrompt = ss.str(),
                         alive = std::weak_ptr<int>(alive),
                         this](CURLcode code, long httpStatus, std::string payload) {
                          if (!alive.lock())
                          {
                            LOG("this was destroyed");
                            return;
                          }
                          if (code != CURLE_OK)
                          {
                            for (const auto &msg : qMsgs)
                              msg.second("");
                            state = State::idle;
                            return;
                          }
                          if (httpStatus >= 400 && httpStatus < 500)
                          {
                            LOG(code, httpStatus, payload);
                            LOG(jsonPrompt);
                            lastError = payload;
                            for (const auto &msg : qMsgs)
                              msg.second("");
                            state = State::idle;
                            return;
                          }
                          if (httpStatus != 200)
                          {
                            LOG(code, httpStatus, payload);
                            LOG(jsonPrompt);
                            lastError = payload;
                            for (const auto &msg : qMsgs)
                              msg.second("");
                            timer.start(
                              [alive = std::weak_ptr<int>(alive), this]() {
                                if (!alive.lock())
                                {
                                  LOG("this was destroyed");
                                  return;
                                }
                                state = State::idle;
                                process();
                              },
                              10'000);
                            return;
                          }
                          lastError = "";
                          const auto j = json::Root{std::move(payload)};
                          // {
                          //   "id": "cmpl-7PJIyTy1pXJD3OvzekQOQnqOdcUF5",
                          //   "object": "text_completion",
                          //   "created": 1686266764,
                          //   "model": "text-davinci-003",
                          //   "choices": [
                          //     {
                          //       "text": " Is this thing on?\n",
                          //       "index": 0,
                          //       "logprobs": null,
                          //       "finish_reason": "stop"
                          //     }
                          //   ],
                          //   "usage": {
                          //     "prompt_tokens": 58,
                          //     "completion_tokens": 6,
                          //     "total_tokens": 64
                          //   }
                          // }
                          auto choices = j("choices");
                          if (choices.empty())
                          {
                            LOG(code, httpStatus, payload);
                            lastError = "0 Choices";
                            for (const auto &msg : qMsgs)
                              msg.second("");
                            state = State::idle;
                            process();
                          }
                          auto cohostMsg =
                            stripHangingSentences(stripWhiteSpaces(choices[0]("text").asStr()));
                          if (!embedName)
                          {
                            if (cohostMsg.find(cohost_ + std::string{":"}) != 0)
                            {
                              for (const auto &msg : qMsgs)
                                msg.second("");
                              state = State::idle;
                              process();
                              return;
                            }
                            cohostMsg = stripWhiteSpaces(cohostMsg.substr(cohost_.size() + 1));
                          }

                          {
                            Msg msg;
                            msg.name = cohost_;
                            msg.msg = cohostMsg;
                            msgs.emplace_back(std::move(msg));
                          }
                          const auto MaxTokens = 2048 * 4 / 10;
                          const auto initWords = countWords();
                          for (auto words = initWords; words > MaxTokens;)
                          {
                            words -= countWords(msgs.front());
                            msgs.pop_front();
                          }
                          for (auto i = 0U; i < qMsgs.size(); ++i)
                          {
                            if (i == 0)
                              qMsgs[i].second(cohostMsg);
                            else
                              qMsgs[i].second("");
                          }
                          state = State::idle;
                          lastReply = std::chrono::high_resolution_clock::now();
                          process();
                        },
                        {{"Content-Type", "application/json"}, {"Authorization", "Bearer " + token}});
  queuedMsgs.clear();
}

auto Gpt::updateToken(std::string aToken) -> void
{
  token = std::move(aToken);
}

auto Gpt::countWords() const -> int
{
  return std::accumulate(
    std::begin(msgs), std::end(msgs), 0, [this](int a, const auto &msg) { return a + countWords(msg); });
}
auto Gpt::countWords(const Msg &msg) const -> int
{
  std::istringstream ss(msg.msg);
  std::string word;
  auto cnt = 0;
  while (std::getline(ss, word, ' '))
    ++cnt;
  return cnt;
}

auto Gpt::systemPrompt(std::string v) -> void
{
  systemPrompt_ = std::move(v);
}

auto Gpt::systemPrompt() const -> const std::string &
{
  return systemPrompt_;
}

auto Gpt::cohost(std::string v) -> void
{
  cohost_ = std::move(v);
}

auto Gpt::cohost() const -> std::string
{
  return cohost_;
}

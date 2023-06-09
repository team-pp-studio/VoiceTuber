#include "gpt.hpp"
#include "http-client.hpp"
#include "uv.hpp"
#include <algorithm>
#include <cctype>
#include <functional>
#include <json/json.hpp>
#include <log/log.hpp>
#include <numeric>
#include <sstream>

//  {
//    "model": "gpt-3.5-turbo",
//    "messages": [{"role": "user", "name":"host","content": "Hello!"}]
//  }

Gpt::Gpt(uv::Uv &uv, std::string aToken, std::string aSystemPrompt, HttpClient &aHttpClient)
  : timer(uv.createTimer()),
    token(std::move(aToken)),
    systemPrompt(std::move(aSystemPrompt)),
    httpClient(aHttpClient)
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
  std::size_t posDot = v.rfind('.');
  std::size_t posExclamation = v.rfind('!');
  std::size_t posQuestion = v.rfind('?');

  std::size_t pos = std::string::npos;

  for (std::size_t p : {posDot, posExclamation, posQuestion})
    if (p != std::string::npos && (pos == std::string::npos || p > pos))
      pos = p;

  if (pos != std::string::npos)
    return v.substr(0, pos + 1);
  else
    return v;
}

auto Gpt::prompt(std::string name, std::string p, Cb cb) -> void
{
  p = stripWhiteSpaces(p);
  queue.emplace([cb = std::move(cb), name = std::move(name), p = std::move(p), this](
                  PostTask postTask) mutable {
    std::ostringstream ss;
    ss << R"({
    "model": "text-davinci-003",
    "prompt": ")";
    ss << esc(systemPrompt);
    for (const auto &msg : msgs)
      ss << esc("\n| ") << esc(msg.name) << ": " << esc(msg.msg);
    ss << esc("\n| ") << esc(name) << ": " << esc(p);
    ss
      << R"(\n| Co-host:", "temperature": 1, "max_tokens": 20, "top_p": 1.0, "frequency_penalty": 0.5, "presence_penalty": 0.6, "stop": ["\n| "]})";

    httpClient.get().post(
      "https://api.openai.com/v1/completions",
      ss.str(),
      [cb = std::move(cb),
       name = std::move(name),
       p = std::move(p),
       postTask = std::move(postTask),
       this](CURLcode code, long httpStatus, std::string payload) {
        if (code != CURLE_OK)
        {
          cb("");
          postTask(false);
          return;
        }
        if (httpStatus >= 400 && httpStatus < 500)
        {
          LOG(code, httpStatus, payload);
          lastError = payload;
          cb("");
          postTask(true);
          return;
        }
        if (httpStatus != 200)
        {
          LOG(code, httpStatus, payload);
          lastError = payload;
          cb("");
          timer.start([postTask = std::move(postTask)]() { postTask(false); }, 10'000);
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
          cb("");
          postTask(true);
        }
        const auto cohostMsg = stripHangingSentences(stripWhiteSpaces(choices[0]("text").asStr()));

        cb(cohostMsg);
        {
          Msg msg;
          msg.name = name;
          msg.msg = p;
          msgs.emplace_back(std::move(msg));
        }
        {
          Msg msg;
          msg.name = "Co-host";
          msg.msg = cohostMsg;
          msgs.emplace_back(std::move(msg));
        }
        const auto MaxTokens = 4097 * 5 / 10;
        const auto initWords = countWords();
        for (auto words = initWords; words > MaxTokens;)
        {
          words -= countWords(msgs.front());
          msgs.pop_front();
        }
        postTask(true);
      },
      {{"Content-Type", "application/json"}, {"Authorization", "Bearer " + token}});
  });
  process();
}

auto Gpt::process() -> void
{
  if (state == State::waiting)
    return;
  if (queue.empty())
  {
    state = State::idle;
    return;
  }
  state = State::waiting;
  queue.front()([this](bool r) {
    timer.start([this, r]() {
      if (r)
        queue.pop();
      state = State::idle;
      process();
    });
  });
}

auto Gpt::updateToken(std::string aToken) -> void
{
  token = std::move(aToken);
}

auto Gpt::queueSize() const -> int
{
  return static_cast<int>(queue.size());
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

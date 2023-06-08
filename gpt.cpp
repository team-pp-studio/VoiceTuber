#include "gpt.hpp"
#include "http-client.hpp"
#include "uv.hpp"
#include <json/json.hpp>
#include <log/log.hpp>
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

auto Gpt::prompt(std::string name, std::string p, Cb cb) -> void
{
  queue.emplace(
    [cb = std::move(cb), name = std::move(name), p = std::move(p), this](PostTask postTask) mutable {
      std::ostringstream ss;
      ss << R"({
    "model": "gpt-3.5-turbo",
    "messages": [)";
      ss << R"({"role": "system", "content": ")" << esc(systemPrompt) << R"("},)";
      for (const auto &msg : msgs)
        ss << R"({"role": ")" << esc(msg.role) << R"(", "name": ")" << esc(msg.name)
           << R"(", "content": ")" << esc(msg.msg) << R"("},
)";
      ss << R"({"role": "user", "name": ")" << esc(name) << R"(", "content": ")" << esc(p) << R"("})";
      ss << "]}";

      httpClient.get().post(
        "https://api.openai.com/v1/chat/completions",
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
          //   "id": "chatcmpl-123",
          //   "object": "chat.completion",
          //   "created": 1677652288,
          //   "choices": [{
          //     "index": 0,
          //     "message": {
          //       "role": "assistant",
          //       "content": "\n\nHello there, how may I assist you today?",
          //     },
          //     "finish_reason": "stop"
          //   }],
          //   "usage": {
          //     "prompt_tokens": 9,
          //     "completion_tokens": 12,
          //     "total_tokens": 21
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
          cb(choices[0]("message")("content").asStr());
          {
            Msg msg;
            msg.role = "user";
            msg.name = name;
            msg.msg = p;
            msgs.emplace_back(std::move(msg));
          }
          {
            Msg msg;
            msg.role = choices[0]("message")("role").asStr();
            msg.name = "Clara";
            msg.msg = choices[0]("message")("content").asStr();
            msgs.emplace_back(std::move(msg));
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

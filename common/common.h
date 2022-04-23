#pragma once

#include <asio.hpp>
#include <iostream>
#include <functional>
#include <chrono>
#include <thread>
#include <optional>
#include <unordered_set>
#include <queue>
#include <string>

#include "ChatMessage.pb.h"

#define SERVER_LISTENING_PORT 15001
#define MAX_STREAM_BUFFER_SIZE 65536

namespace ip = asio::ip;
using tcp = ip::tcp;
using udp = ip::udp;

template<typename... T>
std::string join(T&&... args)
{
    std::ostringstream ss;
    (ss << ... << args);
    return ss.str();
}

inline std::string format_message(const proto::ChatMessage& message)
{
    return join("[", message.sender(), "] ", message.content());
}
#include "StatusServiceImpl.h"
#include "../global/ConfigManager.h"
#include "../global/const.h"
#include "../redis/RedisManager.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <mutex>
#include <spdlog/spdlog.h>

std::string generate_unique_string() {
    boost::uuids::random_generator gen;
    boost::uuids::uuid uuid = gen();
    return boost::uuids::to_string(uuid);
}

grpc::Status
StatusServiceImpl::GetChatServer(grpc::ServerContext *context,
                                 const message::GetChatServerRequest *request,
                                 message::GetChatServerResponse *response) {
    std::string prefix("ChatServer received :");
    const auto &server = GetChatServer();
    response->set_host(server.host);
    response->set_port(server.port);
    response->set_error(static_cast<int>(ErrorCodes::SUCCESS));
    response->set_token(generate_unique_string());
    insertToken(request->uid(), response->token());
    SPDLOG_INFO("{} uid:{}, token:{}, host:{}, port:{}", prefix, request->uid(),
                response->token(), server.host, server.port);
    return grpc::Status::OK;
}

void StatusServiceImpl::insertToken(int uid, const std::string &token) {
    std::string token_key = USER_TOKEN_PREFIX + std::to_string(uid);
    RedisManager::GetInstance()->Set(token_key, token);
}

ChatServer StatusServiceImpl::GetChatServer() {
    std::lock_guard<std::mutex> lock(_server_mutex);
    auto minServer = _servers.begin()->second;

    auto count_str =
        RedisManager::GetInstance()->HGet(LOGIN_COUNT_PREFIX, minServer.name);
    if (count_str.empty()) {
        minServer.con_count = INT_MAX;
    } else {
        minServer.con_count = std::stoi(count_str);
    }

    for (auto &[name, server] : _servers) {
        if (name == minServer.name) {
            continue;
        }

        auto count_str =
            RedisManager::GetInstance()->HGet(LOGIN_COUNT_PREFIX, name);
        if (count_str.empty()) {
            server.con_count = INT_MAX;
        } else {
            server.con_count = std::stoi(count_str);
        }

        if (server.con_count < minServer.con_count) {
            minServer = server;
        }
    }
    return minServer;
}

grpc::Status StatusServiceImpl::Login(grpc::ServerContext *context,
                                      const message::LoginRequest *request,
                                      message::LoginResponse *response) {
    auto uid = request->uid();
    auto token = request->token();

    std::string uid_str = std::to_string(uid);
    std::string token_key = USER_TOKEN_PREFIX + uid_str;
    std::string token_value = "";
    bool success = RedisManager::GetInstance()->Get(token_key, token_value);
    if (success) {
        response->set_error(static_cast<int>(ErrorCodes::ERROR_UID_INVALID));
        return grpc::Status::OK;
    } else {
        response->set_error(static_cast<int>(ErrorCodes::ERROR_TOKEN_INVALID));
        return grpc::Status::OK;
    }

    response->set_error(static_cast<int>(ErrorCodes::SUCCESS));
    response->set_uid(uid);
    response->set_token(token);
    return grpc::Status::OK;
}

StatusServiceImpl::StatusServiceImpl() {
    auto &cfg = ConfigManager::GetInstance();
    auto server_list = cfg["ChatServers"]["name"];

    std::vector<std::string> words;
    words.reserve(10);

    std::stringstream ss(server_list);
    std::string word;

    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    }

    for (auto &word : words) {
        if (cfg[word]["name"].empty()) {
            continue;
        }

        ChatServer server;
        server.host = cfg[word]["host"];
        server.port = cfg[word]["port"];
        server.name = cfg[word]["name"];
        _servers[server.name] = server;
    }
    SPDLOG_INFO("size:{}", _servers.size());
}

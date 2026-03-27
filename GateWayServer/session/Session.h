#ifndef SESSION_H
#define SESSION_H

#include "../global/const.h"
#include <memory>

class Session : public std::enable_shared_from_this<Session> {
    friend class LogicSystem;

public:
    Session(boost::asio::io_context& ioc);
    void Start();
    net::ip::tcp::socket& GetSocket();

private:
    void CheckDeadLine();
    void DeadlineCancel();
    void WriteResponse();
    void ParseGetParam();
    void HandleRequest();
    void HandleHeaders(bool success, const std::string& type = "text/plain", const std::string& body = "");
    
private:
    // 套接字
    net::ip::tcp::socket _socket;
    // 定时器
    net::steady_timer _deadlineTimer { _socket.get_executor(), std::chrono::seconds(30) };
    // 缓冲区
    beast::flat_buffer _buffer { 8192 };
    // 请求
    http::request<http::dynamic_body> _request;
    // 响应
    http::response<http::dynamic_body> _response;

    std::string _get_url;
    std::unordered_map<std::string, std::string> _get_params;
};

#endif
#include "../session/Session.h"
#include "../server/LogicSystem.h"

// 10进制转16进制（字符串形式）
unsigned char ToHex(int x) { return x > 9 ? x + 55 : x + 48; }

// 16进制字符串转10进制
unsigned char FromHex(unsigned char x) {
    unsigned char y;
    if (x >= '0' && x <= '9')
        y = x - '0';
    else if (x >= 'a' && x <= 'f')
        y = x - 'a' + 10;
    else if (x >= 'A' && x <= 'F')
        y = x - 'A' + 10;
    else
        assert(0);
    return y;
}

// 编码
std::string UrlEncode(const std::string &str) {
    std::string temp = "";
    std::size_t length = str.size();
    for (std::size_t i = 0; i < length; ++i) {
        if (std::isalnum(static_cast<unsigned char>(str[i])) || str[i] == '=' ||
            (str[i] == '-') || (str[i] == '_') || (str[i] == '.') ||
            (str[i] == '~'))
            temp += str[i];
        else if (str[i] == ' ') {
            temp += '+';
        } else {
            temp += '%';
            temp += ToHex((unsigned char)str[i] >> 4);
            temp += ToHex((unsigned char)str[i] & 0x0F);
        }
    }
    return temp;
}

// 解析
std::string UrlDecode(const std::string &str) {
    std::string temp = "";
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '+')
            temp += ' ';
        else if (str[i] == '%') {
            assert(i + 2 < str.size());
            unsigned char high = FromHex(str[++i]);
            unsigned char low = FromHex(str[++i]);
            temp += ((high << 4) + low);
        } else
            temp += str[i];
    }
    return temp;
}

Session::Session(boost::asio::io_context &ioc)
    : _socket(ioc) {}

void Session::Start() {

    http::async_read(
        _socket, _buffer, _request,
        [self = shared_from_this()](const boost::system::error_code &ec,
                                    std::size_t bytes_transferred) {
            try {
                if (ec) {
                    SPDLOG_ERROR("Error reading request: {}", ec.message());
                    return;
                }
                boost::ignore_unused(bytes_transferred);
                self->HandleRequest();
                self->CheckDeadLine();

            } catch (std::exception &e) {
                SPDLOG_ERROR("Exception: {}", e.what());
            }
        });
}

net::ip::tcp::socket &Session::GetSocket() { return _socket; }

void Session::CheckDeadLine() {
    auto self = shared_from_this();
    _deadlineTimer.async_wait(
        [self](beast::error_code ec) { ec = self->_socket.close(ec); });
}

void Session::DeadlineCancel() { _deadlineTimer.cancel(); }

void Session::WriteResponse() {
    http::async_write(
        _socket, _response,
        [self = shared_from_this()](boost::system::error_code ec,
                                    std::size_t bytes_transferred) {
            ec =
                self->_socket.shutdown(net::ip::tcp::socket::shutdown_send, ec);
            self->DeadlineCancel();
        });
}

void Session::ParseGetParam() {
    auto url = _request.target();
    auto query_pos = url.find('?');
    if (query_pos == std::string::npos) {
        _get_url = url;
        return;
    }

    _get_url = url.substr(0, query_pos);
    std::string query_string = url.substr(query_pos + 1);
    std::string key;
    std::string value;
    std::size_t pos;
    while ((pos = query_string.find('&')) != std::string::npos) {
        auto pair = query_string.substr(0, pos);
        std::size_t equal_pos = pair.find('=');
        if (equal_pos != std::string::npos) {
            key = UrlDecode(pair.substr(0, equal_pos));
            value = UrlDecode(pair.substr(equal_pos + 1));
            _get_params[key] = value;
        }
        query_string.erase(0, pos + 1);
    }
    if (!query_string.empty()) {
        std::size_t equal_pos = query_string.find('=');
        if (equal_pos != std::string::npos) {
            key = UrlDecode(query_string.substr(0, equal_pos));
            value = UrlDecode(query_string.substr(equal_pos + 1));
            _get_params[key] = value;
        }
    }
}

void Session::HandleRequest() {
    _response.version(_request.version());
    _response.keep_alive(false);
    if (_request.method() == http::verb::get) {
        ParseGetParam();
        bool success =
            LogicSystem::GetInstance()->HandleGet(_get_url, shared_from_this());
        if (!success) {
            HandleHeaders(false);
        } else {
            HandleHeaders(true);
        }
        WriteResponse();
    } else if (_request.method() == http::verb::post) {
        bool success = LogicSystem::GetInstance()->HandlePost(
            _request.target(), shared_from_this());
        if (!success) {
            HandleHeaders(false);
        } else {
            HandleHeaders(true);
        }
        WriteResponse();
    }
}

void Session::HandleHeaders(bool success, const std::string &type,
                            const std::string &body) {
    _response.set(http::field::content_type, type + ";charset=utf-8");
    if (!success) {
        _response.result(http::status::not_found);
        beast::ostream(_response.body()) << "Not found";
    } else {
        _response.result(http::status::ok);
    }
}

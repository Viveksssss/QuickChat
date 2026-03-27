#ifndef LOGICSYSTEM_H
#define LOGICSYSTEM_H

#include "../global/Singleton.h"
#include "../global/const.h"
#include <unordered_map>

class Session;
using SessionHandler = std::function<void(std::shared_ptr<Session>)>;

class LogicSystem
    : public Singleton<LogicSystem>,
      public std::enable_shared_from_this<LogicSystem> {
    friend class Singleton<LogicSystem>;

public:
    ~LogicSystem() = default;
    bool HandleGet(const std::string&, std::shared_ptr<Session>);
    bool HandlePost(const std::string&, std::shared_ptr<Session>);

private:
    LogicSystem();
    void RegistHandlers(const std::string&, RequestType type, SessionHandler);
    void initHandlers();

private:
    std::unordered_map<std::string, SessionHandler> _post_handlers;
    std::unordered_map<std::string, SessionHandler> _get_handlers;
};

#endif // LOGICSYSTEM_H

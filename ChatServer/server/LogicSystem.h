#include "../global/Singleton.h"
#include "../global/const.h"
#include "../session/Session.h"
#include "./Server.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <nlohmann/json.hpp>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>


typedef std::function<void(std::shared_ptr<Session>, uint16_t msg_id, const std::string& msg)> FuncBack;

struct UserInfo;
class Server;
class LogicSystem : public Singleton<LogicSystem> {
    friend class Singleton<LogicSystem>;

public:
    // 设置Server
    void SetServer(std::shared_ptr<Server> server) noexcept;
    // 完成任务提交
    void PostMsgToQueue(std::shared_ptr<LogicNode> msg);
    // 注册回调
    void RegisterCallBacks();
    // 线程处理逻辑
    void DealMsg();
    //—————————————————————————————————————辅助函数—————————————————————————————————————————
    // 获取基本信息
    bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
    // 搜索用户
    bool IsPureDigit(const std::string& str);
    void GetSearchedUsers(const std::string& uid, json& j, bool only_digit);

public:
    LogicSystem(std::size_t size = std::thread::hardware_concurrency());
    ~LogicSystem();

private:
    std::queue<std::shared_ptr<LogicNode>> _queue;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::vector<std::thread> _work_threads;
    std::size_t _size;
    bool _stop;
    std::unordered_map<MsgId, FuncBack> _function_callbacks;
    std::shared_ptr<Server>_server;
};

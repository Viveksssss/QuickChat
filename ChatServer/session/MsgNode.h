#ifndef MSGNODE_H
#define MSGNODE_H

#include <boost/asio.hpp>

class LogicSystem;
class MsgNode {
    friend class Session;
    friend class LogicSystem;

public:
    MsgNode(uint16_t max_len);
    virtual ~MsgNode();
    void Clear();

protected:
    uint16_t _cur_len;
    uint16_t _total_len;
    char* _data;
};

class RecvNode : public MsgNode {
    friend class Session;
    friend class LogicSystem;

public:
    RecvNode(uint16_t max_len, uint16_t msg_id);

private:
    uint16_t _msg_id;
};

class SendNode : public MsgNode {
    friend class Session;
    friend class LogicSystem;

public:
    SendNode(const char* msg, uint16_t max_len, uint16_t msg_id);

private:
    uint16_t _msg_id;
};

#endif

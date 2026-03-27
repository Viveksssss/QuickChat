#include "MsgNode.h"
#include "../global/const.h"
#include <spdlog/spdlog.h>

MsgNode::MsgNode(uint16_t max_len)
    : _total_len(max_len)
    , _cur_len(0)
    , _data(new char[_total_len + 1])
{
    _data[_total_len] = '\0';
}

MsgNode::~MsgNode()
{
    delete[] _data;
}

void MsgNode::Clear()
{
    ::memset(_data, 0, _total_len);
    _cur_len = 0;
}

RecvNode::RecvNode(uint16_t max_len, uint16_t msg_id)
    : MsgNode(max_len)
    , _msg_id(msg_id)
{
}

SendNode::SendNode(const char* msg, uint16_t max_len, uint16_t msg_id)
    : MsgNode(max_len + HEAD_TOTAL_LEN)
    , _msg_id(msg_id)
{
    uint16_t msg_id_host = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
    memcpy(_data, &msg_id_host, HEAD_ID_LEN);

    uint16_t max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
    memcpy(_data + HEAD_ID_LEN, &max_len_host, HEAD_DATA_LEN);
    memcpy(_data + HEAD_TOTAL_LEN, msg, max_len);
}

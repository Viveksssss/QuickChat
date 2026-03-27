#include "ChatGrpcClient.h"
#include "../global/ConfigManager.h"
#include "message.pb.h"
#include <new>
#include <string>


//TODO:
GetChatServerResponse ChatGrpcClient::NotifyAddFriend(std::string server_ip,const AddFriendRequest&){
    GetChatServerResponse rsp;
    return rsp;
}

ChatGrpcClient::ChatGrpcClient(){
    auto&cfg = ConfigManager::GetInstance();
    auto server_list = cfg["ChatServers"]["name"];

    std::vector<std::string>words;
    words.reserve(10);

    std::stringstream ss(server_list);
    std::string word;

    while (std::getline(ss,word,',')){
        words.push_back(word);
    }

    for(const auto&word:words){
        if(cfg["word"]["name"].empty()){
            continue;
        }
        _pool[cfg[word]["name"]] = std::make_unique<RPCPool<ChatServer, ChatServer::Stub>>(10,cfg[word]["host"],cfg[word]["port"]);
    }
}

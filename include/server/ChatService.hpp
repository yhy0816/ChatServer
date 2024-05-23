#pragma once
#include <unordered_map>
#include <functional>
#include <muduo/net/TcpServer.h>
#include "json.hpp"
using namespace muduo;
using namespace muduo::net;
using namespace std;
using json = nlohmann::json;

//处理消息的事件回调函数类型
using MsgHandler = function<void(const TcpConnectionPtr& ,json, Timestamp)>;


//聊天服务器业务类
class ChatService{
    // 消息的类型
    enum EnMsgType {
        LOGIN_MSG = 1,
        REG_MSG
    };

public:
    // 获取单例模式的对象地址
    static ChatService* instance();
    // 处理登录业务
    void login(const TcpConnectionPtr &conn ,json js, Timestamp time);
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn ,json js, Timestamp time);
    // 获取消息对应的处理函数
    MsgHandler getHandler(int msgId);
private:
    ChatService();
    unordered_map<int, MsgHandler> _MsgHandlerMap;

};
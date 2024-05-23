#include "ChatService.hpp"
#include <muduo/base/Logging.h>

ChatService* ChatService::instance() {
    // 线程安全的单例模式
    static ChatService chatService;
    return &chatService;
}

ChatService::ChatService() {
    _MsgHandlerMap.insert({LOGIN_MSG, bind(&ChatService::login, this, _1, _2, _3)});
    _MsgHandlerMap.insert({REG_MSG, bind(&ChatService::reg, this, _1, _2, _3)});
}

// 获取消息对应的处理函数
MsgHandler ChatService::getHandler(int msgId) {
 
    //如果没有对应的处理器， 就打印日志
    if(_MsgHandlerMap.find(msgId) == _MsgHandlerMap.end()) {
 
        return [=](const TcpConnectionPtr& ,json, Timestamp) {
            LOG_ERROR << "msgId:" << msgId << " can't find handler!";
        };
    }
 
    return _MsgHandlerMap[msgId];
}


// 处理登录业务
void ChatService::login(const TcpConnectionPtr &conn ,json js, Timestamp time) {
    LOG_INFO << "do login service!!";
}
// 处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn ,json js, Timestamp time) {
    LOG_INFO << "do reg service!!";
}
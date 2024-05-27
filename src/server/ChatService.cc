#include "ChatService.hpp"
#include "User.hpp"
#include <muduo/base/Logging.h>

ChatService* ChatService::instance()
{
    // 线程安全的单例模式
    static ChatService chatService;
    return &chatService;
}

ChatService::ChatService()
{
    _MsgHandlerMap.insert({ EnMsgType::LOGIN_MSG, bind(&ChatService::login, this, _1, _2, _3) });
    _MsgHandlerMap.insert({ EnMsgType::REG_MSG, bind(&ChatService::reg, this, _1, _2, _3) });
}

// 获取消息对应的处理函数
MsgHandler ChatService::getHandler(EnMsgType msgId)
{

    // 如果没有对应的处理器， 就打印日志
    if (_MsgHandlerMap.find(msgId) == _MsgHandlerMap.end()) {

        return [=](const TcpConnectionPtr&, json, Timestamp) {
            LOG_ERROR << "msgId:" << (int)msgId << " can't find handler!";
        };
    }

    return _MsgHandlerMap[msgId];
}

// 处理登录业务
void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    // LOG_INFO << "do login service!!";
    int id = js["id"].get<int>();
    string pwd = js["password"]; // 获取 js中的id 和 密码

    User user = _userModel.query(id); // 从数据库中查询这个id的用户

    json respone; // 响应的json
    respone["msgid"] = EnMsgType::LOGIN_MSG_ACK;

    if (user.getId() == id && user.getPassword() == pwd) { // 登录成功
        if (user.getState() == "online") {
            // 用户已经登录了， 不能重复登录
            respone["errno"] = 2;
            respone["errmsg"] = "该账号已经登录, 请重新输入账号";
        } else {
            // 要更新登录状态为 online
            user.setState("online");
            _userModel.updateState(user);

            respone["errno"] = 0;
            respone["id"] = user.getId();
            respone["name"] = user.getName();
        }

    } else {
        // 用户不存在或密码错误
        respone["errno"] = 1;
        respone["errmsg"] = "账号或密码错误";
    }

    conn->send(respone.dump()); // 发送响应消息回去
}
// 处理注册业务
void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    // LOG_INFO << "do reg service!!";
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPassword(pwd);
    bool state = _userModel.insert(user); // 插入这个user到数据库中
    json respone;
    respone["msgid"] = EnMsgType::REG_MSG_ACK;
    if (state) { // 插入成功
        respone["errno"] = 0;
        respone["id"] = user.getId();
    } else {
        respone["errno"] = 1;
        respone["errno"] = "注册失败, 请重试";
    }
    conn->send(respone.dump()); // 发送响应消息回去
}

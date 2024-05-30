#include "ChatService.hpp"
#include "User.hpp"
#include <muduo/base/Logging.h>
#include <mutex>
#include <vector>

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
    _MsgHandlerMap.insert({ EnMsgType::ONE_CHAT_MSG, bind(&ChatService::oneChat, this, _1, _2, _3) });

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
            {
                lock_guard<mutex> _guard(_connMutex); // 守卫锁, 保证哈希表线程安全
                // 保存这个id 用户的连接
                _userConnMap.insert({ user.getId(), conn });
            }
            // 要更新登录状态为 online
            user.setState("online");
            _userModel.updateState(user);
            respone["errno"] = 0;
            respone["id"] = user.getId();
            respone["name"] = user.getName();
            vector<string> offlineMsgs = _offlineMsgModel.query(user.getId());
            if(!offlineMsgs.empty()) { // 如果离线消息不为空， 就发送出去
                // 如果这个用户有离线消息， 就把这个用户的离线消息发送给这个用户， 并从数据库删除掉
                _offlineMsgModel.remove(user.getId()); 
                respone["offlinemsg"] = offlineMsgs;
            }
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

// 处理用户异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    User user;
    {
        lock_guard<mutex> _guard(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it) {
            if (conn == it->second) {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    if (user.getId() != -1) { // 如果查找到了这个用户连接就更新为离线
        user.setState("offline");
        _userModel.updateState(user);
    }
}

void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int toid = js["toid"].get<int>();

    {
        lock_guard<mutex> guard(_connMutex);
        auto toConn = _userConnMap.find(toid);
        if (toConn != _userConnMap.end()) {
            // 用户在线， 转发消息
            toConn->second->send(js.dump());
            return;
        }
    }
    // 用户不在线， 存储离线消息
    // TODO  后边需要修改， 因为用户可能连接的不是这台主机 而是集群的其他主机
    _offlineMsgModel.insert(toid, js.dump());

}
/*
{"msgid" : 3, "name" : "yu", "password" : "123456"}
{"msgid" : 1, "id" : 2, "password" : "123456"}
{"msgid" : 5, "fromid" : 1, "toid" : 2, "msg" : "hello3"}

*/ 

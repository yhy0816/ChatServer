#pragma once
#include <muduo/net/Callbacks.h>
#include <unordered_map>
#include <functional>
#include <muduo/net/TcpServer.h>
#include <mutex>
#include "OfflineMsgModel.hpp"
#include "json.hpp"
#include "UserModel.hpp"
#include "MsgType.hpp"

using namespace muduo;
using namespace muduo::net;
using namespace std;
using json = nlohmann::json;

//处理消息的事件回调函数类型
using MsgHandler = function<void(const TcpConnectionPtr& ,json&, Timestamp)>;


//聊天服务器业务类
class ChatService{
   
public:
    // 获取单例模式的对象地址
    static ChatService* instance();
    //处理用户异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
    // 处理登录业务
    void login(const TcpConnectionPtr &conn ,json& js, Timestamp time);
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn ,json& js, Timestamp time);
    // 处理私聊消息
    void oneChat(const TcpConnectionPtr &conn ,json& js, Timestamp time);
    // 获取消息对应的处理函数
    MsgHandler getHandler(EnMsgType msgId);
private:
    ChatService();
    //消息类型对应处理器
    unordered_map<EnMsgType, MsgHandler> _MsgHandlerMap;
    // 用户 id 对应连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    // 用来保证 _userConnMap 线程安全
    mutex _connMutex;
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;

};  
#pragma once
#include <muduo/net/Callbacks.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer {
public:
    ChatServer(EventLoop* loop,
        const InetAddress& listenAddr,
        const string& nameArg);

    void start(); // 启动服务

private:
    // 上报连接信息的回调函数
    void onConnection(const TcpConnectionPtr&);
    // 上报读写事件信息的回调函数
    void onMessage(const TcpConnectionPtr&,
        Buffer*,
        Timestamp);

    TcpServer _server; // 实现服务器功能的muduo库类对象
    EventLoop* _loop; // 指向时间循环对象的指针
};
#include "ChatServer.hpp"
#include "json.hpp"
#include "ChatService.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop,
    const InetAddress& listenAddr,
    const string& nameArg)
    : _server(loop, listenAddr, nameArg)
    , _loop(loop)
{
    // 注册连接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数
    _server.setThreadNum(4);
}

// 启动服务
void ChatServer::start()
{
    _server.start();
}

// 上报连接信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    // 客户端断开连接
    if(!conn->connected()) {
        conn->shutdown();
    }
}
// 上报读写事件信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr& conn,
    Buffer* buf,
    Timestamp time)
{
    string buff = buf->retrieveAllAsString();
    json js = json::parse(buf);
    // 通过 json[msgid]获取消息类型
    // 再通过getHandler 获取对应的业务处理函数， 再进行调用
    auto MsgHandler = ChatService::instance()->getHandler(1);
    // MsgHandler(conn, js, time);
}
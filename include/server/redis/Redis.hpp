#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include <hiredis/hiredis.h>

using std::string;
using std::function;

class Redis {

public:
    Redis();
    ~Redis();
    // 连接 redis 服务器
    bool connect();
    // 向指定频道发送消息
    bool publish(int channel, const string& msg);
    // 订阅消息
    bool subscribe(int channel);
    // 解除订阅消息
    bool unsubscribe(int channel);

    // 在独立的线程中接收订阅的频道中的消息
    void osbserver_channel_msg();
    // 设置向业务层上报消息的回调函数对象
    void init_notify_handler(function<void(int, const string&)> handler);
private:
    //hiredis 同步上下文对象, 相当于打开了一个 redis-cli
    //负责publish消息
    redisContext* _publishContext = nullptr;
    // 负责subscribe消息, 这里要使用两个， 因为 subscribe 命令会阻塞
    redisContext* _subscribeContext = nullptr;

    // 当订阅频道有消息时调用这个函数向业务层报告
    function<void(int, const string&)> _notifyMsgHandler;
    static const string _serverIp;
    static const uint16_t _serverport;
};
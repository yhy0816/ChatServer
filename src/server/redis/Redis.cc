#include "Redis.hpp"
#include <hiredis/hiredis.h>
#include <iostream>
#include <thread>

using namespace std;

const string Redis::_serverIp = "127.0.0.1";
const uint16_t Redis::_serverport = 6379;

Redis::Redis()
{
}

Redis::~Redis()
{
    if (_subscribeContext) {
        redisFree(_subscribeContext);
    }
    if (_publishContext) {
        redisFree(_publishContext);
    }
}

// 连接 redis 服务器
bool Redis::connect()
{
    _publishContext = redisConnect(_serverIp.c_str(), _serverport);
    _subscribeContext = redisConnect(_serverIp.c_str(), _serverport);
    if (_publishContext == nullptr || _subscribeContext == nullptr) {
        cerr << "connect redis failed!" << endl;
        return false;
    }
    thread t(&Redis::osbserver_channel_msg, this); // 启动接收订阅消息的线程
    t.detach();
    cout << "connect redis success!" << endl;
    return true;
}
// 向指定频道发送消息
bool Redis::publish(int channel, const string& msg)
{
    redisReply* reply = static_cast<redisReply*>(redisCommand(_publishContext, "PUBLISH %d %s",
        channel, msg.c_str()));

    if (reply == nullptr) {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 订阅消息
bool Redis::subscribe(int channel)
{
    // 先将命令缓存到本地， 再发送到 redis 上, 直接使用redisCommand会阻塞
    if (REDIS_ERR == redisAppendCommand(_subscribeContext, "SUBSCRIBE  %d", channel)) {
        cerr << "subscribe command failed!" << endl;
        return false;
    }

    // 循环发送
    int done = 0;
    while (done == 0) {
        if (REDIS_ERR == redisBufferWrite(_subscribeContext, &done)) {
            cerr << "subscribe command redisBuffer failed!" << endl;
            return false;
        }
    }
    return true;
}
// 解除订阅消息
bool Redis::unsubscribe(int channel)
{

    // 先将命令缓存到本地， 再发送到 redis 上, 直接使用redisCommand会阻塞
    if (REDIS_ERR == redisAppendCommand(_subscribeContext, "unsubscribe %d", channel)) {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }

    // 循环发送

    int done = 0;
    while (done == 0) {
        if (REDIS_ERR == redisBufferWrite(_subscribeContext, &done)) {
            cerr << "unsubscribe command redisBuffer failed!" << endl;
            return false;
        }
    }
    return true;
}

// 在独立的线程中接收订阅的频道中的消息
void Redis::osbserver_channel_msg()
{


    // 循环阻塞等待消息

    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subscribeContext, (void **)&reply))
    {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 给业务层上报通道上发生的消息
            _notifyMsgHandler(atoi(reply->element[1]->str) , reply->element[2]->str);
        }

        freeReplyObject(reply);
    }
    cerr << "osbserver_channel_msg quit!" << endl;
}
// 设置向业务层上报消息的回调函数对象
void Redis::init_notify_handler(function<void(int, const string&)> handler)
{
    _notifyMsgHandler = handler;
}

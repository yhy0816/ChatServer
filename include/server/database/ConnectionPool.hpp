#pragma once
#include "MysqlConn.hpp"
#include <memory>
#include <queue>
#include <condition_variable>

class ConnectionPool{
public:
    static ConnectionPool& getInstance();
    // 外部调用的接口, 从连接池中取出一个连接
    shared_ptr<MySqlConn> getConnection(); 

private:
    ConnectionPool();
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    void produceConn(); // 生产连接线程函数
    void recycleConn(); // 回收连接线程函数

    queue<MySqlConn*> _connQueue;
    condition_variable _cond; // 控制生产和消费线程的条件变量
    mutex _queMutex;
    int _connCount = 0; // 记录当前所有连接的数量，包括分配出去的

    int _init_size = 5;
    int _min_size = 5;
    int _max_size = 128;
    seconds _maxIdleTime = seconds( 10); // 连接的最大空闲时间 60 秒
    seconds _waitTimeout = seconds(1); // 申请连接的最大超时时间 1 秒
};

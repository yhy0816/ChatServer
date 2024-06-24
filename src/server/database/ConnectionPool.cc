#include "ConnectionPool.hpp"
#include <thread>
#include <muduo/base/Logging.h>
using namespace std;

ConnectionPool& ConnectionPool::getInstance() {
    static ConnectionPool connectionPool;
    return connectionPool;
}

ConnectionPool::ConnectionPool() {
    

    // 创建_init_size个 连接
    for(int i = 0; i < _init_size; i++) {
        MySqlConn *conn =  new MySqlConn();
        _connQueue.push(conn);
    }
    _connCount = _init_size;

    // 启动生产和消费线程
    thread(&ConnectionPool::produceConn, this).detach();
    thread(&ConnectionPool::recycleConn, this).detach();


}

void ConnectionPool::produceConn() {
    
    while(1) {

        unique_lock<mutex> lock(_queMutex);
        // 队列不空或者总链接计数大都不需要生产
        while(!_connQueue.empty() || _connCount >= _max_size) { 
            _cond.wait(lock); // 释放锁并等待
        }
        MySqlConn* conn = new MySqlConn();
        _connQueue.push(conn);
        _connCount++;
        _cond.notify_all();

    }
}

void ConnectionPool::recycleConn() {
    
    while(1) {
        std::this_thread::sleep_for(_maxIdleTime); // 每隔_maxAliveTime扫描一次
        unique_lock<mutex> lock(_queMutex);
        while(_connQueue.size() > _min_size) {
            MySqlConn * conn = _connQueue.front();
            if(conn->getIdleTime() > _maxIdleTime) {
                delete conn;
                _connQueue.pop();
                _connCount--;
            } else {
                break;
            }
        }
        LOG_INFO << "释放超时连接! 池内剩余" << _connQueue.size();
    }
}

shared_ptr<MySqlConn> ConnectionPool::getConnection() {
    unique_lock<mutex> lock(_queMutex);

    while(_connQueue.empty()) {

        if(std::cv_status::timeout == _cond.wait_for(lock, _waitTimeout)) {
            
            // 如果超时了 还没有拿到连接返回空
            if(_connQueue.empty()) { // 因为有可能是被消费者唤醒了所以要判断一下
                LOG_INFO << "获取连接超时" ;
                return nullptr;
            }   
        }
        
    }

    MySqlConn* conn = _connQueue.front();
    _connQueue.pop();
    shared_ptr<MySqlConn> sp(
        conn,[this](MySqlConn* connect) {
            unique_lock<mutex> lock(_queMutex);
            connect->refreshUseTime();
            _connQueue.push(connect);
        }
    );

    // 队列消费空了就通知消费者
    if(_connQueue.empty()) {
        _cond.notify_all();
    }
    // LOG_INFO << "池内剩余连接数: " << _connQueue.size()
    //             << " 连接池总申请连接数: " << _connCount;

    return sp;
}


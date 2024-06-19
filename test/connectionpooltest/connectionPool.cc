#include <mysql/mysql.h>
#include <string>
#include <condition_variable>
#include <memory>
#include <queue>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <unistd.h>

using std::string;
using std::condition_variable;
using std::shared_ptr;
using std::queue;
using std::cout;
using std::thread;
using std::mutex;
using std::unique_lock;
using std::endl;
using namespace std::chrono;

class MySQL{

public:
    MySQL();
    ~MySQL();
    bool connect();
    bool update(const string& sql);
    MYSQL_RES* query(const string& sql);
    MYSQL* getConnection() const {return _conn;};

private:
    MYSQL* _conn;
    static string server;
    static string user;
    static string password;
    static string dbname;
};


string MySQL::server = "127.0.0.1";
string MySQL::user = "root";
string MySQL::password = "123456";
string MySQL::dbname = "chat";


MySQL::MySQL(){
    _conn = mysql_init(nullptr); 
}

MySQL::~MySQL(){

    if (_conn) {
        mysql_close(_conn);
    }
}

bool MySQL::connect(){
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
        password.c_str(), dbname.c_str(), 3306,
        nullptr, 0);
    if(p) {
        mysql_query(_conn, "set names utf8"); // 防止中文乱码
        // LOG_INFO << "connect mysql success!";
    } else {
        cout << "connect mysql error!";
    }
    return p;
}

bool MySQL::update(const string& sql){
    //如果查询成功，返回0。如果出现错误，返回非0值。
    if(mysql_query(_conn, sql.c_str())) {
        cout << sql << " 更新失败!!";
        return false;
    }
    
    return true;
}

MYSQL_RES* MySQL::query(const string& sql){
    //如果查询成功，返回0。如果出现错误，返回非0值。
    if(mysql_query(_conn, sql.c_str())) {
        cout << sql << " 查询失败";
        return nullptr;
    }
    
    return mysql_use_result(_conn);
}

class MySqlConn : public MySQL {
public:
    MySqlConn() {
        connect();
        refreshUseTime();
    }
    // 更新最后一次使用的时间
    void refreshUseTime() {
        _lastUseTime = steady_clock::now();
    }

    seconds getIdleTime() const {
        return duration_cast<seconds>(steady_clock::now() - _lastUseTime);
    }
private:
    steady_clock::time_point _lastUseTime; // 最后使用这个连接的时间
};


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

    int _init_size = 10;
    int _min_size = 5;
    int _max_size = 128;
    seconds _maxIdleTime = seconds( 10); // 连接的最大空闲时间 60 秒
    seconds _waitTimeout = seconds(1); // 申请连接的最大超时时间 1 秒
};



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

    }
}

shared_ptr<MySqlConn> ConnectionPool::getConnection() {
    unique_lock<mutex> lock(_queMutex);

    while(_connQueue.empty()) {

        if(std::cv_status::timeout == _cond.wait_for(lock, _waitTimeout)) {
            
            // 如果超时了 还没有拿到连接返回空
            if(_connQueue.empty()) { // 因为有可能是被消费者唤醒了所以要判断一下
                cout << "获取连接超时" << endl;
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
            cout <<"size: " << _connQueue.size() << endl;
        }
    );

    // 队列消费空了就通知消费者
    if(_connQueue.empty()) {
        _cond.notify_all();
    }

    return sp;
}

int main() {
    shared_ptr<MySqlConn> conns[1024];
    for(int i = 0; i < 64; i++) {
        conns[i] = ConnectionPool::getInstance().getConnection();
        cout << i << " " << conns[i].get() << endl;
    }
    
    for(int i = 0; i < 32; i++) {
        
        conns[i].reset();
    }

    // std::this_thread::sleep_for(seconds(1));

    for(int i = 32; i < 64; i++) {
        
        conns[i].reset();
    }
    sleep(1);
    auto it1 = ConnectionPool::getInstance().getConnection();
    auto it2 = ConnectionPool::getInstance().getConnection();
    auto it3 = ConnectionPool::getInstance().getConnection();
    // cout << it.get() << endl;
    return 0;
}

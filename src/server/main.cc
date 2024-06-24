#include <csignal>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <csignal>
#include <iostream>
#include <unistd.h>
#include "ChatServer.hpp"
#include "ChatService.hpp"
#include "ConnectionPool.hpp"

using namespace std;

void reset(int sig) {
    // 数据库中恢复离线状态
    ChatService::instance()->exceptExit();
    // 关闭连接池的生产消费线程
    ConnectionPool::getInstance().close();
    exit(0);
}

int main(int argc, char* argv[]) {
    if(argc != 3) {
        cout << "参数形式 ChatServer ip port" << endl;
        return 1;
    }
    string ip = argv[1];
    int port = atoi(argv[2]);

//  处理 crtl + c 的信号
    signal(SIGINT, reset);
    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "server");
    server.start();
    loop.loop();
}

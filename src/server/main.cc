// #include <iostream>
#include <csignal>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <signal.h>
#include "ChatServer.hpp"
#include "ChatService.hpp"

using namespace std;

void reset(int sig) {
    
    ChatService::instance()->exceptExit();
    exit(0);
}

int main() {
// TODO 处理 crtl + c 的信号
    signal(SIGINT, reset);
    EventLoop loop;
    InetAddress addr("127.0.0.1", 9000);
    ChatServer server(&loop, addr, "server");
    server.start();
    loop.loop();
}
//{"msgid" : 1, "id": 1, "password": "123456"}
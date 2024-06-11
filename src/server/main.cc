// #include <iostream>
#include <csignal>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <csignal>
#include <iostream>
#include "ChatServer.hpp"
#include "ChatService.hpp"

using namespace std;

void reset(int sig) {
    
    ChatService::instance()->exceptExit();
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
//{"msgid" : 1, "id": 1, "password": "123456"}
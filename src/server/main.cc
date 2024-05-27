// #include <iostream>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include "ChatServer.hpp"

using namespace std;

int main() {
    EventLoop loop;
    InetAddress addr("127.0.0.1", 9000);
    ChatServer server(&loop, addr, "server");
    server.start();
    loop.loop();
}
//{"msgid" : 1, "id": 1, "password": "12356"}
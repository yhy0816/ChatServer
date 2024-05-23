// #include <iostream>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include "ChatServer.hpp"

using namespace std;

int main() {
    EventLoop loop;
    InetAddress addr("107.0.0.1", 10001);
    ChatServer server(&loop, addr, "server");
    server.start();
    loop.loop();
}
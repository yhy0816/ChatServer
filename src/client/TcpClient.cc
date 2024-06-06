#include "ChatService.hpp"
#include "Group.hpp"
#include "MsgType.hpp"
#include "User.hpp"
#include "json.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace nlohmann;
using namespace std;
const string SERVER_IP = "127.0.0.1";
const short SERVER_PORT = 9000;
class ChatClient;

// 对 socket 进行封装
class TcpClient {
public:
    ~TcpClient()
    {
    }
    bool CloseConn()
    {
        return close(sock) == 0;
    }
    bool Connect(const string& ip, short port)
    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);
        addr.sin_port = htons(port);
        return connect(sock, (sockaddr*)&addr, sizeof addr) == 0;
    }
    int Recv(char* buf, size_t bufSize)
    {
        return recv(sock, buf, bufSize, 0);
    }
    int Send(const string& msg)
    {
        return send(sock, msg.c_str(), msg.length(), 0);
    }

    TcpClient()
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            throw strerror(errno);
        }
    }

private:
    int sock;
};


string getCurrentTime(){
    // TDDO 获取时间
    return "111";
}

class ChatClient {

public:
    ChatClient()
    {
    }
    ~ChatClient()
    {
        if(readMsgThread) {
            delete readMsgThread;
        }
    }
    void readMsgHandler()
    {
        char buf[1024];
        while (isloggedIn) {
            memset(buf, 0, sizeof buf);
            int recvLen = tcpClient.Recv(buf, sizeof buf);
            if (recvLen <= 0) {
                tcpClient.CloseConn();
                exit(-1);
            }
            json msgJs = json::parse(buf);
            if (msgJs["msgid"] == EnMsgType::ONE_CHAT_MSG) {
                printf("id为 %d 用户: %s : %s\n", msgJs["id"].get<int>(), string(msgJs["name"]).c_str(), string(msgJs["msg"]).c_str());
                continue;
            }
        }
    }

    bool getisloggedIn() {
        return isloggedIn;
    }


    void logout() {
        isloggedIn = false;
        json logoutJs;
        logoutJs["msgid"] = EnMsgType::LOGOUT_MSG;
        logoutJs["id"] = cur_user.getId();
        tcpClient.Send(logoutJs.dump());
    }

    bool login(int id, const string& pwd)
    {
        json loginJs;
        loginJs["msgid"] = EnMsgType::LOGIN_MSG;
        loginJs["id"] = id;
        loginJs["password"] = pwd;

        tcpClient.Send(loginJs.dump());

        char respone[4096] = {};

        tcpClient.Recv(respone, sizeof respone);

        json responeJs = json::parse(respone);
        cout << respone << endl;
        if (responeJs["msgid"] != EnMsgType::LOGIN_MSG_ACK) {

            return false;
        }
        if (responeJs["errno"] != 0) {
            cout << responeJs["errmsg"] << endl;
            return false;
        }
        isloggedIn = true;
        cur_user.setId(id);
        cur_user.setName(responeJs["name"]);
        // cur_user.setPassword(pwd);
        // cur_user.setState("online");

        offlinemsg.clear();
        offlinemsg = responeJs["offlinemsg"];
        friends.clear();
        vector<json> sfriends = responeJs["friends"];
        friends.reserve(sfriends.size());
        for (const auto& jfriend : sfriends) {

            User u(jfriend["id"].get<int>(), jfriend["name"], "", jfriend["state"]);
            this->friends.push_back(u);
        }
        groups.clear();

        // 登录成功后启动接收数据的线程
        if(readMsgThread == nullptr) {
            readMsgThread = new thread(&ChatClient::readMsgHandler, this);
            readMsgThread->detach();
        }
        
        
        return true;
    }
    void chat(int toid, string msg) {
        json msgJs;
        msgJs["msgid"] = EnMsgType::ONE_CHAT_MSG;
        msgJs["toid"] = toid;
        msgJs["time"] = getCurrentTime();
        msgJs["msg"] = msg;
        tcpClient.Send(msgJs.dump());
    }

    int regist(const string& name, const string& pwd)
    {
        json regJs;
        regJs["msgid"] = EnMsgType::REG_MSG;
        regJs["name"] = name;
        regJs["password"] = pwd;
        tcpClient.Send(regJs.dump());

        char respone[4096] = {};
        tcpClient.Recv(respone, sizeof respone);
        json responeJs = json::parse(respone);

        if (responeJs["msgid"].get<EnMsgType>() != EnMsgType::REG_MSG_ACK) {
            return -1;
        }

        // cout << responeJs.dump() << endl;
        if (responeJs["errno"].get<int>() != 0) {
            cout << responeJs["errmsg"] << endl;
            return -1;
        }
        return responeJs["id"].get<int>();
    }
    void showInfo()
    {
        cout << "-------------当前登录用户--------------" << endl;
        printf("用户 id: %d, 用户名: %s\n", cur_user.getId(), cur_user.getName().c_str());
        showFrined();
        showGroup();
        showOfflinemsg();
    }
    void showFrined()
    {
        cout << "---------------好友列表----------------" << endl;
        for (auto fri : friends) {
            cout << fri.getId() << " " << fri.getName() << " " << fri.getState() << endl;
        }
    }

    void showGroup()
    {
        cout << "---------------群组列表----------------" << endl;
        for (auto gro : groups) {
            cout << gro.getId() << " " << gro.getName() << " " << gro.getDesc() << endl;
        }
    }
    bool connServer()
    {
        return tcpClient.Connect(SERVER_IP, SERVER_PORT);
    }
    void showOfflinemsg() {
        cout << "---------------离线消息----------------" << endl;

        for(auto it : offlinemsg) {
            cout << it.dump() << endl;
        }
    }

private:
    bool isloggedIn = false;
    thread *readMsgThread = nullptr;
    TcpClient tcpClient;
    vector<json> offlinemsg;
    User cur_user;
    vector<User> friends;
    vector<Group> groups;
};

ChatClient chatClient;

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    { "help", "显示所有支持的命令，格式help" },
    { "chat", "一对一聊天，格式chat:friendid:message" },
    { "addfriend", "添加好友，格式addfriend:friendid" },
    { "creategroup", "创建群组，格式creategroup:groupname:groupdesc" },
    { "addgroup", "加入群组，格式addgroup:groupid" },
    { "groupchat", "群聊，格式groupchat:groupid:message" },
    { "loginout", "注销，格式loginout" }

};


void mainPage();
void login()
{

    cout << "---------登录----------" << endl;
    int id;
    string pwd;
    cout << "账号：";
    cin >> id;
    cin.get();
    cout << "密码：";
    cin >> pwd;
    cin.get();

    if (chatClient.login(id, pwd)) {
        chatClient.showInfo();
        mainPage();
    }
}

void regist()
{

    cout << "---------注册----------" << endl;
    cout << "name: ";
    string name;
    getline(cin, name);

    cout << "password: ";
    string password;
    getline(cin, password);

    int id = chatClient.regist(name, password);
    if (id != -1) {
        cout << "你的用户 id 为 " << id << "，请保存好" << endl;
    }
}

void drawMainMenu()
{
    cout << "----------------------" << endl;
    cout << "------1. 登录---------" << endl;
    cout << "------2. 注册---------" << endl;
    cout << "------3. 退出---------" << endl;
    cout << "----------------------" << endl;
}
void drawHelp(string)
{
    for(auto it : commandMap) {
        cout << it.first << ":" << it.second << endl;
    }
}

void chat(string cmd) {
    int idx = cmd.find(":");
    if(idx == -1) {
        cout << "命令有误!" << endl;
        return;
    }
    string sid = cmd.substr(0, idx);
    string message = cmd.substr( idx + 1);
    chatClient.chat(stoi(sid),  message);


}
// TODO  功能
void addfriend(string cmd) {
    
}
void addgroup(string cmd) {
    
}
void creategroup(string cmd) {
    
}
void groupchat(string cmd) {
    
}

void logout(string cmd) {
    chatClient.logout();
}
// 注册系统支持的客户端命令处理
unordered_map<string, function<void(string)>> commandHandlerMap = {
    {"help", drawHelp},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", logout}
       
};
void mainPage()
{
    drawHelp("");
    while(chatClient.getisloggedIn()) {
        string longcmd, realcmd; 
        cout << ">>> ";
        getline(cin, longcmd); 
        if(longcmd == "") continue;
        int idx = longcmd.find(":");
        if(idx == -1) {
            realcmd = longcmd;
        } else {
            realcmd = longcmd.substr(0, idx);
        }
    
        auto handler = commandHandlerMap.find(realcmd);
        if(handler == commandHandlerMap.end()) {
            cout << "无效的命令, 请重新输入" << endl;
        } else {
            handler->second(longcmd.substr(idx + 1));
        }

    }
}

int main()
{

    if (!chatClient.connServer()) {
        cout << "连接服务器失败！！" << endl;
        return 1;
    }

    while (1) {
        drawMainMenu();
        cout << "choise: ";
        int choice;
        cin >> choice;
        cin.get();
        switch (choice) {
        case 1: {
            login();
            break;
        }
        case 2: {
            regist();
            break;
        }
        case 3: {
            cout << "Bye~" << endl;
            exit(0);
        }

        default: {
            cout << "输入有误，请重新输入" << endl;
        }
        }
    }

    return 0;
}
#include "ChatService.hpp"
#include "Group.hpp"
#include "GroupUser.hpp"
#include "MsgType.hpp"
#include "User.hpp"
#include "json.hpp"
#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
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
const short SERVER_PORT = 8000;
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

string getCurrentTime()
{
    auto t = chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // 转为字符串
    stringstream ss;
    ss << std::put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    string str_time = ss.str();
    return str_time;
}

class ChatClient {

public:
    ChatClient()
    {
        using namespace placeholders;
        respHandlerMap.insert({ EnMsgType::ONE_CHAT_MSG, std::bind(&ChatClient::oneChatHandler, this, _1) });
        respHandlerMap.insert({ EnMsgType::GROUP_CHAT_MSG, std::bind(&ChatClient::groupChatHandler, this, _1) });
        respHandlerMap.insert({ EnMsgType::FRIEND_REQUEST_MSG, std::bind(&ChatClient::frendRequestHandler, this, _1) });
        respHandlerMap.insert({ EnMsgType::LOGIN_MSG_ACK, std::bind(&ChatClient::loginAckHandler, this, _1) });
        respHandlerMap.insert({ EnMsgType::REG_MSG_ACK, std::bind(&ChatClient::createUserHandler, this, _1) });
        respHandlerMap.insert({ EnMsgType::CREATE_GROUP_MSG_ACK, std::bind(&ChatClient::createGroupHandler, this, _1) });

        
    }

    void runReadMsgThread(){
        // 启动接收数据的线程
        if (readMsgThread == nullptr) {
            readMsgThread = new thread(&ChatClient::readMsgHandler, this);
            readMsgThread->detach();
        }
    }

    ~ChatClient()
    {
        if (readMsgThread) {
            delete readMsgThread;
        }
    }
    void frendRequestHandler(const json& msgJs)
    {
        printf("\n[%d] %s 向你发送了一个好友请求\n",
            msgJs["id"].get<int>(), string(msgJs["name"]).c_str());
    }

    void oneChatHandler(const json& msgJs)
    {
        printf("\n%s [%d] %s: %s\n", string(msgJs["time"]).c_str(),
            msgJs["id"].get<int>(),
            string(msgJs["name"]).c_str(),
            string(msgJs["msg"]).c_str());
    }

    void groupChatHandler(const json& msgJs)
    {
        printf("\n%s 群组[%d] 用户[%d] : %s\n",
            string(msgJs["time"]).c_str(),
            msgJs["gid"].get<int>(),
            msgJs["uid"].get<int>(),
            string(msgJs["msg"]).c_str());
    }

    void createGroupHandler(const json& msgJs)
    {
        if (msgJs["errno"].get<int>() != 0) {
            printf("\n建群失败: %s\n", string(msgJs["errmsg"]).c_str());
        } else {
            printf("\n你的群号为 %d\n", msgJs["gid"].get<int>());
        }
    }
    
    void createUserHandler(const json& msgJs)
    {
        if (msgJs["errno"].get<int>() != 0) {
            printf("\n注册失败: %s\n", string(msgJs["errmsg"]).c_str());
        } else {
            printf("\n你的 id 号为 请保存好 %d\n", msgJs["id"].get<int>());
        }
        cond.notify_all();
    }

    void loginAckHandler(const json& responeJs)
    {
        
        if (responeJs["errno"] != 0) {
            cout << responeJs["errmsg"] << endl;
            cond.notify_all();
            return;
        }
        cur_user.setId(responeJs["id"].get<int>());
        cur_user.setName(responeJs["name"]);
        // cur_user.setPassword(pwd);
        // cur_user.setState("online");
        offlinemsg.clear();
        vector<string> sofflinemsg = responeJs["offlinemsg"];
        offlinemsg.reserve(sofflinemsg.size());
        for (const auto& it : sofflinemsg) {
            offlinemsg.push_back(json::parse(it));
        }
        friends.clear();
        vector<json> sfriends = responeJs["friends"];
        friends.reserve(sfriends.size());
        for (const auto& jfriend : sfriends) {

            User u(jfriend["id"].get<int>(), jfriend["name"], "", jfriend["state"]);
            this->friends.push_back(u);
        }
        groups.clear();
        //  处理用户群组
        vector<json> groupsjs = responeJs["groups"];
        groups.reserve(groupsjs.size());
        for (const auto& gjs : groupsjs) { // 遍历群组的js
            Group group(gjs["gid"].get<int>(), gjs["gname"].get<string>(), gjs["gdesc"].get<string>());
            vector<json> gmembers = gjs["gmembers"];
            group.getMembers().reserve(gmembers.size());
            for (auto& gmember : gmembers) {
                GroupUser guser(gmember["guid"].get<int>(),
                    gmember["guname"].get<string>(),
                    "",
                    gmember["gustate"].get<string>(),
                    gmember["gurole"].get<string>());
                group.getMembers().push_back(guser);
            }
            groups.push_back(group);
        }
        lock_guard<mutex> guard(mutex);
        isloggedIn = true;
        cond.notify_all();

    }

    void msgHandler(json& msgJs)
    {

        // cout << msgJs.dump() << endl;

        auto it = respHandlerMap.find(msgJs.at("msgid").get<EnMsgType>());
        // 
        if (it == respHandlerMap.end()) {
            if (msgJs["errno"].get<int>() != 0) {
                printf("\n操作失败: %s\n", string(msgJs["errmsg"]).c_str());
            } else {
                printf("\n操作成功!!\n");
            }

        } else {
            it->second(msgJs);
        }
    }

    void readMsgHandler()
    {
        char buf[1024 * 8];

        while (1) {

            memset(buf, 0, sizeof buf);
            int recvLen;

            recvLen = tcpClient.Recv(buf, sizeof buf);

            if (recvLen <= 0) {
                tcpClient.CloseConn();
                exit(-1);
            }
            // cout << "收到" << buf << endl;
            json msgJs = json::parse(buf);
            msgHandler(msgJs);
        }
    }

    bool getisloggedIn()
    {
        lock_guard<mutex> guard(mtx);
        return isloggedIn;
    }

    bool login(int id, const string& pwd)
    {
        json loginJs;
        loginJs["msgid"] = EnMsgType::LOGIN_MSG;
        loginJs["id"] = id;
        loginJs["password"] = pwd;

        tcpClient.Send(loginJs.dump());


        
        unique_lock<mutex> lock(mtx);
        cond.wait(lock);
        return isloggedIn;
        
    }

    void logout()
    {

        {
            lock_guard<mutex> guard(mtx);
            if (isloggedIn == false)
                return;
        }
        
        json logoutJs;
        logoutJs["msgid"] = EnMsgType::LOGOUT_MSG;
        logoutJs["id"] = cur_user.getId();
        tcpClient.Send(logoutJs.dump());
        {
            lock_guard<mutex> guard(mtx);
            isloggedIn = false;
        }

        cur_user.setId(-1);
        offlinemsg.clear();
        friends.clear();
        groups.clear();
    }

    void friendRequest(int fid)
    {
        json js;
        js["msgid"] = EnMsgType::FRIEND_REQUEST_MSG;
        js["id"] = cur_user.getId();
        js["toid"] = fid;
        js["name"] = cur_user.getName();
        tcpClient.Send(js.dump());
    }

    void agreeFriendRequest(int fid)
    {
        json js;
        js["msgid"] = EnMsgType::FRIEND_AGREE_MSG;
        js["uid"] = fid;
        js["fid"] = cur_user.getId();
        tcpClient.Send(js.dump());
    }

    void createGroup(const string& name, const string& groupDesc)
    {
        json js;
        js["msgid"] = EnMsgType::CREATE_GROUP_MSG;
        js["id"] = cur_user.getId();
        js["name"] = name;
        js["desc"] = groupDesc;
        tcpClient.Send(js.dump());
    }

    void addGroup(int gid)
    {
        json js;
        js["msgid"] = EnMsgType::ADD_GROUP_MSG;
        js["uid"] = cur_user.getId();
        js["gid"] = gid;
        tcpClient.Send(js.dump());
    }

    void groupChat(int gid, const string& msg)
    {
        json js;
        js["msgid"] = EnMsgType::GROUP_CHAT_MSG;
        js["uid"] = cur_user.getId();
        js["gid"] = gid;
        js["time"] = getCurrentTime();
        js["msg"] = msg;
        // cout << js.dump() << endl;
        tcpClient.Send(js.dump());
    }

    void chat(int toid, string msg)
    {
        json msgJs;

        msgJs["msgid"] = EnMsgType::ONE_CHAT_MSG;
        msgJs["id"] = cur_user.getId();
        msgJs["toid"] = toid;
        msgJs["name"] = cur_user.getName();
        msgJs["time"] = getCurrentTime();
        msgJs["msg"] = msg;
        tcpClient.Send(msgJs.dump());
    }

    void regist(const string& name, const string& pwd)
    {
        json regJs;
        regJs["msgid"] = EnMsgType::REG_MSG;
        regJs["name"] = name;
        regJs["password"] = pwd;
        tcpClient.Send(regJs.dump());
        unique_lock<mutex> lock(mtx);
        cond.wait(lock);
        // char respone[4096] = {};
        // tcpClient.Recv(respone, sizeof respone);
        // json responeJs = json::parse(respone);

        // if (responeJs["msgid"].get<EnMsgType>() != EnMsgType::REG_MSG_ACK) {
        //     return -1;
        // }

        // // cout << responeJs.dump() << endl;
        // if (responeJs["errno"].get<int>() != 0) {
        //     cout << responeJs["errmsg"] << endl;
        //     return -1;
        // }
        
        // return responeJs["id"].get<int>();
    }
    void showInfo()
    {
        cout << "-------------当前登录用户--------------" << endl;
        printf("用户 id: %d, 用户名: %s\n", cur_user.getId(), cur_user.getName().c_str());
        if (!friends.empty())
            showFrined();
        if (!groups.empty())
            showGroup();
        if (!offlinemsg.empty())
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
        for (auto& gro : groups) {
            cout << gro.getId() << " " << gro.getName() << " " << gro.getDesc() << endl;
            for (const auto& guser : gro.getMembers()) {
                printf("    [%d] [%s] %s : %s\n", guser.getId(),
                    guser.getState().c_str(),
                    guser.getName().c_str(),
                    guser.getRole().c_str());
            }
        }
    }
    bool connServer()
    {
        return tcpClient.Connect(SERVER_IP, SERVER_PORT);
    }
    void showOfflinemsg()
    {
        cout << "---------------离线消息----------------" << endl;

        for (auto it : offlinemsg) {
            //
            msgHandler(it);
        }
    }

private:
    bool isloggedIn;
    thread* readMsgThread = nullptr;
    TcpClient tcpClient;
    vector<json> offlinemsg;
    User cur_user;
    vector<User> friends;
    vector<Group> groups;
    unordered_map<EnMsgType, function<void(const json&)>> respHandlerMap;
    condition_variable cond;
    mutex mtx;
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
    { "loginout", "注销，格式loginout" },
    { "agreefriend", "同意好友请求，格式agreefriend:friendid" },

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
    chatClient.regist(name, password);

}

void drawMainMenu()
{
    cout << "----------------------------------------" << endl;
    cout << "---------------1. 登录------------------" << endl;
    cout << "---------------2. 注册------------------" << endl;
    cout << "---------------3. 退出------------------" << endl;
    cout << "----------------------------------------" << endl;
}
void drawHelp(string)
{
    for (auto it : commandMap) {
        cout << it.first << ":" << it.second << endl;
    }
}

void chat(string cmd)
{
    int idx = cmd.find(":");
    if (idx == string::npos) {
        cout << "命令有误!" << endl;
        return;
    }
    string sid = cmd.substr(0, idx);
    string message = cmd.substr(idx + 1);
    try {
        chatClient.chat(stoi(sid), message);
    } catch (...) {
        cout << "输入有误" << endl;
        return;
    }
}

void addfriend(string cmd)
{
    int friendid;
    try {
        friendid = stoi(cmd);
    } catch (...) {
        cout << "输入有误" << endl;
        return;
    }
    chatClient.friendRequest(friendid);
}

void addgroup(string cmd)
{
    int groupid;
    try {
        groupid = stoi(cmd);
    } catch (...) {
        cout << "输入有误" << endl;
        return;
    }
    chatClient.addGroup(groupid);
}

void creategroup(string cmd)
{
    int idx = cmd.find(":");
    if (idx == string::npos) {
        cout << "命令有误, 请重新输入" << endl;
        return;
    }
    string gname = cmd.substr(0, idx);
    string gdesc = cmd.substr(idx + 1);
    chatClient.createGroup(gname, gdesc);
}

void groupchat(string cmd)
{
    int idx = cmd.find(":");
    if (idx == string::npos) {
        cout << "命令有误, 请重新输入" << endl;
        return;
    }
    int gid;
    try {
        gid = stoi(cmd.substr(0, idx));
    } catch (...) {
        cout << "输入有误" << endl;
        return;
    }

    string msg = cmd.substr(idx + 1);
    chatClient.groupChat(gid, msg);
}

void agreefriend(string cmd)
{
    int fid;
    try {
        fid = stoi(cmd);
    } catch (...) {
        cout << "输入有误" << endl;
        return;
    }
    chatClient.agreeFriendRequest(fid);
}

void logout(string cmd)
{
    chatClient.logout();
}

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(string)>> commandHandlerMap = {
    { "help", drawHelp },
    { "chat", chat },
    { "addfriend", addfriend },
    { "creategroup", creategroup },
    { "addgroup", addgroup },
    { "groupchat", groupchat },
    { "loginout", logout },
    { "agreefriend", agreefriend }
};

void mainPage()
{
    drawHelp("");
    while (chatClient.getisloggedIn()) {
        string longcmd, realcmd;
        cout << ">>> ";
        getline(cin, longcmd);
        if (longcmd == "")
            continue;
        int idx = longcmd.find(":");
        if (idx == string::npos) {
            realcmd = longcmd;
        } else {
            realcmd = longcmd.substr(0, idx);
        }
        auto handler = commandHandlerMap.find(realcmd);
        if (handler == commandHandlerMap.end()) {
            cout << "无效的命令, 请重新输入" << endl;
        } else {
            handler->second(longcmd.substr(idx + 1));
        }
    }
}

int main()
{

    cout << getCurrentTime();
    if (!chatClient.connServer()) {
        cout << "连接服务器失败！！" << endl;
        return 1;
    }
    chatClient.runReadMsgThread();
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
#pragma once
#include <string>
#include <vector>
using namespace std;
// 处理离线消息与数据库的交互
class OfflineMsgModel {
public:
    bool insert(int id, const string& msg);
    bool remove(int id);
    vector<string> query(int id);
private:

};
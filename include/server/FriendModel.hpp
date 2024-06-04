#pragma once
#include <vector>
#include "User.hpp"
using namespace std;

//好友关系的数据操作类
class FriendModel {

public:
    // 插入一条好友记录
    bool insert(int userid, int friendid);
    // 查询指定id的所有好友
    vector<User> query(int userid);

};
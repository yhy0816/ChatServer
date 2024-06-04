#pragma once
#include "Group.hpp"
#include <vector>

// 群组信息的操作接口类
class GroupModel {
public:
    // 创建群组
    bool createGroup(Group& group);
    // 加入群组
    bool addGroup(int userid, int groupid, string role);
    // 查询用户所在群组信息
    vector<Group> queryGroups(int id);
    // 根据groupid 查询群成员 除了 userid 的所有群成员id 
    vector<int> queryGroupUsers(int groupid, int userid);
};
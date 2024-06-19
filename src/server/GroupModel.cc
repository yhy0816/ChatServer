#include "GroupModel.hpp"
// #include "MySQL.hpp"
#include <mysql/mysql.h>
#include <ConnectionPool.hpp>
// 创建群组
bool GroupModel::createGroup(Group& group)
{
    char sql[1024];

    sprintf(sql, "insert into AllGroup(groupname, groupdesc) values('%s', '%s')",
        group.getName().c_str(), group.getDesc().c_str());
    // MySQL mysql;
    // if (mysql.connect()) {
    //     if (mysql.update(sql)) {
    //         group.setId(mysql_insert_id(mysql.getConnection()));
    //         return true;
    //     }
    // }

    shared_ptr<MySqlConn> conn = ConnectionPool::getInstance().getConnection();
    if (conn->update(sql)) {
        group.setId(mysql_insert_id(conn->getConnection()));
        return true;
    }
    return false;
}

// 加入群组
bool GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[1024];
    sprintf(sql, "insert into GroupUser(userid, groupid, grouprole) values(%d, %d, '%s')",
        userid, groupid, role.c_str());
    // MySQL mysql;
    // return mysql.connect() && mysql.update(sql);
    shared_ptr<MySqlConn> conn = ConnectionPool::getInstance().getConnection();

    return conn->update(sql);

}

// 查询用户所在群组信息
//
vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024];
    sprintf(sql, "select AllGroup.id, AllGroup.groupname, AllGroup.groupdesc from GroupUser "
                 "join AllGroup on GroupUser.groupid = AllGroup.id where GroupUser.userid = %d ",
        userid);
    // MySQL mysql;
    vector<Group> groups;
    // 首先查出这个用户所在的所有组
    // if (mysql.connect()) {
    //     MYSQL_RES* res = mysql.query(sql);
    //     if (res) {
    //         groups.reserve(mysql_num_rows(res)); // 提前预留好空间
    //         MYSQL_ROW row;
    //         while ((row = mysql_fetch_row(res)) != nullptr) { // 每一行一次循环, 每一次循环都是一个group
    //             Group group;
    //             group.setId(atoi(row[0]));
    //             group.setName(row[1]);
    //             group.setDesc(row[2]);
    //             groups.push_back(group);
    //         }
    //         mysql_free_result(res);
    //     }
    // }

    shared_ptr<MySqlConn> conn = ConnectionPool::getInstance().getConnection();
    MYSQL_RES* res = conn->query(sql);
    if (res) {
        groups.reserve(mysql_num_rows(res)); // 提前预留好空间
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr) { // 每一行一次循环, 每一次循环都是一个group
            Group group;
            group.setId(atoi(row[0]));
            group.setName(row[1]);
            group.setDesc(row[2]);
            groups.push_back(group);
        }
        mysql_free_result(res);
    }

    // 此时groups内的group的 members 还没有查询到
    // 然后查这个用户所属的组内都有哪些用户
    for (Group& g : groups) {
        char sql[1024];
        sprintf(sql, "select User.id, User.name, User.state, GroupUser.grouprole from User "
                     "join GroupUser on User.id = GroupUser.userid where GroupUser.groupid = %d",
            g.getId());

        MYSQL_RES* res = conn->query(sql);
        if (res) {
            g.getMembers().reserve(mysql_num_rows(res)); // 提前预留好空间
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) { // 每一行一次循环, 每一次循环都是一个group
                GroupUser guser;
                guser.setId(atoi(row[0]));
                guser.setName(row[1]);
                guser.setState(row[2]);
                guser.setRole(row[3]);
                g.getMembers().push_back(guser);
            }
            mysql_free_result(res);
        }
    }
    return groups;
}

// 根据groupid 查询群成员 除了 userid 的所有群成员id
vector<int> GroupModel::queryGroupUsers(int groupid, int userid)
{
    
    char sql[1024];
    sprintf(sql, "select userid from GroupUser where groupid = %d and userid != %d",
        groupid, userid);

    // MySQL mysql;
    vector<int> ids;
    // if(mysql.connect()) {
    //     MYSQL_RES* res = mysql.query(sql);
    //     if(res) {
    //         ids.reserve(mysql_num_rows(res));
    //         MYSQL_ROW row;
    //         while((row = mysql_fetch_row(res)) != nullptr) {
    //             ids.push_back(atoi(row[0]));
    //         }
    //         mysql_free_result(res);
    //     }
    // }

    shared_ptr<MySqlConn> conn = ConnectionPool::getInstance().getConnection();
    MYSQL_RES* res = conn->query(sql);
    if(res) {
        ids.reserve(mysql_num_rows(res));
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res)) != nullptr) {
            ids.push_back(atoi(row[0]));
        }
        mysql_free_result(res);
    }
    return ids;

}
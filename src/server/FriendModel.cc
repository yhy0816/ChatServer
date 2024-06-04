#include "FriendModel.hpp"
#include "MySQL.hpp"
#include <cstdio>
#include <mysql/mysql.h>

bool FriendModel::insert(int userid, int friendid)
{
    char sql[1024];

    sprintf(sql, "insert into Friend(userid, friendid) values(%d, %d)",
        userid, friendid);

    MySQL mysql;

    return mysql.connect() && mysql.update(sql);
}
vector<User> FriendModel::query(int userid)
{
    char sql[1024];
    sprintf(sql, "select id, name, state from Friend join "
                 "User on friendid = id where userid = %d ",
        userid);

    MySQL mysql;
    vector<User> frinedids;
    if (mysql.connect()) { // 先连接成功
        MYSQL_RES* res = mysql.query(sql); // 查询结果
        if (res) { // 查询成功
            frinedids.reserve(mysql_num_rows(res)); // 先预留行数的容量
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) { // 循环插入vec中
                User user(atoi(row[0]), row[1], "", row[2]);
                frinedids.push_back(user);
            }
        }
    }

    return frinedids;
}
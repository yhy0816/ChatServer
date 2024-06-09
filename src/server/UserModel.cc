#include "UserModel.hpp"
#include "MySQL.hpp"
#include <muduo/base/Logging.h>
#include <mysql/mysql.h>
// using namespace std;

bool UserModel::insert(User& user)
{
    char sql[1024];

    sprintf(sql, "insert into User(name, password, state) values('%s', '%s', '%s')",
        user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    MySQL mysql;
    if(mysql.connect()) {
        if(mysql.update(sql)) {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id) {

    char sql[1024];

    sprintf(sql, "select id, name, password, state from User where id = %d", id);

    MySQL mysql;
    User user;
    if(mysql.connect()) {
        MYSQL_RES* result = mysql.query(sql);
        if(result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if(row) {
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(result);  
            }     
        }
    }
    return user;
}

bool UserModel::updateState(const User& user) {
    char sql[1024];

    sprintf(sql, "update User set state = '%s' where id = %d",
            user.getState().c_str(), user.getId());
    // LOG_INFO << sql;
    MySQL mysql;

    return mysql.connect() && mysql.update(sql); // 连接并且执行 sql 
}

void UserModel::resetState() {
    char sql[1024] = "update User set state = 'offline' where state = 'online'";
    MySQL mysql;
    mysql.connect() && mysql.update(sql); 
}
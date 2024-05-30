#include "OfflineMsgModel.hpp"
#include "MySQL.hpp"
#include <mysql/mysql.h>

bool OfflineMsgModel::insert(int id, const string& msg)
{
    char sql[1024];

    sprintf(sql, "insert into OfflineMessage(userid, message) values( %d, '%s')",
        id, msg.c_str());
    MySQL mysql;
    return mysql.connect() && mysql.update(sql);
}

bool OfflineMsgModel::remove(int id)
{
    char sql[1024];

    sprintf(sql, "delete from OfflineMessage where userid = %d", id);
    MySQL mysql;
    return mysql.connect() && mysql.update(sql);
}

vector<string> OfflineMsgModel::query(int id)
{
    char sql[1024];

    sprintf(sql, "select message from OfflineMessage where userid = %d", id);
    vector<string> msgs;
    MySQL mysql;
    if (mysql.connect()) {

        MYSQL_RES* res = mysql.query(sql);
        if (res) {
            msgs.reserve(mysql_num_rows(res));
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                msgs.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }

    return msgs;
}
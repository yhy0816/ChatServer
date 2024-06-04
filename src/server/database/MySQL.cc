#include "MySQL.hpp"
#include <mysql/mysql.h>
#include <muduo/base/Logging.h>
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

MySQL::MySQL(){
    _conn = mysql_init(nullptr); 
}

MySQL::~MySQL(){

    if (_conn) {
        mysql_close(_conn);
    }
}

bool MySQL::connect(){
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
        password.c_str(), dbname.c_str(), 3306,
        nullptr, 0);
    if(p) {
        mysql_query(_conn, "set names utf8"); // 防止中文乱码
        // LOG_INFO << "connect mysql success!";
    } else {
        LOG_INFO << "connect mysql error!";
    }
    return p;
}

bool MySQL::update(const string& sql){
    //如果查询成功，返回0。如果出现错误，返回非0值。
    if(mysql_query(_conn, sql.c_str())) {
        LOG_INFO << sql << " 更新失败!!";
        return false;
    }
    
    return true;
}

MYSQL_RES* MySQL::query(const string& sql){
    //如果查询成功，返回0。如果出现错误，返回非0值。
    if(mysql_query(_conn, sql.c_str())) {
        LOG_INFO << sql << " 查询失败";
        return nullptr;
    }
    
    return mysql_use_result(_conn);
}
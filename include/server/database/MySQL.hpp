#pragma once


#include <mysql/mysql.h>
#include <string>
using namespace std;
class MySQL {

public:
    MySQL();
    ~MySQL();

    bool connect();
    bool update(const string& sql);
    MYSQL_RES* query(const string& sql);
    MYSQL* getConnection() const {return _conn;}
private:
    MYSQL* _conn;
    
};
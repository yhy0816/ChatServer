#pragma once

#include "User.hpp"

// 群组成员用户的 ORM 类, 只比user 多了一个 role (用户组内身份)成员变量， 
class GroupUser : public User{

public:
    GroupUser(int id = -1, const string& name = "", 
        const string& pwd = "", const string& state = "offline",const string&  role = "normal") :
        User(id, name, pwd, state), role(role){}
    void setRole(const string& role) {this->role = role;}
    string getRole() const {return this->role;}
private:
    string role;
};
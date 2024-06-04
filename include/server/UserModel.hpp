#pragma once
#include "User.hpp"

class UserModel {

public:
    bool insert(User& user);
    User query(int id);
    bool updateState(const User& user);
    //重置所有用户状态为offline
    void resetState();
private:

};
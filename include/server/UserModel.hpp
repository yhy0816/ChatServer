#pragma once
#include "User.hpp"

class UserModel {

public:
    bool insert(User& user);
    User query(int id);
    bool updateState(const User& user);
private:

};
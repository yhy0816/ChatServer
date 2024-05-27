#pragma once

#include <string>
using namespace std;


//匹配user表的ORM类
class User {
public:
    User(int id = -1, const string& name = "", 
        const string& pwd = "", const string& state = "offline") :
        id(id), name(name), password(pwd), state(state) {}

    int getId() const {return id;}
    string getName() const {return name;}
    string getPassword() const {return password;}
    string getState() const {return state;}
    void setId(int id) {
        this->id = id;
    }
    void setName(const string& name) {
        this->name = name;
    }
    void setPassword(const string& pwd) {
        this->password = pwd;
    }
    void setState(const string& state) {
        this->state = state;
    }

private:
    int id;
    string name;
    string password;
    string state;
};
#pragma once
#include <string>
#include <vector>
#include "GroupUser.hpp"
using namespace std;


class Group {
public:
    Group(int id = -1, const string& name = "", const string& desc = "") {
        this->id = id;
        this->name = name;
        this->desc = desc;
    }

    int getId() const {return id;} 
    string getName() const {return name;}
    string getDesc() const {return desc;}
    void setId(int id) {this->id = id;;}
    void setName(const string& name) {this->name = name;}
    void setDesc(const string& desc) {this->desc = desc;}
    vector<GroupUser> &getMembers()  {return members;}  

protected:
    int id;
    string name;
    string desc;
    vector<GroupUser> members;
};
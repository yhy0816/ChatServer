create database if not exists chat;

use chat;

create table if not exists User (

    id int primary key auto_increment,  # 用户 id
    name varchar(50) not null unique,  # 用户名
    password varchar(50) not null,  # 密码 
    state enum('online', 'offline')  # 用户当前登录状态: 在线、 离线
);

create table if not exists Friend (
    userid int not null,  # 用户 id
    firendid int not null, # 好友 id
    primary key(userid, firendid)  
);


create table if not exists AllGroup (
    id int primary key auto_increment,  # 组id
    groupname varchar(50) not null unique, # 组名
    groupdesc varchar(200) default ""  # 组功能描述
);

create table if not exists GroupUser (
    groupid int not null, # 组id
    userid int not null, # 组员id
    grouprole enum('creator', 'normal'), # 组内角色
    primary key(groupid, userid)
);

create table if not exists OfflineMessage (
    userid int not null,  # 用户 id
    message varchar(500) not null  # 离线消息 json 字符串
);
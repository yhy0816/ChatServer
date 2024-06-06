#pragma once
// 消息的类型
enum class EnMsgType {
    LOGIN_MSG = 1, // 登录消息
    LOGIN_MSG_ACK, // 登录响应消息
    LOGOUT_MSG, // 注销消息
    REG_MSG, // 注册消息
    REG_MSG_ACK, // 注册响应消息
    ONE_CHAT_MSG, // 私聊消息
    FRIEND_REQUEST_MSG, //  好友请求消息
    FRIEND_AGREE_MSG, // 同意好友请求消息
    CREATE_GROUP_MSG, //建群消息
    CREATE_GROUP_MSG_ACK, // 建群消息响应
    ADD_GROUP_MSG, // 加入群聊消息
    ADD_GROUP_MSG_ACK, //  加入群聊消息响应
    GROUP_CHAT_MSG, // 群聊聊天消息

};
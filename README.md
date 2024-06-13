# 基于 muduo 网络库的在线聊天服务器项目

## 目录/文件说明

- include 头文件目录
- src 源文件目录
- thirdparty 第三方依赖文件
- autobuild.sh 编译脚本
- create.sql 创建数据库脚本

## 依赖环境 

- Muduo
- MySQL
- Redis
- Nginx (编译安装 添加 --with-stream 激活 tcp 负载均衡模块)

## 配置

Nginx 配置文件全局块需要添加配置
以下是一个示例：

```nginx
stream {
    upstream MyServer {
        # weight 都为 1 基于轮询的方式
        # max_fails 心跳机制允许的最大失败次数
        # fail_timeout 心跳机制超时时间
        # IP1:PORT1 集群服务器 ip + 端口
        server IP1:PORT1 weight=1 max_fails=3 fail_timeout=30s;
        server IP2:PORT2 weight=1 max_fails=3 fail_timeout=30s;
    }
    server{
        # 连接超时时间
        proxy_connect_timeout 1s;
        # nginx 监听端口, 客户端需要向这个端口连接
        listen 8000;
        # 自己取的名字, 8000 端口的连接都会在 MyServer这里进行负载均衡
        proxy_pass MyServer;
        # 关闭Nagle算法的选项
        tcp_nodelay on;
    }
}

```
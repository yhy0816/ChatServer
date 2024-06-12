# 自动识别当前目录下所有源文件， 放在 SRC_LIST 变量中
aux_source_directory(. SRC_LIST)
aux_source_directory(database DB_SRC_LIST)
aux_source_directory(redis REDIS_SRC_LIST)
# 指定生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_SRC_LIST} ${REDIS_SRC_LIST})


#指定链接时依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)
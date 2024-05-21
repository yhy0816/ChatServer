# 自动识别当前目录下所有源文件， 放在 SRC_LIST 变量中
aux_source_directory(. SRC_LIST )

# 指定生成可执行文件
add_executable(CharServer ${SRC_LIST})

#指定链接时依赖的库文件
target_link_libraries(CharServer muduo_net muduo_base pthread)
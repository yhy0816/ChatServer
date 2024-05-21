cmake_minimum_required(VERSION 3.0.0)
project(ChatServer VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
# 设置可执行文件的输出路径

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#头文件的搜索路径
include_directories(include/server)
include_directories(include)


# 加载子目录
add_subdirectory(src)
# include(CTest)

# enable_testing()

# add_executable(ChatServer)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

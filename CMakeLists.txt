project(system_status_loger)

cmake_minimum_required(VERSION 3.5)
add_compile_options(-std=c++11)

find_package(Threads REQUIRED)


set(source src/main.cpp)

add_executable(system_status_loger ${source})


target_link_libraries(system_status_loger pthread)
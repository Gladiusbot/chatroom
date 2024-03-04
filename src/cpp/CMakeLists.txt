cmake_minimum_required(VERSION 3.0)
project(YouChat)

set (CMAKE_CXX_STANDARD 14)

add_executable(client Client.cpp)
set_target_properties(client PROPERTIES CXX_STANDARD 14)


add_executable(server Server.cpp Session.cpp)
set_target_properties(server PROPERTIES 
  CXX_STANDARD 14
  COMPILE_FLAGS "-g"
  LINK_FLAGS "-g")
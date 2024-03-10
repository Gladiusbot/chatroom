cmake_minimum_required(VERSION 3.0)
project(YouChat)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(client Client.cpp)
set_target_properties(client PROPERTIES CXX_STANDARD 14)


add_executable(server Server.cpp Session.cpp)
set_target_properties(server PROPERTIES 
  CXX_STANDARD 14
  COMPILE_FLAGS "-g"
  LINK_FLAGS "-g")

set(CPPCHECK_EXECUTABLE /usr/bin/cppcheck)

add_custom_command(TARGET server
                  POST_BUILD
                  COMMAND ${CPPCHECK_EXECUTABLE} --enable=all --suppress=missingIncludeSystem
                  --std=c++14 --force --quiet --project=compile_commands.json
                  COMMENT "Running cppcheck for server")

add_custom_command(TARGET client 
                  POST_BUILD
                  COMMAND ${CPPCHECK_EXECUTABLE} --enable=all --suppress=missingIncludeSystem
                  --std=c++14 --force --quiet --project=compile_commands.json
                  COMMENT "Running cppcheck for client")

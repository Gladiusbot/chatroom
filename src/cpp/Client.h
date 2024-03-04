#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <thread>

#define SERVER_PORT_NUMBER 58000
#define CLIENT_PORT_NUMBER 58001
#define DEBUG_CLIENT true
#define MAX_LENGTH 1024
#define HEAD_LENGTH 2
#define LOCAL_SERVER

#ifdef LOCAL_SERVER
const std::string server_ip = "127.0.0.1";
#else
const std::string server_ip = "39.104.209.73";
#endif
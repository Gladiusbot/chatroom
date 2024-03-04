#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>

#include "Session.h"

// using namespace boost;

class Server {
 public:
  Server(boost::asio::io_context& io_context, unsigned short port);
  void ClearSession(std::string);

 private:
  void HandleAccept(std::shared_ptr<Session>,
                    const boost::system::error_code& error);
  void StartAccept();
  boost::asio::io_context& _io_context;
  unsigned short _port;
  boost::asio::ip::tcp::acceptor _acceptor;
  std::map<std::string, std::shared_ptr<Session>> _sessions;
};
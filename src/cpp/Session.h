#pragma once

#include <memory.h>

#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <mutex>
#include <queue>

#include "const.h"

class Server;

class MessageNode {
  friend class Session;

 private:
  unsigned short _cur_len;
  unsigned short _total_len;
  char* _data;

 public:
  MessageNode(char* msg, unsigned short max_len)
      : _total_len(max_len + HEAD_LENGTH), _cur_len(0) {
    _data = new char[_total_len + 1]();
    int max_len_host =
        boost::asio::detail::socket_ops::host_to_network_short(max_len);
    memcpy(_data, &max_len_host, HEAD_LENGTH);
    memcpy(_data + HEAD_LENGTH, msg, max_len);
    _data[_total_len] = '\0';
  }

  MessageNode(unsigned short max_len) : _total_len(max_len), _cur_len(0) {
    _data = new char[_total_len + 1]{};
  }

  ~MessageNode() { delete[] _data; }

  void Clear() {
    ::memset(_data, 0, _total_len);
    _cur_len = 0;
  }
};

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(boost::asio::io_context& io_context, Server* server);
  ~Session();
  boost::asio::ip::tcp::socket& GetSocket();
  const std::string& GetUuid();
  void Start();
  void Send(char* msg, int max_length);
  void Close();
  std::shared_ptr<Session> SharedSelf();

 private:
  void HandleRead(const boost::system::error_code& error,
                  size_t bytes_tranferred,
                  std::shared_ptr<Session> shared_self);
  void HandleWrite(const boost::system::error_code& error,
                   std::shared_ptr<Session> shared_self);
  boost::asio::ip::tcp::socket _socket;
  std::string _uuid;
  char _data[MAX_LENGTH];
  Server* _server;
  bool _b_close;
  std::queue<std::shared_ptr<MessageNode>> _send_que;
  std::mutex _send_mtx;
  std::shared_ptr<MessageNode> _recv_msg_node;
  bool _b_head_parsed;
  std::shared_ptr<MessageNode> _recv_head_node;
};
#include "Session.h"

#include "Server.h"

Session::Session(boost::asio::io_context& io_context, Server* server)
    : _socket(io_context),
      _server(server),
      _b_close(false),
      _b_head_parsed(false) {
  boost::uuids::uuid temp_uuid = boost::uuids::random_generator()();
  _uuid = boost::uuids::to_string(std::move(temp_uuid));
  _recv_head_node = std::make_shared<MessageNode>(HEAD_LENGTH);
}

Session::~Session() { std::cout << "Session dtor" << std::endl; }

boost::asio::ip::tcp::socket& Session::GetSocket() { return _socket; }

const std::string& Session::GetUuid() { return _uuid; }

void Session::Start() {
  ::memset(_data, 0, MAX_LENGTH);
  _socket.async_read_some(
      boost::asio::buffer(_data, MAX_LENGTH),
      std::bind(&Session::HandleRead, this, std::placeholders::_1,
                std::placeholders::_2, SharedSelf()));
}

void Session::Send(char* msg, int max_length) {
  std::lock_guard<std::mutex> lock(_send_mtx);
  int send_que_size = _send_que.size();
  if (send_que_size > MAX_SENDQUE) {
    std::cout << "session: " << _uuid
              << " send queue is full, size: " << send_que_size << std::endl;
    return;
  }

  _send_que.push(std::make_shared<MessageNode>(msg, max_length));
  if (send_que_size > 0) {
    return;
  }

  auto& msgnode = _send_que.front();
  boost::asio::async_write(
      _socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
      std::bind(&Session::HandleWrite, this, std::placeholders::_1,
                SharedSelf()));
}

void Session::Close() {
  _socket.close();
  _b_close = true;
}

std::shared_ptr<Session> Session::SharedSelf() { return shared_from_this(); }

void Session::HandleWrite(const boost::system::error_code& error,
                          std::shared_ptr<Session> shared_self) {
  if (!error) {
    std::lock_guard<std::mutex> lock(_send_mtx);
    std::cout << "sending data: " << _send_que.front()->_data + HEAD_LENGTH
              << std::endl;
    _send_que.pop();
    if (!_send_que.empty()) {
      auto& msgnode = _send_que.front();
      boost::asio::async_write(
          _socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
          std::bind(&Session::HandleWrite, this, std::placeholders::_1,
                    shared_self));
    }
  }
}

void Session::HandleRead(const boost::system::error_code& error,
                         size_t bytes_transferred,
                         std::shared_ptr<Session> shared_self) {
  if (!error) {
    int copy_len = 0;
    while (bytes_transferred > 0) {
      //  上一次头部未完全接收
      if (!_b_head_parsed) {
        if (bytes_transferred + _recv_head_node->_cur_len < HEAD_LENGTH) {
          memcpy(_recv_head_node->_data + _recv_head_node->_cur_len,
                 _data + copy_len, bytes_transferred);
          _recv_head_node->_cur_len += bytes_transferred;
          // TODO why global?
          ::memset(_data, 0, MAX_LENGTH);
          _socket.async_read_some(
              boost::asio::buffer(_data, MAX_LENGTH),
              std::bind(&Session::HandleRead, this, std::placeholders::_1,
                        std::placeholders::_2, shared_self));
          return;
        }
        // 头部 + ..
        // 计算补全头部所需的长度
        int head_remain = HEAD_LENGTH - _recv_head_node->_cur_len;
        memcpy(_recv_head_node->_data + _recv_head_node->_cur_len,
               _data + copy_len, head_remain);
        copy_len += head_remain;
        bytes_transferred -= head_remain;
        unsigned short data_len = 0;
        memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH);
        // 字节序转换
        data_len =
            boost::asio::detail::socket_ops::network_to_host_short(data_len);
        std::cout << "data body length is: " << data_len << std::endl;
        if (data_len > MAX_LENGTH) {
          std::cout << "invalid data body length: " << data_len << std::endl;
          _server->ClearSession(_uuid);
          return;
        }
        _recv_msg_node = std::make_shared<MessageNode>(data_len);

        // 消息长度小于消息头提供的长度, 说明未完全接受
        if (bytes_transferred < data_len) {
          memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len,
                 _data + copy_len, bytes_transferred);
          _recv_msg_node->_cur_len += bytes_transferred;
          ::memset(_data, 0, MAX_LENGTH);
          _socket.async_read_some(
              boost::asio::buffer(_data, MAX_LENGTH),
              std::bind(&Session::HandleRead, this, std::placeholders::_1,
                        std::placeholders::_2, shared_self));
          _b_head_parsed = true;
          return;
        }

        memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len,
               _data + copy_len, data_len);
        _recv_msg_node->_cur_len += data_len;
        copy_len += data_len;
        bytes_transferred -= data_len;
        _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
        std::cout << "received data: " << _recv_msg_node->_data << std::endl;
        // TODO echo server for now
        Send(_recv_msg_node->_data, _recv_msg_node->_total_len);
        // 继续处理剩余数据
        _b_head_parsed = false;
        _recv_head_node->Clear();
        if (bytes_transferred <= 0) {
          ::memset(_data, 0, MAX_LENGTH);
          _socket.async_read_some(
              boost::asio::buffer(_data, MAX_LENGTH),
              std::bind(&Session::HandleRead, this, std::placeholders::_1,
                        std::placeholders::_2, shared_self));
          return;
        }
        continue;
      }

      // 已经处理完头部, 处理上次剩余的消息
      int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
      if (bytes_transferred < remain_msg) {
        memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len,
               _data + copy_len, bytes_transferred);
        _recv_msg_node->_cur_len += bytes_transferred;
        ::memset(_data, 0, MAX_LENGTH);
        _socket.async_read_some(
            boost::asio::buffer(_data, MAX_LENGTH),
            std::bind(&Session::HandleRead, this, std::placeholders::_1,
                      std::placeholders::_2, shared_self));
        return;
      }
      memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len,
             remain_msg);
      _recv_msg_node->_cur_len += remain_msg;
      bytes_transferred -= remain_msg;
      copy_len += remain_msg;
      _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
      std::cout << "received data: " << _recv_msg_node->_data << std::endl;
      // TODO echo server for now
      _b_head_parsed = false;
      _recv_head_node->Clear();
      if (bytes_transferred <= 0) {
        ::memset(_data, 0, MAX_LENGTH);
        _socket.async_read_some(
            boost::asio::buffer(_data, MAX_LENGTH),
            std::bind(&Session::HandleRead, this, std::placeholders::_1,
                      std::placeholders::_2, shared_self));
        return;
      }
      continue;
    }
  } else {
    std::cout << "handle read failed, error code: " << error.message()
              << std::endl;
    Close();
    _server->ClearSession(_uuid);
  }
}
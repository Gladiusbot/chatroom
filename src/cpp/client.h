#include <boost/asio.hpp>
#include <iostream>

#define SERVER_PORT_NUMBER 58000
#define CLIENT_PORT_NUMBER 58001
#define DEBUG_CLIENT true

extern void error_report();
extern int client_end_point();
extern int create_tcp_socket();
extern int connect_to_end();
template <typename ConstBufferSequence>
std::size_t send(const ConstBufferSequence& buffers);
template <typename BufferType>
std::shared_ptr<BufferType> string_to_buffer(std::string input);

class Client {
  unsigned short port_num;
};
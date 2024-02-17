#include <boost/asio.hpp>
#include <iostream>

// using namespace boost;
#define SERVER_PORT_NUMBER 58000
#define DEBUG_SERVER true

extern void error_report();
extern int server_end_point();
extern int create_tcp_socket();
extern int create_acceptor_socket();
extern int bind_acceptor_socket();
extern int accept_new_connection();
template <typename ConstBufferSequence>
std::size_t send(const ConstBufferSequence& buffers);
template <typename BufferType>
std::shared_ptr<BufferType> string_to_buffer(std::string input);

class Server {
  unsigned short port_num;
};
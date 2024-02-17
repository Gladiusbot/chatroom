#include "server.h"

using namespace boost;

void error_report(const std::string& file_name, const int line_number,
                  const system::system_error& e) {
  std::cout << "Error occured " << file_name << ", line " << line_number
            << ". Error Code:" << e.code() << ", " << e.what();
}

int server_end_point() {
  unsigned int port_num = 3333;
  asio::ip::address ip_address = asio::ip::address_v4::any();
  return 0;
}

int create_tcp_socket() {
  asio::io_context ios;
  asio::ip::tcp protocol = asio::ip::tcp::v4();
  asio::ip::tcp::socket sock(ios);
  boost::system::error_code ec;
  sock.open(protocol, ec);

  if (ec.value() != 0) {
    std::cout << "Failed to open new socket, Error Code:" << ec.value() << ", "
              << ec.message();
    return ec.value();
  }

  return 0;
}

int create_acceptor_socket() {
  asio::io_context ios;
  asio::ip::tcp protocol = asio::ip::tcp::v4();
  asio::ip::tcp::acceptor acceptor(ios);
  boost::system::error_code ec;
  acceptor.open(protocol, ec);
  if (ec.value() != 0) {
    std::cout << "Failed to open acceptor socket, Error Code:" << ec.value()
              << ", " << ec.message();
    return ec.value();
  }
  return 0;
}

int bind_acceptor_socket() {
  unsigned short port_num = SERVER_PORT_NUMBER;
  asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
  asio::io_context ios;
  asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
  boost::system::error_code ec;
  acceptor.bind(ep, ec);
  if (ec.value() != 0) {
    std::cout << "Failed to bind acceptor socket, Error Code:" << ec.value()
              << ", " << ec.message();
    return ec.value();
  }
  return 0;
}

int accept_new_connection() {
  const int BACKLOG_SIZE = 30;
  unsigned short port_num = SERVER_PORT_NUMBER;
  asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
  asio::io_context ios;
  try {
    asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
    acceptor.bind(ep);
    acceptor.listen(BACKLOG_SIZE);
    asio::ip::tcp::socket sock(ios);
    acceptor.accept(sock);
  } catch (system::system_error& e) {
    error_report(__FILE__, __LINE__, e);
    return e.code().value();
  }
  return 0;
}

std::shared_ptr<asio::const_buffers_1> string_to_buffer(std::string input) {
  std::shared_ptr<asio::const_buffers_1> output_buffer =
      std::make_shared<asio::const_buffers_1>(asio::buffer(input));
  return output_buffer;
}

int main() {
  int ret = server_end_point();
  std::cout << "server_end_point ret:" << ret << "\n";
}

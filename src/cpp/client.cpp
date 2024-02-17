#include "client.h"

using namespace boost;

void error_report(const std::string& file_name, const int line_number,
                  const system::system_error& e) {
  std::cout << "Error occured " << file_name << ", line " << line_number
            << ". Error Code:" << e.code() << ", " << e.what();
}

int client_end_point() {
#ifdef DEBUG_CLIENT
  std::string raw_ip_address = "127.0.0.1";
#else
  std::string raw_ip_address = "TODO INPUT SERVER IP";
#endif
  unsigned int port_num = CLIENT_PORT_NUMBER;
  system::error_code ec;
  asio::ip::address ip_address =
      asio::ip::address::from_string(raw_ip_address, ec);
  if (ec.value() != 0) {
    std::cout << "Failed to parse IP, Error Code:" << ec.value() << ", "
              << ec.message() << std::endl;
    return ec.value();
  }
  boost::asio::ip::tcp::endpoint ep(ip_address, port_num);
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

// client connect to server end
int connect_to_end() {
#ifdef DEBUG_CLIENT
  std::string raw_ip_address = "127.0.0.1";
#else
  std::string raw_ip_address = "TODO, INPUT IP";
#endif
  unsigned short port_num = CLIENT_PORT_NUMBER;
  try {
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address),
                               port_num);
    asio::io_context ios;
    asio::ip::tcp::socket sock(ios, ep.protocol());
    sock.connect(ep);
  } catch (system::system_error& e) {
    error_report(__FILE__, __LINE__, e);
    return e.code().value();
  }
  return 0;
}

// convert std::string to const buffer
std::shared_ptr<asio::const_buffers_1> string_to_buffer(
    const std::string& input) {
  std::shared_ptr<asio::const_buffers_1> output_buffer =
      std::make_shared<asio::const_buffers_1>(asio::buffer(input));
  return output_buffer;
}

int main() {
  int ret = client_end_point();
  std::cout << "client_end_point ret:" << ret << "\n";
}

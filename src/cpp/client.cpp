#include "client.h"

using namespace boost;

int main() {
  try {
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::endpoint remote_ep(
        boost::asio::ip::address::from_string(server_ip), SERVER_PORT_NUMBER);
    boost::asio::ip::tcp::socket sock(ioc);
    boost::system::error_code error = boost::asio::error::host_not_found;
    sock.connect(remote_ep, error);
    if (error) {
      std::cout << "connect failed, error code:" << error.value() << ", \""
                << error.message() << "\"\n";
      return 0;
    }
    for (;;) {
      std::cout << "Enter message: ";
      char request[MAX_LENGTH];
      std::cin.getline(request, MAX_LENGTH);
      size_t request_length = strlen(request);
      boost::asio::write(sock, boost::asio::buffer(request, request_length));

      char reply[MAX_LENGTH];
      size_t reply_length =
          boost::asio::read(sock, boost::asio::buffer(reply, request_length));
      std::cout << "Reply is: ";
      std::cout.write(reply, reply_length);
      std::cout << "\n";
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
  return 0;
}
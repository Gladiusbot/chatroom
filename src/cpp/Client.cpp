#include "Client.h"

int main() {
  std::string user_input;
  std::getline(std::cin, user_input);
  try {
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::endpoint remote_ep(
        boost::asio::ip::address::from_string(server_ip), SERVER_PORT_NUMBER);
    boost::asio::ip::tcp::socket sock(ioc);
    boost::system::error_code error = boost::asio::error::host_not_found;
    sock.connect(remote_ep, error);
    if (error) {
      std::cout << "connect failed, error code: " << error.value()
                << " error msg: " << error.message();
      return 0;
    }

    std::thread send_thread([&sock, &user_input]() {
      for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        const char* request = user_input.c_str();
        // const char* request = "hello world!";
        short request_length = strlen(request);
        char send_data[MAX_LENGTH] = {0};
        // 转为网络字节序
        short request_host_length =
            boost::asio::detail::socket_ops::host_to_network_short(
                request_length);
        memcpy(send_data, &request_host_length, 2);
        memcpy(send_data + 2, request, request_length);
        boost::asio::write(sock,
                           boost::asio::buffer(send_data, request_length + 2));
      }
    });

    std::thread recv_thread([&sock]() {
      for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        std::cout << "begin to receive..." << std::endl;
        char reply_head[HEAD_LENGTH];
        size_t reply_length = boost::asio::read(
            sock, boost::asio::buffer(reply_head, HEAD_LENGTH));
        unsigned short msglen = 0;
        memcpy(&msglen, reply_head, HEAD_LENGTH);
        // 转为本地字节序
        msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);
        char msg[MAX_LENGTH] = {0};
        size_t msg_length =
            boost::asio::read(sock, boost::asio::buffer(msg, msglen));

        std::cout << "Reply is: ";
        std::cout.write(msg, msglen) << std::endl;
        std::cout << "Reply len is: " << msglen;
        std::cout << "\n";
      }
    });
    send_thread.join();
    recv_thread.join();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}
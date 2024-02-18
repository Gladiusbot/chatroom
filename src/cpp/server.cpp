#include "server.h"

typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
std::set<std::shared_ptr<std::thread>> thread_set;

void session(socket_ptr sock) {
  try {
    for (;;) {
      char data[MAX_LENGTH];
      memset(data, '\0', MAX_LENGTH);
      boost::system::error_code error;
      size_t length =
          sock->read_some(boost::asio::buffer(data, MAX_LENGTH), error);
      if (error == boost::asio::error::eof) {
        std::cout << "connection closed by peer" << std::endl;
        break;
      } else if (error) {
        throw boost::system::system_error(error);
      }
      std::cout << "receive from "
                << sock->remote_endpoint().address().to_string() << std::endl;
      std::cout << "receive message is " << data << std::endl;
      boost::asio::write(*sock, boost::asio::buffer(data, length));
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
}

void server(boost::asio::io_context& io_context, unsigned short port) {
  // 通过传入的上下文和新建的endpoint创建acceptor
  boost::asio::ip::tcp::acceptor a(
      io_context,
      boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
  // 创建服务线程
  for (;;) {
    socket_ptr socket(new boost::asio::ip::tcp::socket(io_context));
    a.accept(*socket);
    auto t = std::make_shared<std::thread>(session, socket);
    thread_set.insert(t);
  }
}

int main() {
  try {
    // 创建上下文
    boost::asio::io_context ioc;
    // 通过上下文创建服务线程
    server(ioc, SERVER_PORT_NUMBER);
    for (auto& t : thread_set) {
      t->join();
    }
  } catch (std::exception& e) {
    std::cerr << "Exception " << e.what() << "\n";
    return -1;
  }
  return 0;
}
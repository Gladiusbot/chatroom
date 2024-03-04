#include "Server.h"

Server::Server(boost::asio::io_context& io_context, unsigned short port)
    : _io_context(io_context),
      _port(port),
      _acceptor(io_context, boost::asio::ip::tcp::endpoint(
                                boost::asio::ip::tcp::v4(), port)) {
  std::cout << "Server started, listening on port" << _port << std::endl;
  StartAccept();
}

void Server::HandleAccept(std::shared_ptr<Session> new_session,
                          const boost::system::error_code& error) {
  if (!error) {
    new_session->Start();
    _sessions.insert(make_pair(new_session->GetUuid(), new_session));
  } else {
    std::cout << "session accept failed, error code: " << error.message()
              << std::endl;
  }

  StartAccept();
}

void Server::StartAccept() {
  std::shared_ptr<Session> new_session =
      std::make_shared<Session>(_io_context, this);
  _acceptor.async_accept(new_session->GetSocket(),
                         std::bind(&Server::HandleAccept, this, new_session,
                                   std::placeholders::_1));
}

void Server::ClearSession(std::string uuid) { _sessions.erase(uuid); }

int main() {
  try {
    boost::asio::io_context io_contxt;
    Server srv(io_contxt, 58000);
    io_contxt.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  boost::asio::io_context io_contxt;
}
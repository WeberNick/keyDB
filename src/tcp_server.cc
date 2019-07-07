#include "tcp_server.hh"
#include "tcp_connection.hh"

#include <thread>

#include <iostream>

tcp_server::tcp_server(boost::asio::io_service& io_service, unsigned aPort) noexcept
    : m_acceptor(io_service, tcp::endpoint(tcp::v4(), aPort))
{
    start_accept();
}

tcp_server::~tcp_server() noexcept = default;

void tcp_server::start_accept() noexcept
{
    std::cout << "SERVER: Start Async Accept" << std::endl;
    tcp_connection::pointer new_connection = tcp_connection::create(acceptor().get_io_service());
    acceptor().async_accept
    (
        new_connection->socket(),
        [new_connection, this](const boost::system::error_code& ec)
        {
            if(!ec)
            {
                std::cout << "SERVER: Spawn Thread to handle new connection" << std::endl;
                std::thread(&tcp_connection::start, new_connection).detach();
            }
            start_accept();
        }
    );
}


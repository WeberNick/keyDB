#pragma once

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class tcp_server final
{
    public:
        tcp_server()                                                    noexcept = delete;
        tcp_server(boost::asio::io_service& io_service, unsigned aPort) noexcept;
        tcp_server(const tcp_server&)                                   noexcept = delete;
        tcp_server& operator=(const tcp_server&)                        noexcept = delete;
        tcp_server(tcp_server&&)                                        noexcept = delete;
        tcp_server& operator=(tcp_server&&)                             noexcept = delete;
        ~tcp_server()                                                   noexcept;

    private:
        void start_accept()                                             noexcept;
        auto& acceptor()                                                noexcept { return m_acceptor; }

    private:
        tcp::acceptor m_acceptor;
};


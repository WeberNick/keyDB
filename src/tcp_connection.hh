#pragma once
#include "types.hh"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <string>

using boost::asio::ip::tcp;

class tcp_connection : public boost::enable_shared_from_this<tcp_connection>
{
    public:
        using pointer = boost::shared_ptr<tcp_connection>;
        ~tcp_connection()                                           noexcept;

    public:
        static pointer  create(boost::asio::io_service& io_service) noexcept;
        tcp::socket&    socket()                                    noexcept { return m_socket; }
        void            start()                                     noexcept;

    private:
        tcp_connection()                                            noexcept = delete;
        tcp_connection(boost::asio::io_service& io_service)         noexcept;
        tcp_connection(const tcp_connection&)                       noexcept = delete;
        tcp_connection& operator=(const tcp_connection&)            noexcept = delete;
        tcp_connection(tcp_connection&&)                            noexcept = delete;
        tcp_connection& operator=(tcp_connection&&)                 noexcept = delete;

    private:
        void            read()                                      noexcept;
        void            send(const answer_t& ans)                   noexcept;

    private:
        tcp::socket m_socket;
};

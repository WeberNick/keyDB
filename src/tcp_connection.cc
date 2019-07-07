#include "tcp_connection.hh"
#include "string_helper.hh"
#include "database.hh"

#include <boost/bind.hpp>

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

tcp_connection::~tcp_connection() noexcept = default;

tcp_connection::pointer tcp_connection::create(boost::asio::io_service& io_service) noexcept
{
    return pointer(new tcp_connection(io_service));
}

void tcp_connection::start() noexcept
{
    TRACE("CONNECTION: Started connection");
    std::cout << "CONNECTION: Started connection" << std::endl;
    read();
}

tcp_connection::tcp_connection(boost::asio::io_service& io_service) noexcept
    : m_socket(io_service)
{
}

void tcp_connection::read() noexcept
{
    boost::asio::streambuf buf;
    boost::asio::read_until
    ( 
        socket(), 
        buf, 
        '\n'
    );
    const std::string msg = boost::asio::buffer_cast<const char*>(buf.data());
    TRACE("Process Msg: " + msg);
    std::cout << "Process Msg: " << msg << std::endl;
    const string_vt args = StringUtil::splitString(StringUtil::trim_copy(msg), ' ');
    answer_t ans;
    if(valid_request(args))
    {
        ans = KeyValueStore<str_key, str_val>::get_instance().request_handler(args);
    }
    else
    {
        TRACE("Invalid Request");
        std::cerr << "Invalid Request" << std::endl;
        ans = std::make_pair("ERROR", "INVALID REQUEST: '" + msg + "'");
    }
    send(ans);
}

void tcp_connection::send(const answer_t& ans) noexcept
{
    boost::asio::write
    (
        socket(), 
        boost::asio::buffer(ans.first + ":" + ans.second + "\n")
    );
}

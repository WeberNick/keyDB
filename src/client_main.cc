#include "string_helper.hh"

#include <boost/asio.hpp>

#include <string>
#include <iostream>

using boost::asio::ip::tcp;

inline std::string make_string(boost::asio::streambuf& streambuf)
{
    return {buffers_begin(streambuf.data()), buffers_end(streambuf.data())};
}

int main()
{
    try
    {
        bool exit = false;
        std::string msg;
        string_vt ans_vec;
        std::string status;
        std::string ans;
        do
        {
            std::cout << "Enter Message: " << std::endl;
            std::getline (std::cin,msg);
            exit = msg == "END" || msg == "EXIT" || msg == "QUIT";
            if(!exit)
            {
                std::cout << "Send Message '" << msg << "' to Server" << std::endl;
                msg.append("\n");

                boost::asio::io_service io_service;
                tcp::resolver resolver(io_service);
                tcp::resolver::query query("localhost", "8080");
                tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
                tcp::socket socket(io_service);
                boost::asio::connect(socket, endpoint_iterator);

                boost::system::error_code error;

                boost::asio::write(socket, boost::asio::buffer(msg, msg.size()));

                boost::asio::streambuf buf;
                boost::asio::read_until(socket, buf, '\n');
                ans_vec = StringUtil::splitString(make_string(buf), ':');
                status = ans_vec.at(0);
                ans = ans_vec.at(1);
                if(status == "OK")
                {
                    std::cout << "\033[1;32m" << status << "\033[0m : \033[33m" << ans << "\033[0m";
                }
                else if(status == "ERROR")
                {
                    std::cout << "\033[1;31m" << status << "\033[0m : \033[33m" << ans << "\033[0m";
                
                }
                else
                {
                    std::cout << status << " : " << ans; 
                }
                std::cout << std::endl;
                

                if(error)
                    throw boost::system::system_error(error); // Some other error.
            }
        }
        while(!exit);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

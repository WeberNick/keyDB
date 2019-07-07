#include "args.hh"
#include "types.hh"
#include "trace.hh"

#include "database.hh"

#include "tcp_server.hh"
#include <boost/asio.hpp>

#include <chrono>
#include <thread>
#include <iostream>


int main(const int argc, const char* argv[])
{
    /* Parse Command Line Arguments */
    Args lArgs;
    argdesc_vt lArgDesc;
    construct_arg_desc(lArgDesc);

    if(!parse_args<Args>(1, argc, argv, lArgDesc, lArgs))
    {
        std::cerr << "Error while parsing arguments." << std::endl;
        return -1;
    } 
    if(lArgs.help())
    {
        print_usage(std::cout, argv[0], lArgDesc);
        return 0;
    }
    if(lArgs.trace() && !FileUtil::hasValidDir(lArgs.trace_path()))
    {
        std::cerr << "The trace file path is invalid." << std::endl;
        return -1;
    }

    const CB lCB(lArgs.trace(), lArgs.trace_path(), lArgs.buffer_size(), lArgs.port());


    Trace::get_instance().init(lCB);
    auto& kv_store = KeyValueStore<str_key, str_val>::get_instance();
    kv_store.init(lCB);

    try
    {
        boost::asio::io_service io_service;
        tcp_server server(io_service, lCB.port());
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
    
    const std::string key("MyKey");
    const std::string val("MyValue");

    for(size_t i = 0; i < 200000; ++i)
    {
        kv_store.put(str_key(key + std::to_string(i)), str_val(val + std::to_string(i)));
    }


    try
    {
        auto kv = kv_store.get(str_key(key + "0"));
        std::cout << "@@@ \tFound: " << kv << std::endl;
        kv = kv_store.get(str_key(key + "5"));
        std::cout << "@@@ \tFound: " << kv << std::endl;
        kv = kv_store.get(str_key(key + "18"));
        std::cout << "@@@ \tFound: " << kv << std::endl;
        kv = kv_store.get(str_key(key + "199999"));
        std::cout << "@@@ \tFound: " << kv << std::endl;
        std::cout << "@@@ \tLook for non existing key.." << std::endl;
        kv = kv_store.get(str_key(key + "100001"));
    }
    catch(std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}

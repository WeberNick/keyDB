#include <catch2/catch.hpp>

#include "../src/database.hh"

#include <thread>
#include <string>
#include <vector>
#include <iostream>

using key_type = string_t;
using value_type = string_t;
using key_val_type = key_val_t<key_type,value_type>;


constexpr uint RECORDS_PER_THREAD = 100;
const uint no_threads = std::thread::hardware_concurrency();
//const uint no_threads = 1;

const uint total_records = no_threads * RECORDS_PER_THREAD;


std::vector<key_val_type> key_value_vec;

void insert(KeyValueStore<key_type, value_type>& aKVS, uint aThreadNo);
void get(KeyValueStore<key_type, value_type>& aKVS, uint aThreadNo);
void del(KeyValueStore<key_type, value_type>& aKVS, uint aThreadNo);

void insert(KeyValueStore<key_type, value_type>& aKVS, uint aThreadNo)
{
    const uint from = aThreadNo * RECORDS_PER_THREAD;
    const uint to = ((aThreadNo + 1) * RECORDS_PER_THREAD) - 1;

    for(uint i = from; i <= to; ++i)
    {
        const auto& kv = key_value_vec.at(i);
        aKVS.put(kv.key(), kv.val());
    }
}

void get(KeyValueStore<key_type, value_type>& aKVS, uint aThreadNo)
{
    const uint from = aThreadNo * RECORDS_PER_THREAD;
    const uint to = ((aThreadNo + 1) * RECORDS_PER_THREAD) - 1;

    for(uint i = from; i <= to; ++i)
    {
        const auto& kv = key_value_vec.at(i);
        try
        {
            auto kv_tmp = aKVS.get(kv.key());
            REQUIRE(kv_tmp == kv);
        }
        catch(KeyNotInStorageManagerException& ex)
        {
            REQUIRE(i % 2 == 0);
        }
        catch(KeyIsDeletedInWriteManagerException& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }
}

void del(KeyValueStore<key_type, value_type>& aKVS, uint aThreadNo)
{
    const uint from = aThreadNo * RECORDS_PER_THREAD;
    const uint to = ((aThreadNo + 1) * RECORDS_PER_THREAD) - 1;

    for(uint i = from; i <= to; ++i)
    {
        const auto& kv = key_value_vec.at(i);
        if(i % 2 == 0)
        {
            aKVS.del(kv.key());
        }
        try
        {
            bool same = aKVS.get(kv.key()) == kv;
            REQUIRE(same);
        }
        catch(KeyNotInStorageManagerException& ex)
        {
            REQUIRE(false);
        }
        catch(KeyIsDeletedInWriteManagerException& ex)
        {
            REQUIRE(i % 2 == 0);
        }
    }
}


TEST_CASE( "database I/O tests", "[logic]" ) {

    const CB lCB(false, "", 10000, 8080u);
    Trace::get_instance().init(lCB);

    auto& kv_store = KeyValueStore<key_type, value_type>::get_instance();
    kv_store.init(lCB);


    key_value_vec.resize(total_records);
    const std::string TEST_PREFIX = "DBIO_";
    for(size_t i = 0; i < total_records; ++i)
    {
        key_value_vec.at(i) = key_val_type(key_type(TEST_PREFIX + "MyKey" + std::to_string(i)), value_type(TEST_PREFIX + "MyValue" + std::to_string(i)), MOD::kINSERT);
    }

    {
        std::cout << "Test multi threaded insertion" << std::endl;
        std::vector<std::thread> threads;
        for(uint i = 0; i < no_threads; ++i)
        {
            threads.emplace_back(&insert, std::ref(kv_store), i);
        }

        for(uint i = 0; i < no_threads; ++i)
        {
            threads.at(i).join();
        }
    }


    {
        std::cout << "Test multi threaded get" << std::endl;
        std::vector<std::thread> threads;
        for(uint i = 0; i < no_threads; ++i)
        {
            threads.emplace_back(&get, std::ref(kv_store), i);
        }

        for(uint i = 0; i < no_threads; ++i)
        {
            threads.at(i).join();
        }
    
    }

    {
        std::cout << "test multi threaded delete" << std::endl;
        std::vector<std::thread> threads;
        for(uint i = 0; i < no_threads; ++i)
        {
            threads.emplace_back(&del, std::ref(kv_store), i);
        }

        for(uint i = 0; i < no_threads; ++i)
        {
            threads.at(i).join();
        }

        kv_store.flush();
    }

    {
        std::cout << "test multi threaded get after delete" << std::endl;
        std::vector<std::thread> threads;
        for(uint i = 0; i < no_threads; ++i)
        {
            threads.emplace_back(&get, std::ref(kv_store), i);
        }

        for(uint i = 0; i < no_threads; ++i)
        {
            threads.at(i).join();
        }
    }
}


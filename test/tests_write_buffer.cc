#include <catch2/catch.hpp>

#include "../src/write_manager.hh"
#include "../src/storage_manager.hh"

#include <string>
#include <vector>
#include <iostream>

TEST_CASE( "testing write buffer", "[logic]" ) {

    const CB lCB(true, "", 300, 8080u);
    Trace::get_instance().init(lCB);

    using key_type = string_t;
    using value_type = string_t;
    using key_value_type = key_val_t<key_type,value_type>;
    auto& sm = StorageManager<key_type,value_type>::get_instance();
    sm.init(lCB);

    using str_key_val_t = key_val_pt<key_type, value_type>;

    const std::string TEST_PREFIX = "WB_";

    str_key_val_t kv1(TEST_PREFIX + "MyKey1", TEST_PREFIX + "MyValue1");
    str_key_val_t kv2(TEST_PREFIX + "MyKey2", TEST_PREFIX + "MyValue2");
    str_key_val_t kv3(TEST_PREFIX + "MyKey3", TEST_PREFIX + "MyValue3");
    str_key_val_t kv4(TEST_PREFIX + "MyKey4", TEST_PREFIX + "MyValue4");


    SECTION("test to disk and to memory tranformation"){
        std::unique_ptr<byte[]> page = std::make_unique<byte[]>(PAGE_SIZE);

        kv1.first.to_disk(page.get());
        kv1.second.to_disk(page.get() + kv1.first.size());


        key_type k;
        k.to_memory(page.get());

        value_type v;
        v.to_memory(page.get() + k.size());

        std::cout << "Recovered! Key: " << k << ", Value: " << v << std::endl;

        REQUIRE(kv1.first.data() == k.data());
        REQUIRE(kv1.second.data() == v.data());

    }

    SECTION("add data to buffer")
    {
        auto& wbuf = WriteManager<key_type, value_type>::get_instance();
        wbuf.init(lCB);
        std::vector<str_key_val_t> kv_vec = {kv1, kv2, kv3, kv4};

        size_t totalSize = 0;
        for(const auto& kv : kv_vec)
        {
            std::cout << "Key: " << kv.first << " (" << kv.first.bytes() << "B), Value: " << kv.second << " (" << kv.second.bytes() << "B)" << std::endl;
            totalSize += kv.first.bytes() + kv.second.bytes();
            wbuf.put(kv, MOD::kINSERT);
            try
            {
                REQUIRE(key_value_type(kv.first, kv.second, MOD::kINSERT) == wbuf.get(kv.first));
            }
            catch(KeyNotInWriteManagerException& ex)
            {
                REQUIRE(false);
                std::cerr << "Error: " << ex.what() << std::endl;
            }
        }
        std::cout << "Total Size: " << totalSize << std::endl;

        try
        {
            wbuf.get(key_type("NoKey"));
        }
        catch(KeyNotInWriteManagerException& ex)
        {
            REQUIRE(true);
            //std::cerr << "Error: " << ex.what() << std::endl;
        }



        try
        {
            auto kv_tmp = sm.get(kv2.first);
            REQUIRE(kv_tmp == kv2);
        }
        catch(KeyNotInStorageManagerException& ex)
        {
            std::cerr << "Error: " << ex.what() << std::endl;
            REQUIRE(false);
        }


        try
        {
            sm.get(key_type("NoKey"));
        }
        catch(KeyNotInStorageManagerException& ex)
        {
            REQUIRE(true);
            //std::cerr << "Error: " << ex.what() << std::endl;
        }

    }
}


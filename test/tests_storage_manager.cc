#include <catch2/catch.hpp>

#include "../src/storage_manager.hh"

#include <string>
#include <vector>
#include <iostream>

TEST_CASE( "testing storage manager", "[logic]" ) {

    const CB lCB(true, "", 300, 8080u);
    Trace::get_instance().init(lCB);

    using key_type = string_t;
    using value_type = string_t;

    using str_key_val_t = key_val_pt<key_type, value_type>;

    str_key_val_t kv1("MyKey1", "MyValue1");
    str_key_val_t kv2("MyKey2", "MyValue2");
    str_key_val_t kv3("MyKey3", "MyValue3");
    str_key_val_t kv4("MyKey4", "MyValue4");


    SECTION("test to disk and to memory tranformation for storage manager"){
        std::cout << "Create and initialize Storage Manager" << std::endl;
        auto& sm = StorageManager<key_type,value_type>::get_instance();
        sm.init(lCB);

        sm.partition().open();

        std::cout << "Alloc Page" << std::endl;
        uint32_t index = sm.partition().allocPage();
        std::cout << "page index: " << index << std::endl;

        std::unique_ptr<byte[]> page(std::make_unique<byte[]>(PAGE_SIZE));

        sm.partition().readPage(page.get(), index, PAGE_SIZE);

        InterpreterSP sp;
        sp.init_new_page(page.get(), index);

        REQUIRE(sp.header()->index() == index);
        REQUIRE(sp.header()->no_records() == 0u);

        {
            auto [ptr, slot] = sp.add_new_record(kv1.first.size());

            std::cout << "page address=" << static_cast<void*>(page.get()) << ", record address=" << static_cast<void*>(ptr) << ", slot=" << slot << std::endl;

            if(ptr)
            {
                kv1.first.to_disk(ptr);
            }
            else
            {
                std::cerr << "Error when adding record" << std::endl;
            
            }
        }

        {
            auto [ptr, slot] = sp.add_new_record(kv1.second.size());

            std::cout << "page address=" << static_cast<void*>(page.get()) << ", record address=" << static_cast<void*>(ptr) << ", slot=" << slot << std::endl;

            if(ptr)
            {
                kv1.second.to_disk(ptr);
            }
            else
            {
                std::cerr << "Error when adding record" << std::endl;
            
            }
        }

        sp.detach();
        sm.partition().writePage(page.get(), index, PAGE_SIZE);

        for(size_t i = 0; i < PAGE_SIZE; ++i)
        {
            page.get()[i] = static_cast<byte>(0u);
        }

        sm.partition().readPage(page.get(), index, PAGE_SIZE);
        sp.attach(page.get());

        REQUIRE(sp.header()->index() == index);
        REQUIRE(sp.header()->no_records() == 2u);

        byte* r1 = sp.get_record(0);

        key_type k_tmp;
        k_tmp.to_memory(r1);


        byte* r2 = sp.get_record(1);
        value_type v_tmp;
        v_tmp.to_memory(r2);

        std::cout << "Recovered Key: " << k_tmp << ", Value: " << v_tmp << std::endl;

        REQUIRE(k_tmp == kv1.first);
        REQUIRE(v_tmp == kv1.second);

        sm.partition().close();

    }
}


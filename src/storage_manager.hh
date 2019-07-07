#pragma once

#include "types.hh"
#include "trace.hh"
#include "exception.hh"

#include "partition_file.hh"
#include "interpreter_sp.hh"

#include <map>
#include <unordered_map>
#include <utility>
#include <functional>
#include <algorithm>
#include <shared_mutex>
#include <iostream>

template<typename K, typename V>
class StorageManager final
{
    public:
        using key_type = K;
        using value_type = V;
        using key_val_type = key_val_t<key_type, value_type>;

    private:
        StorageManager()                                                  noexcept;
        StorageManager(const StorageManager&)                             noexcept = delete;
        StorageManager& operator=(const StorageManager&)                  noexcept = delete;
        StorageManager(StorageManager&&)                                  noexcept = delete;
        StorageManager& operator=(StorageManager&&)                       noexcept = delete;

    public:
        ~StorageManager()                                                 noexcept;
        static StorageManager& get_instance()                             noexcept
        {
            static StorageManager lInstance;
            return lInstance;
        }
        void init(const CB& aCB)                                          noexcept;

    public:
        void write_to_disk(key_val_vt<K,V>& aKeyValueVec, sync_t& aSync)  noexcept;

    public:
        key_val_type    get(const key_type& aKey);
        PartitionFile&  partition()                                       noexcept { return m_partition; }


    private:
        auto&           mtx()                                       const noexcept { return m_mtx; }
        const CB&       cb()                                        const noexcept { return *m_cb; }
        const auto&     hasher()                                    const noexcept { return m_hasher; }
        auto&           disk_index()                                      noexcept { return m_index; }
        uint64_t        hash_v(const K& aKey)                       const noexcept { return hasher()(aKey);}

    private:
        mutable std::shared_mutex       m_mtx;
        const CB*                       m_cb;
        std::function<uint64_t(K)>      m_hasher;
        std::multimap<uint64_t,TID>     m_index;
        PartitionFile                   m_partition;

};

template<typename K, typename V>
StorageManager<K,V>::StorageManager() noexcept
    : m_mtx()
    , m_cb(nullptr)
    , m_hasher(std::hash<K>{})
    , m_index()
    , m_partition("./part.dat", "Key-Value-Persistency", 32u)
{
    TRACE("StorageManager constructed");
}

template<typename K, typename V>
StorageManager<K,V>::~StorageManager() noexcept = default;

template<typename K, typename V>
void StorageManager<K,V>::init(const CB& aCB) noexcept
{
    if(!m_cb)
    {
        TRACE("StorageManager initialized");
        m_cb = &aCB;
    }
}

template<typename K, typename V>
void StorageManager<K,V>::write_to_disk(key_val_vt<K,V>& aKeyValueVec, sync_t& aSync) noexcept
{
    TRACE("Flushing write managers data to disk...");

    std::lock_guard lock(mtx());
    std::unordered_map<K,key_val_t<K,V>> distinct_writes;

    TRACE("Move buffer elements to hash table in order to only write unique items to disk");
    for(auto& kv : aKeyValueVec)
    {
        distinct_writes.insert_or_assign(kv.key(), std::move(kv));
    }

    TRACE("Clear flush buffer, set flag, unlock and notify.");
    aKeyValueVec.clear();
    aSync.set();
    aSync.cv().notify_one();

    partition().open();
    TRACE("Allocating new page...");
    uint32_t index = partition().allocPage();
    TRACE("Successful");
    auto uptr = alloc_buffer_page();
    TRACE("Read newly allocated page to main memory...");
    partition().readPage(uptr.get(), index);
    TRACE("Successful");
    InterpreterSP sp;
    TRACE("Init newly allocated page with slotted page meta data...");
    sp.init_new_page(uptr.get(), index);
    TRACE("Successful");

    auto kv_iter = distinct_writes.cbegin();
    size_t kv_no = 1;
    while(kv_iter != distinct_writes.cend())
    {
        const auto& kv = kv_iter->second;
        TRACE("Processing record " + std::to_string(kv_no) + "/" + std::to_string(distinct_writes.size()) + ": '" + kv.to_string() + "'");
        //insert type
        if(kv.ins())
        {
            TRACE("Add '" + kv.to_string() + "' to slotted page");
            auto [rec_ptr, offset] = sp.add_new_record(kv.diskB());
            //if valid ptr -> record can be inserted
            if(rec_ptr)
            {
                const TID tid(index, offset);
                TRACE("New record: " + tid.to_string());
                const uint64_t hash = hash_v(kv.key());
                kv.to_disk(rec_ptr);

                disk_index().emplace(hash, tid);
                TRACE("Successful");

                assert(index == tid.page());
                assert(offset == tid.offset());
                assert(offset == sp.no_records() - 1);


            }
            //allocate a new page
            else
            {
                TRACE("Error: Page full.");
                TRACE("Write full page back and allocate a new empty page...");
                partition().writePage(uptr.get(), index);
                index = partition().allocPage();
                TRACE("Successful");
                TRACE("Read newly allocated page to main memory...");
                partition().readPage(uptr.get(), index);
                TRACE("Successful");
                sp.detach();
                TRACE("Init newly allocated page with slotted page meta data...");
                sp.init_new_page(uptr.get(), index);
                TRACE("Successful");
                TRACE("Retry insert...");
                continue;
            }
        }
        //delete type
        else if(kv.del())
        {
            TRACE("Delete '" + kv.to_string() + "'");
            auto range = disk_index().equal_range(hash_v(kv.key()));
            if(std::distance(range.first, range.second) != 0)
            {
                auto uptr_tmp = alloc_buffer_page();
                TRACE("Search for item in index...");
                for(auto it = range.first; it != range.second;)
                {
                    auto current = it;
                    ++it;
                    //get TID stored for this key in the index
                    TID tid = current->second;
                    TRACE("Node with same hash as searched item found. " + tid.to_string());
                    
                    TRACE("Read page in...");
                    //read page at TID position
                    partition().readPage(uptr_tmp.get(), tid.page());
                    TRACE("Successful");

                    InterpreterSP sp_tmp;
                    sp_tmp.attach(uptr_tmp.get());

                    TRACE("Get record...");
                    //get record on loaded page
                    byte* rec_ptr = sp_tmp.get_record(tid.offset());
                    if(rec_ptr)
                    {
                        TRACE("Successful");
                        TRACE("Transform disk record to memory representation...");
                        //transform disk representation to in-memory representation
                        key_val_type kv_tmp;
                        kv_tmp.to_memory(rec_ptr);
                        TRACE("Successful");

                        TRACE("Transformed: '" + kv_tmp.to_string() + "'");

                        TRACE("Check whether retrieved record equals the one to delete...");
                        if(kv_tmp.key() == kv.key())
                        {
                            TRACE("Retrieved record matches");
                            TRACE("Soft delete of record...");
                            sp_tmp.soft_delete(tid.offset());
                            TRACE("Write back page...");
                            partition().writePage(uptr_tmp.get(), tid.page());
                            TRACE("Remove key from index...");
                            disk_index().erase(current);
                            TRACE("Successful");
                        }
                        else
                        {
                            TRACE("Retrieved record does not match. Continue search...");
                        }
                    }
                    else
                    {
                        TRACE("ERROR: Could not retrieve record from page (record was deleted or is invalid)");
                    }
                }
            }
        }
        ++kv_iter;
        ++kv_no;
    }

    TRACE("Write page back to disk...");
    partition().writePage(uptr.get(), index);
    partition().close();

}

template<typename K, typename V>
typename StorageManager<K,V>::key_val_type StorageManager<K,V>::get(const key_type& aKey)
{
    std::shared_lock lock(mtx());
    TRACE("Search for item with key: '" + aKey.to_string() + "' in StorageManager");
    auto range = disk_index().equal_range(hash_v(aKey));
    int count = std::distance(range.first, range.second);
    if(count != 0)
    {
        partition().open();
        auto uptr = alloc_buffer_page();
        TRACE("Reversely iterate all found nodes with same hash as key");
        for(auto it = std::make_reverse_iterator(range.second); it != std::make_reverse_iterator(range.first); --it)
        {
            //get TID stored for this key in the index
            TID tid = it->second;
            TRACE("Item found: '" + tid.to_string() + "'");
            
            TRACE("Read page to main memory...");
            //read page at TID position
            partition().readPage(uptr.get(), tid.page());
            TRACE("Successful");

            InterpreterSP sp;
            sp.attach(uptr.get());

            TRACE("Retrieve record from page...");
            //get record on loaded page
            byte* rec_ptr = sp.get_record(tid.offset());
            if(rec_ptr)
            {
                TRACE("Successful");
                TRACE("Transform record from disk representation to in-memory item");
                //transform disk representation to in-memory representation
                key_val_type kv;
                kv.to_memory(rec_ptr);
                TRACE("Successful");

                TRACE("Transformed item: '" + kv.to_string() + "'");

                if(kv.key() == aKey)
                {
                    TRACE("Key found. Return item.");
                    partition().close();
                    return kv;
                }
                TRACE("Wrong Key, continue");
            }
            else
            {
                TRACE("ERROR: Could not retrieve record from page");
            }
        }
        partition().close();
        //error
    }
    TRACE("Key not found in storage manager");
    throw KeyNotInStorageManagerException(FLF);
}

#pragma once

#include "types.hh"
#include "exception.hh"
#include "storage_manager.hh"

#include <vector>
#include <utility>
#include <algorithm>
#include <shared_mutex>
#include <thread>
#include <iostream>

template<typename K, typename V>
class WriteManager final
{
    public:
        using key_type = K;
        using value_type = V;
        using key_val_type = key_val_t<key_type, value_type>;

    private:
        WriteManager()                                                                    noexcept;
        WriteManager(const WriteManager&)                                                 noexcept = delete;
        WriteManager& operator=(const WriteManager&)                                      noexcept = delete;
        WriteManager(WriteManager&&)                                                      noexcept = delete;
        WriteManager& operator=(WriteManager&&)                                           noexcept = delete;

    public:
        ~WriteManager()                                                                   noexcept;
        static WriteManager& get_instance()                                               noexcept
        {
            static WriteManager lInstance;
            return lInstance;
        }
        void init(const CB& aCB)                                                          noexcept;

    public:
        key_val_type    get(const key_type& aKey);
        void            put(const key_val_pt<K,V>& aKeyValue, MOD aModType)               noexcept;
        void            put(const key_type& aKey, const value_type& aVal, MOD aModType)   noexcept;
        void            del(const key_type& aKey, const value_type& aVal)                 noexcept;
        void            flush()                                                           noexcept;

    private:
        void            flush_no_lock()                                                   noexcept;
        auto&           input_mtx()                                                 const noexcept { return m_input_mtx; }
        auto&           flush_mtx()                                                       noexcept { return m_flush_mtx; }
        sync_t&         sync()                                                            noexcept { return m_sync; }
        const CB&       cb()                                                        const noexcept { return *m_cb; }
        size_t&         get_buf_size()                                                    noexcept { return m_buffer_size; }
        auto&           get_ibuf()                                                        noexcept { return m_input_buffer; }
        auto&           get_fbuf()                                                        noexcept { return m_flush_buffer; }

    private:
        mutable std::shared_mutex m_input_mtx;
        std::mutex      m_flush_mtx;
        sync_t          m_sync;
        const CB*       m_cb;
        size_t          m_buffer_size;
        key_val_vt<K,V> m_input_buffer;
        key_val_vt<K,V> m_flush_buffer;

};

template<typename K, typename V>
WriteManager<K,V>::WriteManager() noexcept
    : m_input_mtx()
    , m_flush_mtx()
    , m_sync()
    , m_cb(nullptr)
    , m_buffer_size(0)
    , m_input_buffer()
    , m_flush_buffer()
{
    TRACE("WriteManager constructed");
}

template<typename K, typename V>
WriteManager<K,V>::~WriteManager() noexcept = default;

template<typename K, typename V>
void WriteManager<K,V>::init(const CB& aCB) noexcept
{
    if(!m_cb)
    {
        TRACE("WriteManager initialized");
        m_cb = &aCB;
    }
}

template<typename K, typename V>
typename WriteManager<K,V>::key_val_type WriteManager<K,V>::get(const key_type& aKey)
{
    std::shared_lock lock(input_mtx());
    TRACE("Search for key: '" + aKey.to_string() + "' in WriteManager");
    for(auto it = get_ibuf().rbegin(); it != get_ibuf().rend(); ++it) 
    {
        //TRACE("Current: '" + it->to_string() + "'");
        if(it->key() == aKey)
        {
            TRACE("Current has same key as searched one.");
            if(it->ins())
            {
                TRACE("Found Valid Key.");
                return *it;
            }
            else if(it->del())
            {
                throw KeyIsDeletedInWriteManagerException(FLF);
            }
            else if(!it->valid())
            {
                TRACE("Found Invalid Key.");
                break;
            }
        }
    }
    TRACE("Key not found in WriteManager");
    throw KeyNotInWriteManagerException(FLF);
}
        
template<typename K, typename V>
void WriteManager<K,V>::put(const key_val_pt<K,V>& aKeyValue, MOD aModType) noexcept
{
    put(aKeyValue.first, aKeyValue.second, aModType);
}

template<typename K, typename V>
void WriteManager<K,V>::put(const key_type& aKey, const value_type& aVal, MOD aModType) noexcept
{
    key_val_type data(aKey, aVal, aModType);
    std::lock_guard lock(input_mtx());
    TRACE("Add KV-pair to the input buffer: '" + data.to_string() + "'");
    TRACE("Curr buffer size=" + std::to_string(get_buf_size()) + ", KV-pair size=" + std::to_string(data.bytes()) + " @@ Allowed size=" + std::to_string(cb().buffer_size()));
    if(get_buf_size() + data.bytes()  >= cb().buffer_size())
    {
        //need to write to disk before inserting to buffer
        TRACE("Input buffer full. Need to flush data to disk.");
        flush_no_lock();
        TRACE("Flusher thread started working. Continue adding KV-pair...");
    }
    get_buf_size() += data.bytes();
    get_ibuf().emplace_back(std::move(data));
    TRACE("Add successful");
}

template<typename K, typename V>
void WriteManager<K,V>::del(const key_type& aKey, const value_type& aVal) noexcept
{
    put(aKey, aVal, MOD::kDELETE);
}

template<typename K, typename V>
void WriteManager<K,V>::flush() noexcept
{
    std::lock_guard lock(input_mtx());
    flush_no_lock();
}

template<typename K, typename V>
void WriteManager<K,V>::flush_no_lock() noexcept
{
    TRACE("Acquire flush mutex...");
    std::unique_lock flush_lock(flush_mtx());
    if(!sync()())
    {
        TRACE("Need to wait until flush copy finishes...");
        sync().cv().wait(flush_lock, [this](){ return sync()(); });
        TRACE("Notified: Continue flushing...");
    }
    sync().clear();
    TRACE("Swap flush buffer with input buffer and set buffer size back to 0");
    std::swap(get_ibuf(), get_fbuf());
    get_buf_size() = 0;

    TRACE("Spawn flusher thread to write flush buffer to disk...");
    std::thread(&StorageManager<K,V>::write_to_disk, &StorageManager<K,V>::get_instance(), std::ref(get_fbuf()), std::ref(sync())).detach();
}

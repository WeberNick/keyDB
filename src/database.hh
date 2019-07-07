#pragma once

#include "types.hh"
#include "exception.hh"
#include "trace.hh"
#include "write_manager.hh"
#include "storage_manager.hh"

template<typename K, typename V>
class KeyValueStore final
{
    public:
        using key_type = K;
        using value_type = V;
        using key_val_type = key_val_t<key_type,value_type>;

    private:
        KeyValueStore()                                                   noexcept;
        KeyValueStore(const KeyValueStore&)                               noexcept = delete;
        KeyValueStore& operator=(const KeyValueStore&)                    noexcept = delete;
        KeyValueStore(KeyValueStore&&)                                    noexcept = delete;
        KeyValueStore& operator=(KeyValueStore&&)                         noexcept = delete;

    public:
        ~KeyValueStore()                                                  noexcept;
        static KeyValueStore& get_instance()                              noexcept
        {
            static KeyValueStore lInstance;
            return lInstance;
        }
        void init(const CB& aCB)                                          noexcept;

    public:
        answer_t        request_handler(const string_vt& args)            noexcept;

    public:
        key_val_type    get(const key_type& aKey);
        void            put(const key_type& aKey, const value_type& aVal) noexcept;
        void            del(const key_type& aKey)                         noexcept;
        void            flush()                                           noexcept;


    private:
        const CB&       cb()                                        const noexcept { return *m_cb; }
        auto&           get_write_mngr()                                  noexcept { return m_write_mngr; }
        auto&           get_storage_mngr()                                noexcept { return m_storage_mngr; }

    private:
        const CB*               m_cb;
        WriteManager<K,V>&      m_write_mngr;
        StorageManager<K,V>&    m_storage_mngr;

};

template<typename K, typename V>
KeyValueStore<K,V>::KeyValueStore() noexcept
    : m_cb(nullptr)
    , m_write_mngr(WriteManager<K,V>::get_instance())
    , m_storage_mngr(StorageManager<K,V>::get_instance())
{

}

template<typename K, typename V>
KeyValueStore<K,V>::~KeyValueStore() noexcept = default;

template<typename K, typename V>
void KeyValueStore<K,V>::init(const CB& aCB) noexcept
{
    if(!m_cb)
    {
        m_cb = &aCB;
        get_write_mngr().init(aCB);
        get_storage_mngr().init(aCB);
    }
}

template<typename K, typename V>
answer_t KeyValueStore<K,V>::request_handler(const string_vt& args) noexcept
{
    if(args.at(0) == "GET")
    {
        try
        {
            auto kv = get(key_type(args.at(1)));
            return std::make_pair("OK", kv.to_string_f());
        }
        catch(KeyNotInStorageManagerException& ex)
        {
            return std::make_pair("ERROR", ex.what());
        }
        catch(KeyIsDeletedInWriteManagerException& ex)
        {
            return std::make_pair("ERROR", ex.what());
        }
    }
    else if(args.at(0) == "PUT")
    {
        put(key_type(args.at(1)), value_type(args.at(2)));
        return std::make_pair("OK", "Successful Insert");
    }
    else if(args.at(0) == "DEL")
    {
        del(key_type(args.at(1)));
        return std::make_pair("OK", "Successful Delete");
    }
    else if(args.at(0) == "FLUSH")
    {
        flush();
        return std::make_pair("OK", "Successful Flush");
    }
    else
    {
        return std::make_pair("ERROR", "INVALID REQUEST");
    }
}

template<typename K, typename V>
typename KeyValueStore<K,V>::key_val_type KeyValueStore<K,V>::get(const key_type& aKey)
{
    try
    {
        return get_write_mngr().get(aKey);
    }
    catch(KeyNotInWriteManagerException& ex)
    {
        return get_storage_mngr().get(aKey);
    }
}
        
template<typename K, typename V>
void KeyValueStore<K,V>::put(const key_type& aKey, const value_type& aVal) noexcept
{
    get_write_mngr().put(aKey, aVal, MOD::kINSERT);
}

template<typename K, typename V>
void KeyValueStore<K,V>::del(const key_type& aKey) noexcept
{
    get_write_mngr().put(aKey, V(), MOD::kDELETE);
}

template<typename K, typename V>
void KeyValueStore<K,V>::flush() noexcept
{
    get_write_mngr().flush();
}

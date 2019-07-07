#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>
#include <cassert>
#include <limits>
#include <type_traits>
#include <memory>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <atomic>

using int8_t = std::int8_t;
using uint8_t = std::uint8_t;
using int16_t = std::int16_t;
using uint16_t = std::uint16_t;
using int32_t = std::int32_t;
using uint32_t = std::uint32_t;
using int64_t = std::int64_t;
using uint64_t = std::uint64_t;
using size_t = std::size_t;
using byte = std::byte;
using uint = unsigned int;
using string_vt = std::vector<std::string>;

constexpr uint16_t PAGE_SIZE = 16384;

inline std::unique_ptr<byte[]> alloc_buffer_page() noexcept
{
    return std::make_unique<byte[]>(PAGE_SIZE);
}

std::string to_string(bool aBool) noexcept;

class control_block_t final
{
    public:
        control_block_t()                                     noexcept = delete;
        control_block_t(const control_block_t&)               noexcept = delete;
        control_block_t(control_block_t&&)                    noexcept = delete;
        control_block_t& operator=(const control_block_t&)    noexcept = delete;
        control_block_t& operator=(control_block_t&&)         noexcept = delete;
        control_block_t(
                bool aTrace, 
                const std::string& aTracePath,
                uint aBufferSize,
                uint aPort)                                   noexcept;
        ~control_block_t()                                    noexcept;

    public:
        bool                trace()                     const noexcept;
        const std::string&  trace_path()                const noexcept;
        uint                buffer_size()               const noexcept;
        uint                port()                      const noexcept;
        std::ostream&       print(std::ostream& os)     const noexcept;

    public:
        friend std::ostream& operator<<(std::ostream& os, const control_block_t& aCB) noexcept
        {
            return aCB.print(os);
        }

    private:
        bool                m_trace;
        std::string         m_trace_path;
        uint                m_buffer_size;
        uint                m_port;
};
using CB = control_block_t;

class string_t final
{
    public:
        string_t()                                                noexcept;
        string_t(const std::string& aData)                        noexcept;
        string_t(const string_t&);
        string_t& operator=(const string_t&);
        string_t(string_t&&)                                      noexcept;
        string_t& operator=(string_t&&)                           noexcept;
        ~string_t()                                               noexcept;

    public:
        bool                operator==(const string_t& rhs) const noexcept;
        bool                operator!=(const string_t& rhs) const noexcept;
        bool                operator<(const string_t& rhs)  const noexcept;
        bool                operator<=(const string_t& rhs) const noexcept;
        bool                operator>(const string_t& rhs)  const noexcept;
        bool                operator>=(const string_t& rhs) const noexcept;

    public:
        const std::string&  data()                          const noexcept;
        size_t              bytes()                         const noexcept ;
        size_t              size()                          const noexcept;
        byte*               to_disk(byte* aMem)             const noexcept;
        byte*               to_disk(byte* aMem)                   noexcept;
        byte*               to_memory(byte* aMem)                 noexcept;
        std::string         to_string()                     const noexcept;
        friend std::ostream& operator<<(std::ostream& os, const string_t& t) noexcept
        {
            return os << t.to_string();
        }

    private:
        std::string m_data;
};

using str_key = string_t;
using str_val = string_t;


namespace std
{
    template<> struct hash<string_t>
    {
        size_t operator()(const string_t& s) const noexcept
        {
            return std::hash<std::string>{}(s.data());
        }
    };
}

template<typename K, typename V>
using key_val_pt = std::pair<K,V>;

enum class MOD : int8_t
{
    kINVALID = -1,
    kINSERT = 0,
    kUPDATE = 1,
    kDELETE = 2,
    kNoTypes = 3
};

inline std::string to_string_mod(MOD aMod) noexcept
{
    std::string result;
    switch(aMod)
    {
        case MOD::kINVALID: result = "INVALID"; break;
        case MOD::kINSERT: result = "INSERT"; break;
        case MOD::kUPDATE: result = "UPDATE"; break;
        case MOD::kDELETE: result = "DELETE"; break;
        default: result = "DEFAULT/ERROR";
    }
    return result;
}

template<typename K, typename V>
class key_val_t final
{
    public:
        key_val_t()                             noexcept : m_key_val(), m_mod_type(MOD::kINVALID){};
        key_val_t(const K& aKey, const V& aVal, MOD aModType = MOD::kINVALID) noexcept
            : m_key_val(aKey, aVal), m_mod_type(aModType)
        {}
        key_val_t(const key_val_t&)            = default;
        key_val_t& operator=(const key_val_t&) = default;
        key_val_t(key_val_t&&)                 noexcept = default;
        key_val_t& operator=(key_val_t&&)      noexcept = default;
        ~key_val_t()                           noexcept = default;

    public:
        bool operator==(const key_val_t& rhs) const noexcept { return key() == rhs.key() && val() == rhs.val(); }
        bool operator==(const key_val_pt<K,V>& rhs) const noexcept { return key() == rhs.first && val() == rhs.second; }
        bool operator!=(const key_val_t& rhs) const noexcept { return !operator==(rhs); }
        bool operator!=(const key_val_pt<K,V>& rhs) const noexcept { return !operator==(rhs); }

    public:
        auto&       key_val()     noexcept { return m_key_val; }
        const K&    key()   const noexcept { return m_key_val.first; }
        const V&    val()   const noexcept { return m_key_val.second; }
        MOD         type()  const noexcept { return m_mod_type; }
        bool        ins()   const noexcept { return type() == MOD::kINSERT; }
        bool        del()   const noexcept { return type() == MOD::kDELETE; }
        bool        valid() const noexcept { return !del() && type() != MOD::kINVALID;}
        size_t      bytes() const noexcept { return key().bytes() + val().bytes() + sizeof(m_mod_type); }
        size_t      diskB() const noexcept { return key().bytes() + val().bytes(); }

    public:
        void        to_disk(byte* aMem) const noexcept
        {
            byte* lMem = key().to_disk(aMem);
            assert((aMem + key().size()) == lMem);
            lMem = val().to_disk(lMem);
            assert((aMem + key().size() + val().size()) == lMem);
        }
        void        to_disk(byte* aMem) noexcept
        {
            static_cast<const key_val_t&>(*this).to_disk(aMem);
        }
        void        to_memory(byte* aMem) noexcept
        {
            byte* lMem = key_val().first.to_memory(aMem);
            lMem = key_val().second.to_memory(lMem);
        }
        std::string to_string() const noexcept { return key().to_string() + " @ " + val().to_string() + " @ " + to_string_mod(type()); }
        std::string to_string_f() const noexcept { return "Key: '" + key().to_string() + "', Value: '" + val().to_string() + "'"; }
        friend std::ostream& operator<<(std::ostream& os, const key_val_t& t) noexcept
        {
            return os << t.to_string();
        }

    public:
        key_val_pt<K,V>     m_key_val;
        MOD                 m_mod_type;
};

template<typename K, typename V>
using key_val_vt = std::vector<key_val_t<K,V>>;

class TID final
{
    public:
        TID(uint16_t aPageNo, uint16_t aOffsetNo) noexcept : m_pNo(aPageNo), m_oNo(aOffsetNo){}

    public:
        uint16_t page() const noexcept { return m_pNo; }
        uint16_t offset() const noexcept { return m_oNo; }
        std::string to_string() const noexcept { return "TID: page=" + std::to_string(static_cast<uint32_t>(page())) + ", offset=" + std::to_string(static_cast<uint32_t>(offset())); }

    private:
        uint16_t m_pNo;
        uint16_t m_oNo;
};

template<typename T>
constexpr T MAX_VALUE = std::numeric_limits<T>::max();

template<typename T>
constexpr T invalid_v() noexcept
{
    return MAX_VALUE<T>;
}

template<typename E>
constexpr auto underlying_type(E enumerator) noexcept
{
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

inline bool valid_request(const string_vt& args) noexcept
{
    bool valid = false;
    if(args.at(0) == "GET" || args.at(0) == "DEL")
    {
        valid = args.size() >= 2;
    }
    else if(args.at(0) == "PUT")
    {
        valid = args.size() >= 3;
    }
    else if(args.at(0) == "FLUSH")
    {
        valid = true;
    }
    return valid;
}

using answer_t = std::pair<std::string, std::string>;

class sync_t final
{
    public:
        sync_t()                            noexcept = default;
        sync_t(const sync_t&)               noexcept = delete;
        sync_t& operator==(const sync_t&)   noexcept = delete;
        sync_t(sync_t&&)                    noexcept = delete;
        sync_t& operator==(sync_t&&)        noexcept = delete;
        ~sync_t()                           noexcept = default;

    public:
        auto& cv()                noexcept { return m_cv; }
        void  set()               noexcept { return m_flag.store(true); }
        void  clear()             noexcept { return m_flag.store(false); }
        bool  operator()()  const noexcept { return m_flag.load(); }

    private:
        std::condition_variable m_cv{};
        std::atomic_bool        m_flag{true};
};


#include "types.hh"
#include <iostream>
#include <sstream>

std::string to_string(bool aBool) noexcept
{
    return aBool ? "true" : "false";
}

control_block_t::control_block_t(bool aTrace, const std::string& aTracePath, uint aBufferSize, uint aPort) noexcept
    : m_trace(aTrace)
    , m_trace_path(aTracePath)
    , m_buffer_size(aBufferSize)
    , m_port(aPort)
{
    std::cout << *this << std::endl;
}

control_block_t::~control_block_t() noexcept = default;

bool control_block_t::trace() const noexcept
{  
    return m_trace;
}

const std::string& control_block_t::trace_path() const noexcept
{  
    return m_trace_path;
}

uint control_block_t::buffer_size() const noexcept
{
    return m_buffer_size;
}

uint control_block_t::port() const noexcept
{
    return m_port;
}

std::ostream& control_block_t::print(std::ostream& os) const noexcept
{
    os << "Control Block Settings:\n"
        << "\t* Trace: \t'" << to_string(trace()) << "'"
        << "\n\t* Trace Path: \t'" << trace_path() << "'"
        << "\n\t* Page Size: \t'" << PAGE_SIZE << "'"
        << "\n\t* Buffer Size: \t'" << buffer_size() << "'"
        << "\n\t* Port: \t'" << port() << "'"
        << std::endl;
    return os;
}

string_t::string_t() noexcept
    : m_data()
{}

string_t::string_t(const std::string& aData) noexcept 
    : m_data(aData) 
{ 
    m_data.shrink_to_fit(); 
}

string_t::string_t(const string_t&) = default;
string_t& string_t::operator=(const string_t&) = default;
string_t::string_t(string_t&&) noexcept = default;
string_t& string_t::operator=(string_t&&) noexcept = default;
string_t::~string_t() noexcept= default;

bool string_t::operator==(const string_t& rhs) const noexcept
{
    return data() == rhs.data();
}

bool string_t::operator!=(const string_t& rhs) const noexcept
{
    return !operator==(rhs);
}

bool string_t::operator<(const string_t& rhs)  const noexcept
{
    return data() < rhs.data();
}

bool string_t::operator<=(const string_t& rhs) const noexcept
{
    return operator==(rhs) || operator<(rhs);
}

bool string_t::operator>(const string_t& rhs)  const noexcept
{
    return data() > rhs.data();
}

bool string_t::operator>=(const string_t& rhs) const noexcept
{
    return operator==(rhs) || operator>(rhs);
}

const std::string& string_t::data() const noexcept
{
    return m_data;
}

size_t string_t::bytes() const noexcept 
{ 
    return sizeof(m_data) + m_data.capacity(); 
}

size_t string_t::size() const noexcept
{
    return data().size() + 1;
}

byte* string_t::to_disk(byte* aMem) const noexcept
{
    char* char_ptr = reinterpret_cast<char*>(aMem);
    size_t index = m_data.copy(char_ptr, m_data.size());
    char_ptr[index++] = '\0';
    return aMem + index;
}

byte* string_t::to_disk(byte* aMem) noexcept
{
    return static_cast<const string_t&>(*this).to_disk(aMem);
}

byte* string_t::to_memory(byte* aMem) noexcept
{
    m_data = std::string(reinterpret_cast<char*>(aMem));
    size_t index = m_data.size() + 1;
    return aMem + index;
}

std::string string_t::to_string() const noexcept
{
    std::ostringstream strm;
    strm << data();
    return strm.str();
}

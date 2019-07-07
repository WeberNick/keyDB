#pragma once
#include "types.hh"
#include "exception.hh"
#include "trace.hh"

#include <string>
#include <utility>

class InterpreterSP final
{
    public:
        /* A header for a slotted page */
        struct sp_header_t final
        {
            uint16_t m_page_index; // Page index inside the partition
            uint16_t m_no_records;     // number of records stored on this page
            uint16_t m_free_space;     // total number of free bytes
            uint16_t m_next_free_space; // pointer to first free space on page

            uint16_t&   index()         noexcept { return m_page_index; }
            uint16_t&   no_records()    noexcept { return m_no_records; }
            uint16_t&   free_space()    noexcept { return m_free_space; }
            uint16_t&   next()          noexcept { return m_next_free_space; }

            std::string to_string()     noexcept;


        };

        struct slot_t final
        {
            uint16_t m_offset; // offset to record

            uint16_t&   offset()    noexcept { return m_offset;}
            bool        valid()     noexcept { return offset() != invalid_v<uint16_t>(); }
        };

    public:
        InterpreterSP()                                                     noexcept;
        InterpreterSP(const InterpreterSP&)                                 noexcept = delete;
        InterpreterSP(InterpreterSP&&)                                      noexcept = delete;
        InterpreterSP& operator=(const InterpreterSP&)                      noexcept = delete;
        InterpreterSP& operator=(InterpreterSP&&)                           noexcept = delete;
        ~InterpreterSP()                                                    noexcept;

    public:
        inline void                 attach(byte* aPP)                       noexcept;
        void                        detach()                                noexcept;

    public:
        void                        init_new_page(byte* aPP, uint aPageNo)  noexcept;
        /**
         * @brief return ptr where to insert record and its offset in the slots
         * @param aRecordSize the record size
         * @return std::pair<byte*, uint16_t> the location where to write the new record
         */
        std::pair<byte*, uint16_t>  add_new_record(uint aRecordSize)        noexcept;
        // just mark as deleted
        void                        soft_delete(uint16_t aRecordNo)         noexcept;
        //gets a record, returns a nullptr if it does not exist or is marked invalid
        byte*                       get_record(uint aRecordNo)              noexcept;

    public:
        inline byte*                page_ptr()                              noexcept;
        inline sp_header_t*         header()                                noexcept;
        inline uint                 free_space()                            noexcept;
        inline uint                 no_records()                            noexcept;
        inline slot_t&              slot(uint i)                            noexcept;

    private:
        inline sp_header_t*         get_hdr_ptr()                           noexcept;
        inline slot_t*   	        get_slot_base_ptr()                     noexcept;

    private:
        byte*        m_page_ptr;
        sp_header_t* m_header;
        slot_t*      m_slots;
    };

void InterpreterSP::attach(byte* aPP) noexcept
{
    m_page_ptr = aPP;
    m_header = get_hdr_ptr();
    m_slots  = get_slot_base_ptr();
}

byte* InterpreterSP::page_ptr() noexcept 
{ 
    return m_page_ptr; 
}
InterpreterSP::sp_header_t* InterpreterSP::header() noexcept 
{ 
    return m_header; 
}

uint InterpreterSP::free_space() noexcept 
{ 
    return header()->free_space(); 
}

uint InterpreterSP::no_records() noexcept 
{ 
    return header()->no_records(); 
}

InterpreterSP::slot_t& InterpreterSP::slot(uint i) noexcept 
{ 
    return m_slots[- static_cast<int>(i)]; 
}

InterpreterSP::sp_header_t* InterpreterSP::get_hdr_ptr() noexcept 
{ 
    return reinterpret_cast<sp_header_t*>(page_ptr() + PAGE_SIZE - sizeof(sp_header_t)); 
}

InterpreterSP::slot_t* InterpreterSP::get_slot_base_ptr() noexcept 
{ 
    return reinterpret_cast<slot_t*>(page_ptr() + PAGE_SIZE - sizeof(sp_header_t) - sizeof(slot_t)); 
}

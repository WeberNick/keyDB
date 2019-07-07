/**
 *  @file	 interpreter_fsip.hh
 *  @brief	 A class implementing a Free Space Indicator Page (FSIP) interpreter for little Endian
 *  @bugs	 Might not work for big Endian
 *  @todos	 -
 *  @section  DESCRIPTION
 *  This class implements a free space indicator page (FSIP) that is used to see which pages are free (value of 0) to use or occupied (value of 1).
 *  The correct interpretation of the pages status is only working for little endian.
 */

#pragma once

#include "types.hh"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

class InterpreterFSIP final
{
    private:
        friend class PartitionBase;

    public:
        /* A header for the free space indicator page */
        struct fsip_header_t final
        {
            uint32_t m_free_blocks_count; // Number of free pages in the managed part (numer of 0s)
            uint32_t m_next_free_page;    // index of the next 0 (indicating a free Block)
            uint32_t m_managed_pages;    // how many pages managed by this fsip?
            uint16_t m_page_index; // Page index inside the partition
            uint16_t m_unused;

            uint32_t& free_blocks()         noexcept { return m_free_blocks_count; }
            uint32_t& next_free_page()      noexcept { return m_next_free_page; }
            uint32_t& no_managed_pages()    noexcept { return m_managed_pages; }
            uint16_t& index()               noexcept { return m_page_index; }
            uint16_t& unused()              noexcept { return m_unused; }
        };
    
    public:
        InterpreterFSIP()                                   noexcept;
        InterpreterFSIP& operator=(const InterpreterFSIP&)  noexcept = delete;
        InterpreterFSIP(const InterpreterFSIP&)             noexcept = delete;
        InterpreterFSIP(InterpreterFSIP&&)                  noexcept = delete;
        InterpreterFSIP& operator=(InterpreterFSIP&&)       noexcept = delete;
        ~InterpreterFSIP()                                  noexcept;


    public:
        inline void attach(byte *aPP)                       noexcept;
        void detach()                                       noexcept;

    public:
        /**
         *	@brief	initialize the FSIP through setting all bits to 0 and the header
         *	@param	aPP - Pointer to the start of the page
         *	@param	aOffset - Page index inside the partition
         *	@param	aNoBlocks - Number of stored Pages in FSIP
         */
        void        init_new_FSIP(byte *aPP, uint16_t aPageIndex, uint32_t aNoBlocks)   noexcept;

        /**
         *	@brief	looks for the next free block in the FSIP and reserves the page
         *	@param	aPP - Pointer to the start of the page
         * 	@return an offset to the free block or an INVALID value if no free pages are present
         */
        uint32_t    get_new_page(byte *aPP)                                             noexcept;
    
        /**
         *	@brief	free the page at the given index position
         *	@param	aPageIndex - Page index inside the partition
         */
        void        free_page(uint aPageIndex)                                          noexcept;
    
        /**
         * @brief   if the partition can grow, this method will mark accordingly many pages as free
         *
         * @param   aNumberOfPages - number of pages which shall be freed in general
         * @param   aMaxPagesPerFSIP - Is calculated by the partition and therefore handed over.
         * @return  the number of pages which have to be freed on the next fsip (which is to be created by the partition)
         */
        uint32_t    grow(uint aNumberOfPages, uint aMaxPagesPerFSIP)                    noexcept;

        /**
         *	@brief	reserve the page at the given index position
         *	@param	aPageIndex - Page index inside the partition
         */
        void        reserve_page(uint aPageIndex)                                       noexcept;
    

    public:
        inline byte*            page_ptr()                                              noexcept;
        inline fsip_header_t*   header()                                                noexcept;
        inline uint             no_managed_pages()                                      noexcept;
        constexpr static uint   header_size()                                           noexcept;

    private:
        inline fsip_header_t*   get_hdr_ptr()                                           noexcept;
        /**
         *	@brief	get the Position of the next free page
         *
         * 	@return either the position if successfull or 0
         */
        uint                    next_free_page()                                        noexcept;
    
    private:
        byte*            m_page_ptr;       // pointer to beginning of the page
        fsip_header_t*   m_header;   // FSIP Header
};

void InterpreterFSIP::attach(byte* aPP) noexcept
{
    m_page_ptr = aPP;
    m_header = get_hdr_ptr();
}

byte* InterpreterFSIP::page_ptr() noexcept 
{ 
    return m_page_ptr; 
}

InterpreterFSIP::fsip_header_t* InterpreterFSIP::header() noexcept 
{ 
    return m_header; 
}

uint InterpreterFSIP::no_managed_pages() noexcept 
{ 
    return header()->no_managed_pages(); 
}

constexpr uint InterpreterFSIP::header_size() noexcept 
{ 
    return sizeof(fsip_header_t); 
}

InterpreterFSIP::fsip_header_t* InterpreterFSIP::get_hdr_ptr() noexcept 
{ 
    return reinterpret_cast<fsip_header_t*>(page_ptr() + PAGE_SIZE - header_size()); 
}

#include "interpreter_sp.hh"
#include <sstream>

std::string InterpreterSP::sp_header_t::to_string() noexcept
{
    std::ostringstream strm;
    strm << "@@ Slotted Page Header @@ Page Index=" << index() << ", #records=" << no_records() << ", free space=" << free_space() << ", next free=" << next();
    return strm.str();
}

InterpreterSP::InterpreterSP() noexcept
    : m_page_ptr(nullptr)
    , m_header(nullptr)
    , m_slots(nullptr) 
{}

InterpreterSP::~InterpreterSP() noexcept = default;

void InterpreterSP::detach() noexcept
{
    m_page_ptr = nullptr;
	m_header = nullptr;
	m_slots  = nullptr;
}

void InterpreterSP::init_new_page(byte* aPP, uint aPageNo) noexcept 
{
	if(aPP)
	{
		attach(aPP);
        header()->index() = aPageNo;
		header()->no_records() = 0;
		header()->free_space() = (PAGE_SIZE - sizeof(sp_header_t));
		header()->next() = 0;
        TRACE(header()->to_string());
	}
}

std::pair<byte*, uint16_t> InterpreterSP::add_new_record(uint aRecordSize) noexcept
{
    TRACE("Slotted Page: Add new record (size=" + std::to_string(aRecordSize) + ")");
	const uint lRecordSize = ((aRecordSize + 7) & ~static_cast<uint>(0x07)); // adjust for 8 byte alignment
	const uint lTotalSize = lRecordSize + sizeof(slot_t);        // add space for one new slot 

    byte* lResultRecord = nullptr;

	if(lTotalSize <= free_space()) 
	{
        TRACE("Enough free space for record...");
		lResultRecord = page_ptr() + header()->next();
		// how much space is there?
		header()->next() += lRecordSize;               // remember pointer to next free record
		header()->free_space() -= lTotalSize;
		slot(no_records()).offset() = lResultRecord - page_ptr(); // store offset of new record in slot
        // set remaining slot
        ++(header()->no_records());
	}
	return std::make_pair(lResultRecord, header()->no_records() - 1);
}

// just mark deleted
void InterpreterSP::soft_delete (uint16_t aRecordNo) noexcept
{
	slot(aRecordNo).offset() = invalid_v<uint16_t>();
}

byte* InterpreterSP::get_record(uint aRecordNo) noexcept
{
    TRACE("Get record " + std::to_string(aRecordNo));
	if(aRecordNo >= no_records())
	{ 
        TRACE("Invalid record no");
		return nullptr;
	}
	else
	{
		if(slot(aRecordNo).valid()){
            TRACE("Record is valid");
			return page_ptr() + slot(aRecordNo).offset();
		}
		else{
            TRACE("Record is invalid");
			return nullptr;
		}
	}
}


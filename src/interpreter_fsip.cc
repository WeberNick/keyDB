#include "interpreter_fsip.hh"

#include "bit_intrinsics.hh"
#include "trace.hh"

InterpreterFSIP::InterpreterFSIP() noexcept :
    m_page_ptr(nullptr),
    m_header(nullptr)
{}

InterpreterFSIP::~InterpreterFSIP() noexcept = default;

void InterpreterFSIP::detach() noexcept
{
    m_page_ptr = nullptr;
    m_header = nullptr;
}

void InterpreterFSIP::init_new_FSIP(byte *aPP, uint16_t aPageIndex, uint32_t aNoBlocks) noexcept 
{
    attach(aPP);
    uint32_t max = aNoBlocks / 32; // how far the page is free
    uint32_t i = 0;
    while (i < max)
    {
        *(reinterpret_cast<uint32_t*>(aPP) + i) = 0; // set to 0
        ++i;
    }
    // only the first bits to 0, remaining to 1
    max = (PAGE_SIZE - header_size()) / 4; //new limit
    uint32_t lMask = 0;
    if (i < max)
    {
        lMask = ~lMask;
        lMask = lMask << (aNoBlocks % 32);
        *(reinterpret_cast<uint32_t*>(aPP) + i) = lMask;
        ++i;
    }

    lMask = 0;
    lMask = ~lMask; // lMask only 1s
    while (i < max)
    {
        *(reinterpret_cast<uint32_t*>(aPP) + i) = lMask;
        ++i;
    }
    // set header
    uint32_t lNextFreePage = 0;
    uint16_t lUnused = 0;
    fsip_header_t temp = {aNoBlocks, lNextFreePage, aNoBlocks, aPageIndex, lUnused};
    *header() = temp;
}

uint32_t InterpreterFSIP::get_new_page(byte *aPP) noexcept
{
    if (header()->free_blocks() == 0)
    {
        TRACE("No free pages on this FSIP");
        return invalid_v<uint32_t>();
    }
    attach(aPP);
    uint32_t lPosFreeBlock = header()->next_free_page();
    byte *lPP = aPP;
    lPP += lPosFreeBlock / 8; // set pointer lPosfreeBlocks/8 bytes forward
    uint8_t lMask = 1;
    uint8_t lPartBits = *reinterpret_cast<uint8_t*>(lPP);       // get 8 bit Int representation of the lPP byte pointer
    lPartBits |= (lMask << lPosFreeBlock % 8); // set complement bit at lPosFreeBlock in lPartBits
    *reinterpret_cast<uint8_t*>(lPP) = lPartBits;

    header()->next_free_page() = next_free_page();
    --(header()->free_blocks());
    return lPosFreeBlock + 1 + header()->index();
}

void InterpreterFSIP::free_page(uint aPageIndex) noexcept
{
    uint lPageIndex = aPageIndex;
    lPageIndex -= header()->index() + 1u;
    if (header()->next_free_page() > lPageIndex)
    {
        header()->next_free_page() = lPageIndex;
    }
    uint32_t *lPP = reinterpret_cast<uint32_t*>(page_ptr());
    lPP += (lPageIndex / 32);
    uint32_t lBitindex = (lPageIndex % 32);
    uint32_t lMask = 1;
    lMask <<= lBitindex;
    *lPP = *lPP & (~lMask);
    ++(header()->free_blocks());
}

uint32_t InterpreterFSIP::grow(uint aNumberOfPages, uint aMaxPagesPerFSIP) noexcept
{
    //TRACE("Updating FSIP's with new partition size starts...");

    // how many pages fit on page
    uint freeOnThisPage = aMaxPagesPerFSIP - header()->no_managed_pages();

    if(freeOnThisPage == 0)
    {
        return aNumberOfPages;
    }
    // distance of first page to be freed to last one.
    int64_t ldist = (static_cast<int64_t>(freeOnThisPage)) - (static_cast<int64_t>(aNumberOfPages));
    byte* lPP = page_ptr();
    uint8_t lMask = 0;
    uint remainingPages; // to be set on this FSIP
    size_t start;
    if(ldist >= 0) // if Pages to grow fit on this page
    {
        remainingPages = aNumberOfPages;
    }
    else
    {
        // free rest of page by setting remainingPages to rest of bits.
        remainingPages = aMaxPagesPerFSIP - header()->no_managed_pages();
    }
    // free from no_managed_pages() remainnigPages many
    header()->free_blocks() += remainingPages; // mark how many new free pages there will be.
    // first byte aligned or not
    if(header()->no_managed_pages() % 8 != 0)
    {
        // changed to shift right, negate result
        lMask = (~lMask) << (header()->no_managed_pages() % 8);
        *(reinterpret_cast<uint8_t*>(lPP) + (header()->no_managed_pages()) / 8) = ~lMask;
        remainingPages -= 8 - (header()->no_managed_pages() % 8);
        start = header()->no_managed_pages() / 8 + 1;
    }
    else
    {
        start = header()->no_managed_pages() / 8;
    }
    // free all aligned bytes
    size_t i = 0;
    size_t max = remainingPages / 8;
    while( i < max)
    {
        *(reinterpret_cast<uint8_t*>(lPP) + i + start) = 0;
        remainingPages -= 8;
        ++i;
    }
    // if there are some left
    if(remainingPages !=0 )
    {
        lMask = 0;
        // changed to shift left
        lMask = (~lMask) << remainingPages;
       *(reinterpret_cast<uint8_t*>(lPP) + i + start) &= lMask;
    }
    // next free page is position up to which pages were managed till now.
   header()->next_free_page() = header()->no_managed_pages();

        // switch the return value
    if(ldist >=0) 
    {
        header()->no_managed_pages() += aNumberOfPages;
        // debug(header()->index());
        return 0;
    }
    else
    {
        // free rest of page by setting remainingPages to rest of bits.
        header()->no_managed_pages() = aMaxPagesPerFSIP;
        return static_cast<uint32_t>((-1) * ldist); // contains pages to be managed by next fsip
    }
}

void InterpreterFSIP::reserve_page(uint aPageIndex) noexcept
{
    uint lPageIndex = aPageIndex;
    lPageIndex -= header()->index() + 1u;
    uint32_t* lPP = reinterpret_cast<uint32_t*>(page_ptr());
    lPP += (lPageIndex / 32);
    uint32_t lBitindex = (lPageIndex % 32);
    uint32_t lMask = 1;
    lMask <<= lBitindex;
    // check if free
    uint32_t test = *lPP;
    test &= lMask;
    *lPP = *lPP | (lMask);
    --(header()->free_blocks());
    header()->next_free_page() = next_free_page();
}

uint InterpreterFSIP::next_free_page() noexcept
{
    size_t lCondition = ((PAGE_SIZE - header_size()) / 4) - 1;
    for (uint32_t j = (header()->next_free_page()) / 32; j <= lCondition; ++j)
    {   // looping through FSIP with step 8
        uint32_t* lPP = reinterpret_cast<uint32_t*>(page_ptr()) + j;
        uint32_t lPartBytes = *lPP; // cast to 8 Byte Int Pointer, add the next j 8Byte block and dereference
        lPartBytes = ~lPartBytes;
        if ((lPartBytes) != 0)
        {
            uint32_t lCalcFreePos = idx_lowest_bit_set<uint32_t>(lPartBytes); // find the first "leftmost" zero
            return ((j * 32) + lCalcFreePos);
            // change LSN
            break;
        }
    }
    return 0;
}

//void InterpreterFSIP::debug(const uint aPageIndex)
//{
    //TRACE("debug");
    //std::ofstream myfile;
    //std::string filename = "page" + std::to_string(aPageIndex) + ".txt";
    //myfile.open(filename);
    //uint32_t *lPP2 = (uint32_t *)page_ptr();
    //for (uint a = 0; a < PAGE_SIZE / 4; ++a)
    //{
        //myfile << std::hex << std::setw(8) << std::setfill('0') << *(lPP2 + a) << std::endl;
    //}
    //myfile.close();
//}


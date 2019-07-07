#include "partition_base.hh"
#include "exception.hh"
#include "trace.hh"
#include "interpreter_fsip.hh"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

PartitionBase::PartitionBase(const std::string& aPath, const std::string& aName) noexcept : 
	_partitionPath(aPath),
	_partitionName(aName),
	_pageSize(PAGE_SIZE),
	_sizeInPages(0),
	_openCount(0),
	_fileDescriptor(-1)
{
}

PartitionBase::~PartitionBase() noexcept = default;

void PartitionBase::open()
{
	if(_openCount == 0)
	{
		_fileDescriptor = ::open(_partitionPath.c_str(), O_RDWR); // call open in global namespace
		if(_fileDescriptor == -1)
		{
            const std::string lErrMsg = std::string("An error occured while opening the file: '") + std::string(std::strerror(errno));
            TRACE(lErrMsg);
            throw FileException(FLF, _partitionPath.c_str(), lErrMsg);
		}
	}
	++_openCount;
}

void PartitionBase::close()
{
	if(_openCount == 1)
	{
		if(::close(_fileDescriptor) == -1) // call close in global namespace
		{
            const std::string lErrMsg = std::string("An error occured while closing the file: '") + std::string(std::strerror(errno));
            TRACE(lErrMsg);
            throw FileException(FLF, _partitionPath.c_str(), lErrMsg);
		}
		--_openCount;
		_fileDescriptor = -1;
	}
	else if(_openCount > 1)
	{
		--_openCount;
	}
}

uint32_t PartitionBase::allocPage()
{
	byte* lPagePointer = new byte[_pageSize];
	InterpreterFSIP fsip;
	fsip.attach(lPagePointer);
	uint lIndexOfFSIP = 0;
	uint32_t lAllocatedPageIndex;
	do
	{
		readPage(lPagePointer, lIndexOfFSIP, _pageSize); // Read FSIP into buffer
        
		lAllocatedPageIndex = fsip.get_new_page(lPagePointer);	// Request free block from FSIP
        if(lAllocatedPageIndex == invalid_v<uint32_t>())
        {
			uint lIndexOfNextFSIP = lIndexOfFSIP + (1 + getMaxPagesPerFSIP()); // Prepare next offset to FSIP
		    if(lIndexOfNextFSIP >= _sizeInPages) // Next offset is bigger than the partition
            {
                const std::string lErrMsg("The partition is full. Can not allocate any new pages on fsip: " + std::to_string(lIndexOfFSIP));
                TRACE(lErrMsg);
				close();
                // if file partition: can recover by growing file
                throw PartitionFullException(FLF, lPagePointer, lIndexOfFSIP); 
            }
			else{
				lIndexOfFSIP = lIndexOfNextFSIP;
			}
            continue;
        }
		writePage(lPagePointer, lIndexOfFSIP, _pageSize);
        break;
        // continue will jump here
	}
	while(true); // if a free page is found, break will be executed. If not, an exception is thrown
	delete[] lPagePointer;
	TRACE(std::string("Page ") + std::to_string(lAllocatedPageIndex) + std::string(" allocated."));
	return lAllocatedPageIndex;	// return offset to free block
}

void PartitionBase::freePage(const uint32_t aPageIndex)
{
    std::unique_ptr<byte[]> lPagePointer = std::make_unique<byte[]>(_pageSize);

	uint32_t fsipIndex = (aPageIndex / (getMaxPagesPerFSIP()+1))*getMaxPagesPerFSIP();
	readPage(lPagePointer.get(), fsipIndex , _pageSize); // fsip auf der aPageIndex verwaltet wird
	InterpreterFSIP fsip;
	fsip.attach(lPagePointer.get());
	fsip.free_page(aPageIndex);
	fsip.detach();
	writePage(lPagePointer.get(), fsipIndex,_pageSize);
}

void PartitionBase::readPage(byte* aBuffer, const uint32_t aPageIndex, const uint aBufferSize)
{
    assert(aBufferSize == PAGE_SIZE && aBufferSize == _pageSize);
	if(pread(_fileDescriptor, aBuffer, aBufferSize, (aPageIndex * _pageSize)) == -1)
	{
        const std::string lErrMsg = std::string("An error occured while reading the file: '") + std::string(std::strerror(errno));
        TRACE(lErrMsg);
        throw FileException(FLF, _partitionPath.c_str(), lErrMsg);
	}
}

void PartitionBase::writePage(const byte* aBuffer, const uint32_t aPageIndex, const uint aBufferSize)
{
    assert(aBufferSize == PAGE_SIZE && aBufferSize == _pageSize);
	if(pwrite(_fileDescriptor, aBuffer, aBufferSize, (aPageIndex * _pageSize)) == -1)
	{
        const std::string lErrMsg = std::string("An error occured while writing the file: '") + std::string(std::strerror(errno));
        TRACE(lErrMsg);
        throw FileException(FLF, _partitionPath.c_str(), lErrMsg);
	}
}

void PartitionBase::format()
{
    std::unique_ptr<byte[]> lPagePointer = std::make_unique<byte[]>(_pageSize);
	const uint lPagesPerFSIP = getMaxPagesPerFSIP();
	uint lCurrentPageNo = 0;
	InterpreterFSIP fsip;
	uint remainingPages = _sizeInPages;
	uint lNumberOfPagesToManage;
    open();
	while(remainingPages > 1)
	{
		--remainingPages;
		lNumberOfPagesToManage = ((remainingPages > lPagesPerFSIP) ? lPagesPerFSIP : remainingPages);
		fsip.init_new_FSIP(lPagePointer.get(), lCurrentPageNo, lNumberOfPagesToManage);
		writePage(lPagePointer.get(), lCurrentPageNo, _pageSize);
		lCurrentPageNo += (lPagesPerFSIP + 1);
		remainingPages -= lNumberOfPagesToManage;
	}
	fsip.detach();
	readPage(lPagePointer.get(), 0u, _pageSize);
	fsip.attach(lPagePointer.get());
	writePage(lPagePointer.get(), 0u, _pageSize);
	close();
}

uint PartitionBase::getMaxPagesPerFSIP() noexcept
{
	return (_pageSize - InterpreterFSIP::header_size()) * 8;
}

std::ostream& operator<< (std::ostream& stream, const PartitionBase& aPartition)
{
    stream << aPartition.to_string();
    return stream;
}



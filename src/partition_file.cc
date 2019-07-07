#include "partition_file.hh"
#include "exception.hh"
#include "trace.hh"
#include "interpreter_fsip.hh"

#include <cmath>
#include <cstdlib>

PartitionFile::PartitionFile(const std::string& aPath, const std::string& aName, const uint16_t aGrowthIndicator) noexcept :
	PartitionBase(aPath, aName),
	_growthIndicator(aGrowthIndicator)
{
    if(_growthIndicator < 8)
    {
        TRACE("A growth indicator smaller than 8 was provided. As the system needs a growth factor of at least 8, it is set accordingly");
        _growthIndicator = 8;
    }
    create();
    TRACE("'PartitionFile' object constructed (For a new partition)");
    TRACE("Initial partition size in pages: " + std::to_string(partSizeInPages()));
}

PartitionFile::~PartitionFile() noexcept
{
    close();
    remove();
    TRACE("'PartitionFile' object destructed");
}

uint32_t PartitionFile::allocPage()
{
    uint32_t lPageIndex = 0;
    try
    {
        lPageIndex = PartitionBase::allocPage();
    }
    catch(const PartitionFullException& ex)
    {
        // Before the exception is thrown, the partition (file) will be closed by the alloc call in partition base...
        // Open file again for recovering from the exception
        open();
        // extend
        TRACE("Extending the file partition. Grow by " + std::to_string(static_cast<uint32_t>(getGrowthIndicator())) + " pages (currently " + std::to_string(_sizeInPages) + " pages)");
        const size_t lNewSize = (_sizeInPages + _growthIndicator) * _pageSize;
        FileUtil::resize(_partitionPath, lNewSize);
        _sizeInPages = lNewSize / _pageSize;
        TRACE("Extending the file partition was successful. New size is " + std::to_string(_sizeInPages) + " pages");
        // extend finished
        // grow fsip
        InterpreterFSIP lFSIP;
        lFSIP.attach(ex.getBufferPtr());
        const size_t lPagesPerFSIP = getMaxPagesPerFSIP();
        const uint lRemainingPages = lFSIP.grow(_growthIndicator, lPagesPerFSIP);
        writePage(ex.getBufferPtr(),ex.getIndexOfFSIP(),_pageSize);
        if(lRemainingPages > 0)
        {
            const uint lNextFSIP = ex.getIndexOfFSIP() + lPagesPerFSIP + 1;
            const uint lNumberOfPagesToManage = ((lRemainingPages > lPagesPerFSIP) ? lPagesPerFSIP : lRemainingPages);
            lFSIP.init_new_FSIP(ex.getBufferPtr(), lNextFSIP, lNumberOfPagesToManage);
		    writePage(ex.getBufferPtr(), lNextFSIP, _pageSize);
        }
        // in the partition_base part of alloc, a buffer was allocated but not deleted before throwing the exception..
        delete[] ex.getBufferPtr();
        TRACE("FSIP's were successfully updated with the new partition size");
        lPageIndex = PartitionBase::allocPage();
    }
    return lPageIndex;
}

size_t PartitionFile::partSize() noexcept
{
    return FileUtil::isFile(_partitionPath) ? FileUtil::fileSize(_partitionPath) : 0;
}

size_t PartitionFile::partSizeInPages() noexcept
{
    return (partSize() / _pageSize);
}

void PartitionFile::create()
{   
	if(exists())
	{
        TRACE("Partition already exists and cannot be created");
        throw PartitionExistsException(FLF);
    }
    TRACE("Creating a file at '" + _partitionPath + "'");
    FileUtil::create(_partitionPath);
    if(exists())
    {
        TRACE("File created at '" + _partitionPath + "'");
        const size_t lFileSize = _growthIndicator * _pageSize;
        FileUtil::resize(_partitionPath, lFileSize);
        _sizeInPages = partSizeInPages(); 
        TRACE("File partition (with " + std::to_string(_sizeInPages) + " pages) was successfully created in the file system");
        format(); // may throw
    }
    else
    {
        const std::string lMsg = "Something went wrong while trying to create the file";
        TRACE(lMsg); 
        throw PartitionException(FLF, lMsg);
    }
}

void PartitionFile::remove()
{
    std::string lTraceMsg;
    if(!exists())
    {
        lTraceMsg = std::string("No file exists at '") + _partitionPath + std::string("'");
        TRACE(lTraceMsg);
        return;
    }
    lTraceMsg = std::string("Trying to remove file partition at '") + _partitionPath + std::string("'");
    TRACE(lTraceMsg);
    if(FileUtil::remove(_partitionPath))
    {
        lTraceMsg = "File partition was successfully removed from the file system";
        TRACE(lTraceMsg); 
    }
    else
    {
        lTraceMsg = "Something went wrong while trying to remove the file from the file system";
        TRACE(lTraceMsg); 
        throw PartitionException(FLF, lTraceMsg);
    }
}

std::ostream& operator<< (std::ostream& stream, const PartitionFile& aPartition)
{
    stream << aPartition.to_string();
    return stream;
}

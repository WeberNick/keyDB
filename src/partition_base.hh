/**
 *  @file 	partition_base.hh
 *  @author	Nick Weber (nickwebe@pi3.informatik.uni-mannheim.de)
 *  @brief	An abstract class implementing the interface for every partition
 *  @bugs	Currently no bugs known
 *  @todos	Remove new/delete in allocPage and replace with unique ptr
 *  @section TBD
 */

#pragma once

#include "types.hh"
#include "file_util.hh"

#ifdef __linux__
    #include <linux/fs.h>
    #define P_NO_BLOCKS BLKGETSIZE
  #define P_BLOCK_SIZE BLKSSZGET
#elif __APPLE__
    #include <sys/disk.h>
    #define P_NO_BLOCKS DKIOCGETBLOCKCOUNT 
    #define P_BLOCK_SIZE DKIOCGETBLOCKSIZE 
#else
    // unsupported
    #define P_NO_BLOCKS 0 
    #define P_BLOCK_SIZE 0 
    #error SYSTEM IS NOT COMPATIBLE WITH NON-UNIX OPERATING SYSTEMS
#endif

#include <iostream>
#include <string>

class PartitionBase 
{
    protected:
        PartitionBase()                                                     noexcept = delete;
        PartitionBase(const std::string& aPath, const std::string& aName)   noexcept;
        PartitionBase(const PartitionBase&)                                 noexcept = delete;
        PartitionBase& operator=(const PartitionBase&)                      noexcept = delete;
        PartitionBase(PartitionBase&&)                                      noexcept = delete;
        PartitionBase& operator=(PartitionBase&&)                           noexcept = delete;

    public:
        virtual ~PartitionBase()                                            noexcept;

    public:
        /**
         *  @brief  Opens the partition in read/write mode. If the partition is already open, an open counter
         *          will be increased
         *  @throws FileException on failure
         *  @see    infra/exception.hh
         */
        void                open();
    
        /**
         *  @brief  Closes the partition. If the open count is greater than 1, it is decreased. Otherwise the
         *          partition will be closed and the file descriptor is set to -1
         *
         *  @throws FileException on failure
         *  @see    infra/exception.hh
         */
        void                close();
    
        /**
         *  @brief  Allocates a new page in the partition
         *  @return an index to the allocated page
         *
         *  @throws
         *  @see    interpreter/interpreter_fsip.hh, infra/exception.hh
         *  @note   doesn't use the buffer manager yet but retrieves its fsips directly
         */
        virtual uint32_t    allocPage();
    
        /**
         *  @brief  Physically remove a page by setting its bit in the fsip
         *
         *  @param  aPageIndex: an index indicating which page to remove
         *  @see    interpeter/interpreter_fsip.hh
         */
        void                freePage(uint32_t aPageIndex);
    
        /**
         *  @brief  Read a page from the partition into a main memory buffer
         *
         *  @param  aBuffer: the buffer to read the page into
         *  @param  aPageIndex: an index indicating which page to read
         *  @param  aBufferSize: size of the buffer in bytes
         *  @throws FileException on failure
         *  @see    infra/exception.hh
         */
        void                readPage(byte* aBuffer, uint32_t aPageIndex, uint aBufferSize = PAGE_SIZE);
    
        /**
         *  @brief  Write a page from a main memory buffer on the partition
         *
         *  @param  aBuffer: where to write from
         *  @param  aPageIndex: an index indicating which page to write
         *  @param  aBufferSize: size of the buffer in bytes
         *  @throws FileException on Failure
         *  @see    infra/exception.hh
         */
        void                writePage(const byte* aBuffer, uint32_t aPageIndex, uint aBufferSize = PAGE_SIZE);

    public:
        // Getter
        inline const std::string&   getPath()           const noexcept { return _partitionPath; }
        inline const std::string&   getPath()                 noexcept { return _partitionPath; }
        inline const std::string&   getName()           const noexcept { return _partitionName; }
        inline const std::string&   getName()                 noexcept { return _partitionName; }
        inline uint                 getPageSize()       const noexcept { return _pageSize; }
        inline uint                 getPageSize()             noexcept { return _pageSize; }
        inline uint                 getSizeInPages()    const noexcept { return _sizeInPages; }
        inline uint                 getSizeInPages()          noexcept { return _sizeInPages; }
  
        inline std::string          to_string()         const noexcept;
        inline std::string          to_string()               noexcept;

    protected:
        /**
         *  @brief  Format the partition by initializing its fsip
         *
         */
        void            format();
    
        /**
         *  @brief  Physically create the partition. Needs to be implemented by respective partition type
         *
         *  @see    partition_file.hh, partition_raw.hh
         */
        virtual void    create() = 0;
    
        /**
         *  @brief  Physically removes the partition. Needs to be implemented by respective partition type
         *
         *  @see    partition_file.hh, partition_raw.hh
         */
        virtual void    remove() = 0;

    protected:
        // Wrapper call. Checks if a file exists at the partition path
        inline bool     exists()                noexcept { return FileUtil::exists(_partitionPath); }
        // Wrapper call. Checks if the partition path is a regular file
        inline bool     isFile()                noexcept { return FileUtil::isFile(_partitionPath); }
        // Wrapper call. Checks if the partition path is a raw device
        inline bool     isRawDevice()           noexcept { return FileUtil::isRawDevice(_partitionPath); }
        virtual size_t  partSize() = 0;
        virtual size_t  partSizeInPages() = 0;
        uint            getMaxPagesPerFSIP()    noexcept;

    protected:
        std::string _partitionPath; // A path to a partition (i.e., a file)
        std::string _partitionName; // Name of the partition
        uint _pageSize;             // The page size in bytes, used by the partition
        uint _sizeInPages;          // The current size of the partition in number of pages
        uint _openCount;            // Counts the number of open calls
        int _fileDescriptor;        // The partitions file descriptor
};

std::string PartitionBase::to_string() const noexcept
{
    return std::string("Path : '")
      + getPath() + "', Name: '"
      + getName();
}

std::string PartitionBase::to_string() noexcept 
{ 
    return static_cast<const PartitionBase&>(*this).to_string(); 
}

std::ostream& operator<< (std::ostream& stream, const PartitionBase& aPartition);

/**
 *  @file    partition_file.hh
 *  @author  Nick Weber (nickwebe@pi3.informatik.uni-mannheim.de)
 *  @brief   A class implementing the interface of a partition stored in a file
 *  @bugs    Currently no bugs known
 *  @section TODO
 */

#pragma once

#include "partition_base.hh"

#include <string>

class PartitionFile : public PartitionBase
{
    public:
        PartitionFile()                                 noexcept = delete;
        PartitionFile(const std::string& aPath, const std::string& aName, const uint16_t aGrowthIndicator)  noexcept;
        PartitionFile(const PartitionFile&)             noexcept = delete;
        PartitionFile& operator=(const PartitionFile&)  noexcept = delete;
        PartitionFile(PartitionFile&&)                  noexcept = delete;
        PartitionFile& operator=(PartitionFile&&)       noexcept = delete;
        ~PartitionFile()                                noexcept;
    
    public:
        /**
         *  @brief  Wrapper for call to allocPage in PartitonBase (this handles specific behaviour)
         *  @return an index to the allocated page
         *  @see    partition_base.hh
         */
        uint32_t            allocPage() override;
        /**
        * @brief Retrieves the size of the file
        */
        size_t              partSize() noexcept override;
        /** TODO
         * @brief 
         * 
         * @return size_t 
         */
        size_t              partSizeInPages() noexcept override;

    public:
        // Getter
        inline uint16_t     getGrowthIndicator()    const noexcept { return _growthIndicator; }
        inline uint16_t     getGrowthIndicator()          noexcept { return _growthIndicator; }
        
        inline std::string  to_string()             const noexcept;
        inline std::string  to_string()                   noexcept;

    private:
        void                create() override;
        void                remove() override;

    private: 
        uint16_t _growthIndicator; // An indicator how the partition will grow (indicator * block size)
};


std::string PartitionFile::to_string() const noexcept
{
    return PartitionBase::to_string() 
        + std::string(", Growth : ")
        + std::to_string(static_cast<uint32_t>(getGrowthIndicator()));
}

std::string  PartitionFile::to_string() noexcept 
{ 
    return static_cast<const PartitionFile&>(*this).to_string(); 
}

std::ostream& operator<< (std::ostream& stream, const PartitionFile& aPartition);

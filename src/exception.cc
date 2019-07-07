#include "exception.hh"
#include <iostream>
#include <cerrno>
#include <cstring>

BaseException::BaseException(
        const char*         aFileName,
        const unsigned int  aLineNumber,
        const char*         aFunctionName,
        const char*         aErrorMessage) :
    std::runtime_error(aErrorMessage),
	_file(aFileName),
	_line(aLineNumber),
	_func(aFunctionName)
	// _errMsg(aErrorMessage)
{}

BaseException::BaseException(
        const std::string&  aFileName,
        const unsigned int  aLineNumber,
        const std::string&  aFunctionName,
        const std::string&  aErrorMessage) :
    std::runtime_error(aErrorMessage),
	_file(aFileName),
	_line(aLineNumber),
	_func(aFunctionName)
{}

BaseException::BaseException(const BaseException& aException) :
    std::runtime_error(aException.what()),
	_file(aException._file),
	_line(aException._line),
	_func(aException._func)
{}

void BaseException::print() const
{
    std::cerr << "An error occured in " << _file
        << ", line " << _line
        << ", '" << _func
        << "':\n\t\"" << what() << "\""
        << std::endl;
}

KeyNotInWriteManagerException::KeyNotInWriteManagerException(
        const char*         aFileName,
        const unsigned int  aLineNumber,
        const char*         aFunctionName) :
	BaseException(
            aFileName,
            aLineNumber,
            aFunctionName,
            "Requested key was not found in the write manager.")
{}

KeyIsDeletedInWriteManagerException::KeyIsDeletedInWriteManagerException(
        const char*         aFileName,
        const unsigned int  aLineNumber,
        const char*         aFunctionName) :
	BaseException(
            aFileName,
            aLineNumber,
            aFunctionName,
            "Requested key is marked as deleted in write manager")
{}

KeyNotInStorageManagerException::KeyNotInStorageManagerException(
        const char*         aFileName,
        const unsigned int  aLineNumber,
        const char*         aFunctionName) :
	BaseException(
            aFileName,
            aLineNumber,
            aFunctionName,
            "Requested key was not found in the storage manager.")
{}

FileException::FileException(
        const char*         aFileName,
        const unsigned int  aLineNumber,
        const char*         aFunctionName,
        const char*         aErrorFileName,
        const std::string&  aErrorMessage) :
	BaseException(
            aFileName,
            aLineNumber,
            aFunctionName,
            std::string("Problem with file ") + std::string(aErrorFileName) + std::string(" : ") + aErrorMessage + ".")
{}

PartitionException::PartitionException(
        const char*         aFileName,
        const unsigned int  aLineNumber,
        const char*         aFunctionName,
        const std::string&  aErrorMessage) :
	BaseException(
            aFileName,
            aLineNumber,
            aFunctionName,
            aErrorMessage)
{}

PartitionFullException::PartitionFullException(
        const char*         aFileName,
        const unsigned int  aLineNumber,
        const char*         aFunctionName,
        byte*               aBufferPointer,
        const uint          aIndexOfFSIP) :
	BaseException(
            aFileName,
            aLineNumber,
            aFunctionName,
            "The partition is full. Can not allocate any new pages."),
    _bufPtr(aBufferPointer), 
    _index(aIndexOfFSIP)
{}

PartitionFullException::PartitionFullException(const PartitionFullException& aOther) : 
    BaseException(aOther),
    _bufPtr(aOther.getBufferPtr()),
    _index(aOther.getIndexOfFSIP())
{}

PartitionExistsException::PartitionExistsException(
        const char*         aFileName,
        const unsigned int  aLineNumber,
        const char*         aFunctionName) :
	BaseException(
            aFileName,
            aLineNumber,
            aFunctionName,
            std::string("Cannot create existing partition or remove non existing partition."))
{}

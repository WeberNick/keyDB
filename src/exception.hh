/**
 *	@file 	exception.hh
 *	@author	Nick Weber (nickwebe@pi3.informatik.uni-mannheim.de)
 *	@brief	Provides a central class for exception handling
 *	@bugs 	Currently no bugs known
 *	@todos  -	
 *
 *	@section DESCRIPTION
 *	    TODO
 *  @section USE
 *	    throw BaseException(__FILE__, __LINE__, __PRETTY_FUNCTION__, "Error Message");
 */

#pragma once

#include "types.hh"

#include <exception>
#include <string>

#define FLF __FILE__, __LINE__, __PRETTY_FUNCTION__
#define ASSERT_MSG(MSG) assert(!MSG);  

class BaseException : public std::runtime_error 
{
    public:
        BaseException(
            const char*         aFileName, 
            const unsigned int  aLineNumber, 
            const char*         aFunctionName, 
            const char*         aErrorMessage); 
        BaseException(
            const std::string&  aFileName, 
            const unsigned int  aLineNumber, 
            const std::string&  aFunctionName, 
            const std::string&  aErrorMessage); 
        BaseException(const BaseException& aException);
        BaseException(BaseException&&) = delete;
        BaseException& operator=(const BaseException&) = delete;
        BaseException& operator=(BaseException&&) = delete;
        virtual ~BaseException() = default;

    public:
        // virtual const char* what() const; // inherited from std::runtime_error
        void print() const;

    private:
        const std::string   _file;
        const unsigned int  _line;
        const std::string   _func;
};

class KeyNotInWriteManagerException : public BaseException 
{
    public:
        KeyNotInWriteManagerException(
            const char*         aFileName,
            const unsigned int  aLineNumber,
            const char*         aFunctionName);
};

class KeyIsDeletedInWriteManagerException : public BaseException 
{
    public:
        KeyIsDeletedInWriteManagerException(
            const char*         aFileName,
            const unsigned int  aLineNumber,
            const char*         aFunctionName);
};

class KeyNotInStorageManagerException : public BaseException 
{
    public:
        KeyNotInStorageManagerException(
            const char*         aFileName,
            const unsigned int  aLineNumber,
            const char*         aFunctionName);
};

class FileException : public BaseException
{
	public:
		FileException(
            const char*         aFileName, 
            const unsigned int  aLineNumber, 
            const char*         aFunctionName, 
            const char*         aErrorFileName, 
            const std::string&  aErrorMessage); 
};

class PartitionException : public BaseException
{
    public:
        PartitionException(
            const char*         aFileName,
            const unsigned int  aLineNumber,
            const char*         aFunctionName,
            const std::string&  aErrorMessage);
};

class PartitionFullException : public BaseException
{
    public:
        PartitionFullException(
            const char*         aFileName,
            const unsigned int  aLineNumber,
            const char*         aFunctionName,
            byte*               aBufferPointer,
            const uint          aIndexOfFSIP);
        PartitionFullException(const PartitionFullException& aOther);
        PartitionFullException& operator=(const PartitionFullException&) = delete;

    public:
        inline byte* getBufferPtr() const { return _bufPtr; }
        inline uint  getIndexOfFSIP() const { return _index; }

    private:
        byte*   _bufPtr;
        uint    _index;
};

class PartitionExistsException : public BaseException
{
    public:
        PartitionExistsException(
            const char*         aFileName,
            const unsigned int  aLineNumber,
            const char*         aFunctionName);
};


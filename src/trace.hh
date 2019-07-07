/**
 *  @author Nick Weber
 *  @brief  Implements tracing functionality
 *  @bugs   -
 *
 *  @section DESCRIPTION
 *      TODO
 */

#pragma once

#include "types.hh"
#include <string>
#include <fstream>

#define TRACE(msg) Trace::get_instance().log(__FILE__, __LINE__, __PRETTY_FUNCTION__, msg)

class Trace final
{
    private:
        Trace()                                         noexcept;
        Trace(const Trace&)                             noexcept = delete;
        Trace(Trace&&)                                  noexcept = delete;
        Trace& operator=(const Trace&)                  noexcept = delete;
        Trace& operator=(Trace&&)                       noexcept = delete;
        ~Trace()                                        noexcept;

    public:
        static Trace& get_instance()                    noexcept
        {
            static Trace lInstance;
            return lInstance;
        }

        void    init(const CB& aControlBlock)           noexcept;

    public:
        void    log(const char* aFileName, uint aLineNumber, const char* aFunctionName, const std::string& aMessage) noexcept;

    public:
        inline const std::string&       getLogPath()    noexcept { return _logPath; }
        inline const std::ofstream&     getLogStream()  noexcept { return _logStream; }

    private:
        std::string   _logPath;
        std::ofstream _logStream;
        const CB*     _cb;
};


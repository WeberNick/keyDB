#include "trace.hh"

#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

Trace::Trace() noexcept :
    _logPath(),
    _logStream(),
    _cb(nullptr)
{}

Trace::~Trace() noexcept
{
    if(_cb->trace())
    {
        TRACE("Closing the log file...");
        TRACE("'Trace' destructed");
        _logStream.close();
    }
}

void Trace::init(const CB& aControlBlock) noexcept
{
    if(!_cb)
    {
        _cb = &aControlBlock;
        _logPath = _cb->trace_path();
        _logPath.append("logs/");
        fs::create_directory(_logPath);
        std::time_t lCurrTime = std::time(nullptr);
        std::string lTime = std::string(std::ctime(&lCurrTime));
        _logPath.append(lTime.substr(0, lTime.size() - 1).append(".log"));
        if(_cb->trace())
        {
            _logStream.open(_logPath.c_str(), std::ofstream::out | std::ofstream::app);
            TRACE("'Trace' constructed"); // just for consistency with the other singletons
            TRACE("Log file created and opened");
        }
        TRACE("'Trace' initialized");
    }
}

void Trace::log(const char* aFileName, uint aLineNumber, const char* aFunctionName, const std::string& aMessage) noexcept
{
    if(_cb->trace())
    {
        std::time_t lCurrTime = std::time(nullptr);
        std::string lTime = std::ctime(&lCurrTime);
        lTime = lTime.substr(0, lTime.size() - 1);
        _logStream << lTime 
            << ": " << aFileName 
            << ", line " << aLineNumber
            << ", " << aFunctionName
            << ":\n'" << aMessage << "'"
            << std::endl;
    }
}


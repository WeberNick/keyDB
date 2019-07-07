/**
 *	@file 	args.hh
 *	@author	Nick Weber (nickwebe@pi3.informatik.uni-mannheim.de)
 *	@brief	Implementation of command line arguments parser
 *	@bugs 	Currently no bugs known
 *	@todos	-
 *
 *	@section DESCRIPTION
 *	This class implements the command line arguements. A command line arguement has the form:
 * 	--[command] [optional parameter]
 * 	Where '--' indicates a command will follow,
 *	'command' is the command name
 *	'optional parameter' is an optional parameter only needed for certain commands 
 */

#pragma once 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include "argbase.hh"
#pragma GCC diagnostic pop
#include "types.hh"
#include <string>

class Args
{
    public:
        Args()                                                        noexcept;
        Args(const Args&)                                             noexcept = delete;
        Args& operator=(const Args&)                                  noexcept = delete;
        Args(Args&&)                                                  noexcept = delete;
        Args& operator=(Args&&)                                       noexcept = delete;
        ~Args()                                                       noexcept;

    public:
        bool                help()                              const noexcept;
        void                help(const bool& x)                       noexcept;
        
        bool                trace()                             const noexcept;
        void                trace(const bool& x)                      noexcept;
    
        const std::string   trace_path()                        const noexcept;
        void                trace_path(const std::string& x)          noexcept;

        uint                buffer_size()                       const noexcept;
        void                buffer_size(const uint& x)                noexcept;

        uint                port()                              const noexcept;
        void                port(const uint& x)                       noexcept;

    private:
        bool        m_help;
        bool        m_trace;
        std::string m_trace_path;
        uint        m_buffer_size;
        uint        m_port;
};

using argdesc_vt = std::vector<argdescbase_t<Args> *>;

void construct_arg_desc(argdesc_vt &aArgDesc);

#include "args.hh"

void construct_arg_desc(argdesc_vt& x) {
	// typedef argdesc_t<Args, char>        carg_t;
	// typedef argdesc_t<Args, int>         iarg_t;
    typedef argdesc_t<Args, uint>        uarg_t;
	// typedef argdesc_t<Args, float>       farg_t;
	// typedef argdesc_t<Args, double>      darg_t;
	typedef argdesc_t<Args, bool>        barg_t;
	typedef argdesc_t<Args, std::string> sarg_t;

	x.push_back( new barg_t("--help", false, &Args::help, "print this message" ));
	x.push_back( new barg_t("--trace", false, &Args::trace, "sets the flag for tracing"));
    x.push_back( new sarg_t("--trace-path", "./", &Args::trace_path, "path to log files"));
    x.push_back( new uarg_t("--buffer-size", 10240000, &Args::buffer_size, "sets the size of the memory buffer"));
    x.push_back( new uarg_t("--port", 8080u, &Args::port, "sets the port on which the server listens"));
}

Args::Args() noexcept
    : m_help(false)
    , m_trace(false)
    , m_trace_path("./")
    , m_buffer_size(2500 * PAGE_SIZE)
    , m_port(8080u)
{}

Args::~Args() noexcept = default;

bool Args::help() const noexcept 
{ 
    return m_help; 
}

void Args::help(const bool& x) noexcept 
{ 
    m_help = x; 
}

bool Args::trace() const noexcept 
{ 
    return m_trace; 
}

void Args::trace(const bool& x) noexcept 
{ 
    m_trace = x; 
}

const std::string Args::trace_path() const noexcept 
{ 
    return m_trace_path; 
}

void Args::trace_path(const std::string& x) noexcept 
{ 
    m_trace_path = x; 
}

uint Args::buffer_size() const noexcept
{
    return m_buffer_size;
}

void Args::buffer_size(const uint& x) noexcept
{
    m_buffer_size = x;
}

uint Args::port() const noexcept
{
    return m_port;
}

void Args::port(const uint& x) noexcept
{
    m_port = x;
}

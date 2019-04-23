#include "log.h"

Log::Log()
    : m_default(Log::DISABLE)
{ }

Log Log::Instance;

void Log::Default(Log::LogLevel level)
{
    Instance.m_default = level;
}

void Log::Set(std::string name, Log::LogLevel level)
{
    Instance.m_map[name] = level;
}

Log::LogLevel Log::Get(std::string name)
{
    if (Instance.m_map.find(name) != Instance.m_map.end())
    {
        return Instance.m_map[name];
    }
    return Instance.m_default;
}

void Log::Enable(std::string name)
{
    Set(name, ENABLE);
}

void Log::Disable(std::string name)
{
    Set(name, DISABLE);
}

#include <fstream>
#include "assert.h"
std::ostream* Log::GetOutstream()
{
    //return &std::cout;
    static bool init = false;
    static std::ofstream ofs("log.txt");
    if (!init)
    {
        if (!ofs.is_open())
        {
            std::cout << "initiliaze log stream failed" << std::endl;
            ASSERT(false);
        }
        init = true;
    }
    return &ofs;
}

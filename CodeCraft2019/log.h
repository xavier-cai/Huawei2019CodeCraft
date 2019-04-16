#ifndef LOG_H
#define LOG_H

#if defined(__linux__)
	#include <cxxabi.h>
#else
    #include <string.h>
    #include <stdlib.h>
	class abi
	{
	public:
		virtual ~abi() = 0;
		static const char* __cxa_demangle(const char* str, void* p1, void* p2, void* p3)
		{
            auto size = strlen(str);
            char* ret = (char*)malloc(size + 1);
            strncpy(ret, str, size);
            ret[size] = 0;
			return ret;
		}
	};
#endif //#if defined(__linux__)

#include <iostream>
#include <map>
#include <string>

class Log {
public:
    enum LogLevel {
        DISABLE,
        ENABLE,
    };//enum LogLevel

private:
    Log();
    static Log Instance;
    
    LogLevel m_default;
    std::map<std::string, LogLevel> m_map;

public:
    template <typename _T>
    static std::string GetName()
    {
        const char* name = abi::__cxa_demangle(typeid(_T).name(), NULL, NULL, NULL);
        std::string ret(name);
        free(const_cast<char*>(name));
        return ret;
    }
    template <typename _T>
    static std::string GetName(const _T& o)
    {
        return GetName<_T>();
    }

    static void Default(LogLevel level);
    static void Set(std::string name, LogLevel level);
    static LogLevel Get(std::string name);
    static void Enable(std::string name);
    static void Disable(std::string name);

    template <typename _T>
    static void Set(LogLevel level)
    {
        Set(GetName<_T>(), level);
    }

    template <typename _T>
    static LogLevel Get()
    {
        return Get(GetName<_T>());
    }
    
    template <typename _T>
    static void Enable()
    {
        Enable(GetName<_T>());
    }

    template <typename _T>
    static void Disable()
    {
        Disable(GetName<_T>());
    }

    static std::ostream* GetOutstream();

};//class Log

#define LOG_IS_ENABLE (Log::Get(Log::GetName(*this)) == Log::ENABLE)

#define LOG_IMPL(info, msg)                                 \
    do {                                                    \
        if (LOG_IS_ENABLE)                                  \
        {                                                   \
            std::ostream* os = Log::GetOutstream();         \
            if (os != 0)                                    \
                (*os) << info  << ' '                       \
                      /*<< Log::GetName(*this) << "::"*/        \
                      << __FUNCTION__ << ' '                \
                      << msg << std::endl;                  \
        }                                                   \
    } while(false)

#define LOG(msg) LOG_IMPL("", msg)

#endif

#pragma once

#define LOG(level,format,...) log::logPrint(level,format,__VA_ARGS__);



namespace vmm
{
	namespace log
	{
		
		enum LogLevel
		{
			DEBUG,
			INFO,
			WARNNING,
			ERROR,
			VERBOSE
		};

		void logPrint(LogLevel level, const char* format, ...);

	}
}


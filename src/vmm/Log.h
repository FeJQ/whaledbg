#pragma once



	namespace vmm
	{
		namespace log
		{
			enum LogLevel
			{
				LOG_LEVEL_DEBUG,
				LOG_LEVEL_ERROR,
				LOG_LEVEL_DUMP,
				LOG_LEVEL_INFO
			};

			void logPrint(LogLevel level, const char* format, ...);

		}
	}


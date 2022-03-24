#include "Log.h"
#include <ntifs.h>
#include <stdarg.h>
#include <ntstrsafe.h>

namespace vmm
{
	namespace log
	{
		void logPrint(LogLevel level, const char* format, ...)
		{
			char* logPrifix;
			LARGE_INTEGER systemTime;
			LARGE_INTEGER localTime;
			TIME_FIELDS timeFields;
			char timeBuffer[20] = {};
			char messageBuffer[412] = {};
			char outputBuffer[512] = {};
			va_list args = {};

			switch (level)
			{
			case LOG_LEVEL_DEBUG:
			{
				logPrifix = "[DEBUG]";
				break;
			}
			case LOG_LEVEL_DUMP:
			{
				logPrifix = "[DUMP]";
				break;
			}
			case LOG_LEVEL_ERROR:
			{
				logPrifix = "[ERROR]";
				break;
			}
			case LOG_LEVEL_INFO:
			{
				logPrifix = "[INFORMATION]";
				break;
			}
			default:
			{
				logPrifix = "[UNKNOWN]";
				break;
			}

			}

			KeQuerySystemTime(&systemTime);
			ExSystemTimeToLocalTime(&systemTime, &localTime);
			RtlTimeToTimeFields(&localTime, &timeFields);

			RtlStringCchPrintfA(
				timeBuffer,
				sizeof(timeBuffer),
				"[%02hd:%02hd:%02hd.%03hd]",
				timeFields.Hour,
				timeFields.Minute,
				timeFields.Second,
				timeFields.Milliseconds);

			va_start(args, format);
			RtlStringCchVPrintfA(messageBuffer, sizeof(messageBuffer), format, args);
			va_end(args);

			char* outputFormat = "%s  %s  %s\r\n";

			RtlStringCchPrintfA(
				outputBuffer,
				sizeof(outputBuffer),
				outputFormat,
				timeBuffer,
				logPrifix,
				messageBuffer);

			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%s", outputBuffer);
		}
	}

}

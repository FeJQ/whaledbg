#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <ntddk.h>
#include <intrin.h>

#ifdef _DEBUG
#define DbgLog(logLevel,...) {Common::log(__FUNCTION__,logLevel,__VA_ARGS__);}
#else 
#define DbgLog(message,value)
#endif // DEBUG

#define NT_CHECK(x) if (!NT_SUCCESS(status)){return status;}

#define Export extern "C" __declspec(dllexport)

#define ASM

#ifdef  __cplusplus
#define EXTERN_C_BEGIN extern "C"{    
#define EXTERN_C_END }  
#else 
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

#define GetPageHead(PA) (PA & 0xFFFFFFFFFFFFF000ull)

class Common
{
public:
	enum class LogLevel
	{
		Info,
		Warnning,
		Error,
		Crash,
	};
	static void log(const char* functionName, LogLevel level, const char* format...)
	{
		char* levelHeader;
		switch (level)
		{
		case LogLevel::Info:
			levelHeader = "Info:";
			break;
		case LogLevel::Warnning:
			levelHeader = "Warnning:";
			break;
		case LogLevel::Error:
			levelHeader = "Error:";
			break;
		case LogLevel::Crash:
			levelHeader = "Crash:";
			break;
		default:
			levelHeader = "";
			break;
		}
#ifdef _DEBUG
		const int LOG_SIZE = 256;
		char message[LOG_SIZE] = { 0 };
		const char* formatValue = format;
		int argCount = 1;
		va_list va;
		va_start(va, format);
		vsprintf(message, format, va);
		va_end(va);
		//sprintf(finalBuff, "[WhaleDbg::%s] %s%s", functionName, levelHeader, message);
		DbgPrint("[WhaleDbg::%s] %s%s\n", levelHeader, message);
#endif	
		return;
	}
};
#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <ntddk.h>

#define override

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
		char message[256] = { 0 };
		char finalBuff[256] = { 0 };
		va_list va;
		va_start(va, format);
		vsprintf_s(message, format, va);
		va_end(va);
		sprintf(finalBuff, "[WhaleDbg::%s] %s%s", functionName, levelHeader, message);
		KdPrint((message));
#endif	
		return;
	}
};
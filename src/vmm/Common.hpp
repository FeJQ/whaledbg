#pragma once
#include <stdarg.h>
#include <stdio.h>


#ifdef DEBUG
#define Log(message,value) {{KdPrint(("[HVM] %-40s [%p]\n",message,value));}}
#else 
#define DbgLog(message,value)
#endif // DEBUG

class Common
{
public:
	static void log(const char* format...)
	{
#ifdef _DEBUG
		char buff[256] = { 0 };
		va_list va;
		va_start(va, format);
		vsprintf_s(buff, format, va);
		va_end(va);
		KdPrint((buff));
#else
		return;
#endif	
	}
};
#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <ntddk.h>
#include <intrin.h>

#ifdef _DEBUG
//#define DbgLog(logLevel,...) {Common::log(__FUNCTION__,logLevel,__VA_ARGS__);}
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
	
	
};
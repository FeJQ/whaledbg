#pragma once
#include <ntddk.h>

class VmManager
{
public:
	virtual NTSTATUS checkVtAvailable() = 0;
	virtual NTSTATUS checkEptAvailable()=0;

};


#pragma once
#include <ntddk.h>

class VtManager
{
public:
	virtual NTSTATUS checkVtAvailable() = 0;
	virtual NTSTATUS checkEptAvailable() = 0;
	virtual NTSTATUS enableVtFeature() = 0;
	virtual NTSTATUS allocateRegion() = 0;
};


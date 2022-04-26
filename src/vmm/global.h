#pragma once
#include <Native.h>
#include "Common.h"

#define POOL_TAG_VMXON 'vmon'
#define POOL_TAG_VMCS 'vmcs'
#define POOL_TAG_HOST_STACK 'hstk'

enum VmxState
{
    Off,           // No virtualization
    Transition,    // Virtualized, context not yet restored
    On,            // Virtualized, running guest
};















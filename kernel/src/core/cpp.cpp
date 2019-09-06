#include <stddef.h>
#include "util/core.h"

/* static variable guarding routines */
namespace __cxxabiv1
{
    __extension__ typedef int __guard __attribute__((mode(__DI__)));

    extern "C" int __cxa_guard_acquire(__guard* g)
    {
        /* TODO: implement mutex */
        return !*(char*)(g);
    }

    extern "C" void __cxa_guard_release(__guard* g)
    {
        /* TODO: implement mutex release */
        *(char*)g = 1;
    }

    extern "C" void __cxa_guard_abort(__guard* g)
    {
        /* TODO: implement mutex release */
        UNUSED(g);
    }
}

/* Memory management features */
void* operator new(size_t size)
{
    /* TODO: implement memoroy allocation */
    UNUSED(size);

    return (void*)0;
}

void* operator new[](size_t size)
{
    /* TODO: implement memory allocation */
    UNUSED(size);

    return (void*)0;
}

void operator delete(void* p)
{
    /* TODO: implement memory deallocation */
    UNUSED(p);
}

void operator delete[](void* p)
{
    /* TODO: implement memory deallocation */
    UNUSED(p);
}

/* TODO: define placement new and delete */
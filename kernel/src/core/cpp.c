#include "core/cpp.h"

void _init(void);
extern void _fini(void);

void __cxa_pure_virtual(void)
{
    /* TODO: add a kernel panic */
}

void cpp_init(void)
{
    _init();
}

void cpp_destroy(void)
{
    _fini();
}
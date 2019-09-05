#include "paging/paging.h"
#include "core/cpp.h"

void kernel_main(void* hw_info)
{
    cpp_init();
    paging_init();

    cpp_destroy();
}
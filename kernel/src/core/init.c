#include "paging/paging.h"
#include "core/cpp.h"
#include "core/sse.h"

void kernel_main(void* hw_info)
{
    sse_init();
    cpp_init();
    paging_init();

    cpp_destroy();
}
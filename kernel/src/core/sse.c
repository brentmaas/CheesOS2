#include "core/sse.h"

void sse_init_avx(void)
{
    /* Determine if avx is supported */
}

void sse_init(void)
{
    /* SSE should be supported, initialize is */
    sse_init_native();

    /* Enable AVX if supported */
    sse_init_avx();
}
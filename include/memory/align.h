#ifndef _CHEESOS2_MEMORY_ALIGN_H
#define _CHEESOS2_MEMORY_ALIGN_H

#define ALIGN_BACKWARD_2POW(addr, align) ((addr) & ~((align) - 1))
#define ALIGN_FORWARD_2POW(addr, align) (ALIGN_BACKWARD_2POW((addr) + (align) - 1, (align)))

#define IS_ALIGNED_TO(addr, align) ((addr) % (align) == 0)

#endif

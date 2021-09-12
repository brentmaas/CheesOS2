#ifndef _CHEESOS2_MEMORY_ALIGN_H
#define _CHEESOS2_MEMORY_ALIGN_H

// Note: we need (typeof(addr)) to ensure that align is coerced to the first argument size
#define ALIGN_BACKWARD_2POW(addr, align) ((addr) & ~((typeof(addr)) (align) - 1U))
#define ALIGN_FORWARD_2POW(addr, align) ({typeof(addr) _align = (align); ALIGN_BACKWARD_2POW((addr) + _align - 1U, _align);})

#define IS_ALIGNED_TO(addr, align) ((addr) % (align) == 0)

#endif

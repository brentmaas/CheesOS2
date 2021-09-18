#ifndef _CHEESOS2_UTILITY_CONTAINER_OF_H
#define _CHEESOS2_UTILITY_CONTAINER_OF_H

#define CONTAINER_OF(container_type, member, member_ptr) ({                                 \
        typedef typeof(((container_type*) 0)->member) _member_type;                         \
        const _member_type* _member_ptr = (member_ptr); /* invalid member pointer type */   \
        (container_type*) ((char*) _member_ptr - offsetof(container_type, member));         \
    })


#endif

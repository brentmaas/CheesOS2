#ifndef _CHEESOS2_UTILITY_BITCAST_H
#define _CHEESOS2_UTILITY_BITCAST_H

#define BITCAST(result_type, expr) ({                       \
        typedef typeof(expr) _expr_type;                    \
        _Static_assert(                                     \
            sizeof(result_type) == sizeof(_expr_type),      \
            "Incompatible bitcast sizes");                  \
        (union{                                             \
            _expr_type a;                                   \
            result_type b;                                  \
        }){.a = (expr)}.b;                                  \
    })

#endif

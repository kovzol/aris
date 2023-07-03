#ifndef VAR_H
#define VAR_H

#ifdef __cplusplus
extern "C" {
#endif
#include "typedef.h"

// The variable structure.

struct variable {
    unsigned char * text;  // The text of the variable.
    int arbitrary : 1;     // Whether or not the variable is arbitrary.
};


variable * variable_init (unsigned char * text,
                        int arbitrary);

#ifdef __cplusplus
}
#endif

#endif // VAR_H

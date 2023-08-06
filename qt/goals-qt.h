#ifndef GOALSQT_H
#define GOALSQT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

int qtgoal_check_line (proof_t * proof, unsigned char * sen_text, vec_t * rets, int * ln, int * is_valid);

#ifdef __cplusplus
}
#endif

#endif // GOALSQT_H

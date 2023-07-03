#ifndef PROOF_H
#define PROOF_H

#ifdef __cplusplus
extern "C" {
#endif
#include "typedef.h"

// Proof data structure.

struct proof {
    list_t * everything;  // List of sentences of this proof.
    list_t * goals;       // List of goals for this proof.
    int boolean : 1;      // Whether or not this is a boolean mode proof.
};

proof_t * proof_init ();
void proof_destroy (proof_t * proof);
int proof_eval (proof_t * proof, vec_t * rets, int verbose);
int eval_proof (list_t * everything, vec_t * rets, int verbose);

int convert_proof_latex (proof_t * proof, const char * filename);
#ifdef __cplusplus
}
#endif


#endif // PROOF_H

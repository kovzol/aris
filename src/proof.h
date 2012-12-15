/* Proof data type.

   Copyright (C) 2012 Ian Dunn.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PROOF_H
#define PROOF_H

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

#endif  /*  PROOF_H  */

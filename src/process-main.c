/* Contains the main process function, from which the others are called.

   Copyright (C) 2012, 2013, 2014 Ian Dunn.

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

#include "process.h"
#include "vec.h"

char *
process (unsigned char * conc, vec_t * prems, const char * rule, vec_t * vars,
	 proof_t * proof)
{
  unsigned char * conclusion;

  conclusion = conc;

  char * infer, * equiv, * quant, * misc, * bool;

  infer = process_inference (conclusion, prems, rule);
  if (!infer)
    return NULL;

  if (strncmp (infer, NOT_MINE, 28))
    return infer;

  equiv = process_equivalence (conclusion, prems, rule);
  if (!equiv)
    return NULL;
  if (strncmp (equiv, NOT_MINE, 28))
    return equiv;

  bool = process_bool (conclusion, prems, rule);
  if (!bool)
    return NULL;
  if (strncmp (bool, NOT_MINE, 28))
    return bool;

  quant = process_quantifiers (conclusion, prems, rule, vars);
  if (!quant)
    return NULL;
  if (strncmp (quant, NOT_MINE, 28))
    return quant;

  misc = process_misc (conclusion, prems, rule, vars, proof);
  if (!misc)
    return NULL;
  if (strncmp (misc, NOT_MINE, 28))
    return misc;

  return "Rule not recognized.";
}

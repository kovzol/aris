/* Function to check a goal line (platform independent but named with qt as it's required only for the qt version).
   Inspired from goals.h.

   Copyright (C) 2023 Saksham Attri.

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
#ifndef GOALSQT_H
#define GOALSQT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../src/typedef.h"

int qtgoal_check_line (proof_t * proof, unsigned char * sen_text, vec_t * rets, int * ln, int * is_valid);

#ifdef __cplusplus
}
#endif

#endif // GOALSQT_H

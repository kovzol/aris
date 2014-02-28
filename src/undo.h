/* Contains the undo data type and function declarations.

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


#ifndef ARIS_UNDO_H
#define ARIS_UNDO_H

#include "typedef.h"

enum UNDO_OP {
  UNDO_ADD_SEN = 0,
  UNDO_REM_SEN,
  UNDO_ADD_GOAL,
  UNDO_REM_GOAL,
  UNDO_MOD_TEXT
};

enum UNDO_INFO_TYPE {
  UIT_ADD_SEN = 0,
  UIT_REM_SEN,
  UIT_ADD_GOAL,
  UIT_REM_GOAL,
  UIT_MOD_TEXT
};

#define UNDO_INT 1

struct undo_info {
  int type;
  time_t stamp;
  list_t * ls;
};

typedef int (*undo_op) (aris_proof *, undo_info *);

undo_info undo_info_init (aris_proof * ap, list_t * sens, int type);
undo_info undo_info_init_one (aris_proof * ap, sentence * sen, int type);
void undo_info_destroy (undo_info ui);

undo_op undo_determine_op (int undo, int type);
int undo_op_remove (aris_proof * ap, undo_info * ui);
int undo_op_add (aris_proof * ap, undo_info * ui);
int undo_op_mod (aris_proof * ap, undo_info * ui);

#endif /*  ARIS_UNDO_H  */

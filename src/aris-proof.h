/*  Contains the data backends for the aris GUI.

   Copyright (C) 2012, 2013 Ian Dunn.

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


#ifndef ARIS_PROOF_H
#define ARIS_PROOF_H

#include "pound.h"
#include "typedef.h"
#include "sen-parent.h"
#include "undo.h"
#include <time.h>

#define ARIS_PROOF(o) ((aris_proof *) o)

// The main proof/gui structure.

struct aris_proof {
  struct sen_parent sp;	// The parent object of this proof.

  goal_t * goal;	// The goal structure for this proof.

  int edited : 1;	// Whether or not this proof has been edited.
  char * cur_file;	// The current file associated with this proof.

  item_t * fin_prem;	// The final premise.
  list_t * yanked;	// The currently yanked lines.
  list_t * selected;	// The currently selected lines.

  char * sb_text;	// The statusbar text - may not be needed.
  int boolean : 1;	// Whether or not the proof is in boolean mode.

  vec_t * undo_stack;	// The stack of previous actions to undo.
  int undo_pt;		// The position within the undo stack.
};

aris_proof * aris_proof_init ();
int aris_proof_post_init (aris_proof * ap);
aris_proof * aris_proof_init_from_proof (proof_t * proof);
void aris_proof_destroy (aris_proof * ap);

void aris_proof_create_menu (sen_parent * ap);

int aris_proof_set_changed (aris_proof * ap, int changed, undo_info ui);
int aris_proof_adjust_lines (aris_proof * ap, item_t * itm, int mod);

proof_t * aris_proof_to_proof (aris_proof * ap);

sentence * aris_proof_create_sentence (aris_proof * ap, sen_data * sd, int ui);
sentence * aris_proof_create_new_prem (aris_proof * ap);
sentence * aris_proof_create_new_conc (aris_proof * ap);
sentence * aris_proof_create_new_sub (aris_proof * ap);
sentence * aris_proof_end_sub (aris_proof * ap);

int aris_proof_remove_sentence (aris_proof * ap, sentence * sen);

void aris_proof_set_font (aris_proof * ap, int font);
void aris_proof_set_sb (aris_proof * ap, char * sb_text);
int aris_proof_set_filename (aris_proof * ap, const char * filename);

int aris_proof_kill (aris_proof * ap);
int aris_proof_yank (aris_proof * ap);
int aris_proof_copy (aris_proof * ap);

int aris_proof_clear_selected (aris_proof * ap);
int aris_proof_select_sentence (aris_proof * ap, sentence * sen);
int aris_proof_deselect_sentence (aris_proof * ap, sentence * sen);

int aris_proof_toggle_boolean_mode (aris_proof * ap);

int aris_proof_submit (aris_proof * ap, const char * hw,
		       const char * user_email,
		       const char * instr_email);

int aris_proof_import_proof (aris_proof * ap);

int aris_proof_undo_stack_push (aris_proof * ap, undo_info ui);
int aris_proof_undo_stack_pop (aris_proof * ap);

int aris_proof_undo (aris_proof * ap, int undo);


#endif /*  ARIS_PROOF_H  */

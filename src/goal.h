/*  Contains the data structure for the goal GUI.

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

#ifndef ARIS_GOAL_H
#define ARIS_GOAL_H

#include <gtk/gtk.h>
#include "typedef.h"
#include "sen-parent.h"

#define GOAL(o) ((goal_t *) o)

// The goal list structure.

struct goal {
  sen_parent sp;
  aris_proof * parent;  // The parent of this goal.
  int toggle;           // 1 if the goal window is toggled else 0
};

goal_t * goal_init (aris_proof * ag);
goal_t * goal_init_from_list (aris_proof * ap, list_t * goals);
void goal_destroy (goal_t * goal);
void goal_gui_create_menu (sen_parent * goal);
int goal_check_line (goal_t * goal, sentence * sen);
int goal_check_all (goal_t * goal);
int goal_add_line (goal_t * goal, sen_data * sd);
int goal_rem_line (goal_t * goal);
int goal_update_title (goal_t * goal);

#endif  /*  ARIS_GOAL_H  */

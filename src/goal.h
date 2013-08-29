/*  Contains the data structure for the goal GUI.

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

#ifndef ARIS_GOAL_H
#define ARIS_GOAL_H

#include <gtk/gtk.h>
#include "typedef.h"

#define GOAL(o) ((goal_t *) o)

// The goal list structure.

struct goal {
  // Starting here, the order of these elements must match those in sen_parent.
  GtkWidget * window;          // The main window.
  GtkWidget * vbox;            // The container for the menu, statusbar, and
                               //   scrolledwindow.
  GtkWidget * menubar;         // The menu bar for this gui.
  GtkWidget * statusbar;       // The statusbar that displays status messages.
  GtkWidget * scrolledwindow;  // The scrolledwindow that contains the viewport.
  GtkWidget * viewport;        // The vewport that allows scrolling through sentences.
  GtkWidget * container;       // The container of the sentences.
  GtkWidget * separator;       // The separator that separates prems from concs.
  GtkAccelGroup * accel;       // The accelerator for the keybindings.

  GdkPixbuf * conn_pixbufs[11];
  struct list * goals;    // The list of sentences.
  struct item * focused;  // The currently focused sentence.
  int font;               // The index of the font in the_app->fonts.
  int type;               // The type of sentence parent.
  // sen_parent ends here.

  aris_proof * parent;  // The parent of this goal.
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

/* The sentence parent data type.

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

#ifndef ARIS_SEN_PARENT_H
#define ARIS_SEN_PARENT_H

#include <gtk/gtk.h>

#define SEN_PARENT(o) ((sen_parent *) o)

enum SEN_PARENT_TYPE {
  SEN_PARENT_TYPE_PROOF = 0,
  SEN_PARENT_TYPE_GOAL
};

typedef struct sen_parent sen_parent;
typedef struct sentence sentence;
typedef struct item item_t;
typedef struct sen_data sen_data;

// The data structure that goal and aris_proof 'inherit' from.

struct sen_parent {
  GtkWidget * window;          // The main window.
  GtkWidget * vbox;            // The container for the menu, statusbar, and
                               // scrolledwindow.
  GtkWidget * menubar;         // The menu bar for this gui.
  GtkWidget * statusbar;       // The statusbar that displays status messages.
  GtkWidget * scrolledwindow;  // The scrolledwindow that contains the viewport.
  GtkWidget * viewport;        // The vewport that allows scrolling through sentences.
  GtkWidget * container;       // The container of the sentences.
  GtkWidget * separator;       // The separator that separates prems from concs.
  GtkAccelGroup * accel;       // The accelerator for the keybindings.

  struct list * everything;  // The list of sentences.
  struct item * focused;     // The currently focused sentence.
  int font;                  // The index of the font in the_app->fonts.
  int type;                  // The type of sentence parent.
};

void sen_parent_init (sen_parent * sp, const char * title,
		      int width, int height,
		      void (* menu_func) (sen_parent *),
		      int type);
void sen_parent_destroy (sen_parent * sp);
void sen_parent_set_font (sen_parent * sp, int new_font);
void sen_parent_set_sb (sen_parent * sp, char * sb_text);
item_t * sen_parent_ins_sentence (sen_parent * sp, sen_data * sd,
				  item_t * fcs, int new_order);
item_t * sen_parent_rem_sentence (sen_parent * sp, sentence * sen);
void sen_parent_set_focus (sen_parent * sp, item_t * focus);

#endif

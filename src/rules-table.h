/* The rules table data type.

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

#ifndef ARIS_RULES_TABLE_H
#define ARIS_RULES_TABLE_H

#include "pound.h"
#include "rules.h"
#include "typedef.h"

#define TOGGLE_BUTTON(b) { gboolean is_toggled = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (b)); gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), !is_toggled);}

struct rules_group {
  GtkWidget * frame;
  GtkWidget * table;
};

struct rules_table {
  // The table that contains the rules groups.
  GtkWidget * window;
  GtkWidget * vbox;
  GtkWidget * menubar;
  GtkAccelGroup * accel;
  GtkWidget * layout;

  // The groups of the rules.
  rules_group * infer;
  rules_group * equiv;
  rules_group * pred;
  rules_group * misc;
  rules_group * boole;

  GtkWidget * rules[NUM_RULES];

  int toggled;
  int user:1;

  int font;
  int boolean : 1;
};

rules_group * rules_group_init (int num_rules, char * label, rules_table * parent);
rules_table * rules_table_init (int boolean);
void rules_table_destroy (rules_table * rt);
void rules_table_create_menu (rules_table * rt);
void rule_toggled (int index);
int rules_table_focused ();
int rules_table_align (rules_table * rt, aris_proof * ap);
int rules_table_set_font (rules_table * rt, int font);
int rules_table_set_boolean_mode (rules_table * rt, int boolean);
int rules_table_set_lm (rules_table * rt, sentence * sen, char * filename);
int rules_table_destroy_menu_item (sentence * sen);

#endif  /*  ARIS_RULES_TABLE_H  */

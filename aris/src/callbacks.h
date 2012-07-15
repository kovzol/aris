/* The declarations of callback functions.

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

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "pound.h"

typedef struct aris_proof aris_proof;
typedef struct sentence sentence;
typedef struct item item_t;

G_MODULE_EXPORT void toggled (GtkToggleButton * tb,
			      gpointer data);
G_MODULE_EXPORT gboolean rules_table_focus (GtkWidget * widget,
					    GdkEvent * event,
					    gpointer data);
G_MODULE_EXPORT void rule_menu_activated (GtkMenuItem * menuitem,
					  gpointer data);
G_MODULE_EXPORT gboolean rules_table_deleted (GtkWidget * widget,
					      GdkEvent * event,
					      gpointer data);

G_MODULE_EXPORT void menu_activate (GtkMenuItem * menuitem,
				    gpointer data);
G_MODULE_EXPORT gboolean window_delete (GtkWidget * widget,
					GdkEvent * event,
					gpointer data);
G_MODULE_EXPORT gboolean window_focus_in (GtkWidget * widget,
					  GdkEvent * event,
					  gpointer data);
G_MODULE_EXPORT gboolean sen_parent_btn_press (GtkWidget * widget,
					       GdkEventButton * event,
					       gpointer data);


G_MODULE_EXPORT gboolean sentence_focus_in (GtkWidget * widget,
					    GdkEvent * event,
					    gpointer data);
G_MODULE_EXPORT gboolean sentence_focus_out (GtkWidget * widget,
					     GdkEvent * event,
					     gpointer data);
G_MODULE_EXPORT gboolean sentence_btn_press (GtkWidget * widget,
					     GdkEventButton * event,
					     gpointer data);
G_MODULE_EXPORT gboolean sentence_btn_release (GtkWidget * widget,
					       GdkEventButton * event,
					       gpointer data);
G_MODULE_EXPORT gboolean sentence_key_press (GtkWidget * widget,
					     GdkEventKey * event,
					     gpointer data);
G_MODULE_EXPORT void sentence_changed (GtkTextView * edit, gpointer data);
G_MODULE_EXPORT void sentence_mapped (GtkWidget * widget,
				      GdkRectangle * rect,
				      gpointer data);
					  

G_MODULE_EXPORT void goal_menu_activate (GtkMenuItem * item, gpointer data);
G_MODULE_EXPORT gboolean goal_delete (GtkWidget * widget,
				      GdkEvent * event,
				      gpointer data);
G_MODULE_EXPORT gboolean goal_focus_in (GtkWidget * widget,
					GdkEvent * event,
					gpointer data);


// Functions to be CALLED BY the actual callbacks.

int gui_destroy (aris_proof * ap);

item_t * gui_copy (aris_proof * ap, sentence * sen);
item_t * gui_kill (aris_proof * ap, sentence * sen);
int gui_yank (aris_proof * ap);

int gui_new ();
int gui_save (aris_proof * ap, int save_as);
int gui_open (GtkWidget * window);

int evaluate_line (aris_proof * ap, sentence * sen);
int evaluate_proof (aris_proof * ap);

int gui_keydown (aris_proof * ap, unsigned int mask, unsigned int val);
int gui_goal_check (aris_proof * ap);

int gui_toggle_rules (aris_proof * ap);
int gui_set_custom (GtkWidget * window, int cur_font);

int gui_about (GtkWidget * window);
int gui_help ();
int menu_activated (aris_proof * ap, int menu_id);

#endif /* CALLBACKS_H */

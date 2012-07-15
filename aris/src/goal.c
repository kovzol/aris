/*  Functions to handle the goal structures.

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

#include <stdlib.h>
#include <string.h>

#include "goal.h"
#include "list.h"
#include "sentence.h"
#include "aris-proof.h"
#include "menu.h"
#include "app.h"
#include "sen-data.h"
#include "sen-parent.h"
#include "callbacks.h"
#include "process.h"

// The goal menu data.

static mid_t goal_data[7] =
  {
    {"Add Goal", "Add a new goal for this proof." , GTK_STOCK_ADD, GDK_KEY_j, GDK_CONTROL_MASK, STANDARD, MENU_ADD_PREM},
    {"Remove Goal", "Remove the current goal for this proof.", GTK_STOCK_REMOVE, GDK_KEY_k, GDK_CONTROL_MASK, STANDARD, MENU_KILL},
    {NULL, NULL, NULL, -1, -1, SEPARATOR, -1},
    {"Check Line", "Check if the current goal has been met.", GTK_STOCK_INDEX, GDK_KEY_e, GDK_CONTROL_MASK, STANDARD, MENU_EVAL_LINE},
    {"Check All", "Check if all goals have been met.", GTK_STOCK_SELECT_ALL, GDK_KEY_f, GDK_CONTROL_MASK, STANDARD, MENU_EVAL_PROOF},
    {NULL, NULL, NULL, -1, -1, SEPARATOR, -1},
    {"Hide Goals", "Hide the goals window for this proof.", GTK_STOCK_REFRESH, GDK_KEY_l, GDK_CONTROL_MASK, STANDARD, MENU_GOAL}
  };


/* Initializes a goal structure.
 *  input:
 *    ap - the new parent of the goal structure.
 *  output:
 *    the goal structure, or NULL on error.
 */
goal_t *
goal_init (aris_proof * ap)
{
  goal_t * goal;

  goal = (goal_t *) calloc (1, sizeof (goal_t));
  CHECK_ALLOC (goal, NULL);

  goal->parent = ap;
  sen_parent_init ((sen_parent *) goal, "GNU Aris - Untitled - Goals", 360, 100,
		   goal_gui_create_menu, SEN_PARENT_TYPE_GOAL);

  gtk_window_set_transient_for (GTK_WINDOW (goal->window),
				GTK_WINDOW (SEN_PARENT (ap)->window));

  if (ap->cur_file)
    {
      int ret;
      ret = goal_update_title (goal);
      if (ret == -1)
	return NULL;
    }
    
  goal->goals = init_list ();
  if (!goal->goals)
    return NULL;

  g_signal_connect (goal->window, "delete-event",
		    G_CALLBACK (goal_delete), (gpointer) goal);
  g_signal_connect (goal->window, "focus-in-event",
		    G_CALLBACK (goal_focus_in), (gpointer) goal);

  return goal;
}

/* Initializes a goal from a list of goals.
 *  input:
 *    ap - the new parent of the goal structure.
 *    goals - the list of strings representing goals.
 *  output:
 *    the new goal structure, or NULL on error.
 */
goal_t *
goal_init_from_list (aris_proof * ap, list_t * goals)
{
  goal_t * goal;

  goal = goal_init (ap);
  if (!goal)
    return NULL;

  item_t * g_itr;
  for (g_itr = goals->head; g_itr != NULL; g_itr = g_itr->next)
    {
      sen_data * sd;
      sd = sen_data_init (-1, -1, (unsigned char *) g_itr->value,
			  NULL, 1, NULL, 0, 0, NULL);
      if (!sd)
	return NULL;

      int ret;
      ret = goal_add_line (goal, sd);
      if (ret < 0)
	return NULL;
    }

  return goal;
}

/* Destroys a goal structure.
 *  input:
 *    goal - the goal being destroyed.
 *  output:
 *    none.
 */
void
goal_destroy (goal_t * goal)
{
  goal->parent = NULL;
  sen_parent_destroy ((sen_parent *) goal);
}

/* Creates the goal menu, used as a function in sen_parent_init.
 *  input:
 *    goal - the goal, or sentence parent, for which to initialize the menu.
 *  output:
 *    none.
 */
void
goal_gui_create_menu (sen_parent * goal)
{
  GtkWidget * goal_menu, * goal_sub_menu;
  int i, toss_me = 0;

  goal->menubar = gtk_menu_bar_new ();
  goal_sub_menu = gtk_menu_new ();
  goal_menu = gtk_menu_item_new_with_label ("Goal");
  
  for (i = 0; i < 7; i++)
    construct_menu_item (goal_data[i], G_CALLBACK (goal_menu_activate), goal_sub_menu, goal->accel, &toss_me);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (goal_menu), goal_sub_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (goal->menubar), goal_menu);
}

/* Checks a line in the goal.
 *  input:
 *    goal - the goal containing the sentence to be checked.
 *    sen - the sentence being checked.
 *  output:
 *    0 on success, -1 on error.
 */
int
goal_check_line (goal_t * goal, sentence * sen)
{
  // First, check for text errors.

  int ret_check = check_text (sen->text);
  if (ret_check < 0)
    return -1;

  unsigned char * cmp_text;
  cmp_text = die_spaces_die (sen->text);
  if (!cmp_text)
    return -1;

  item_t * ev_itr;
  int is_valid = 1;

  for (ev_itr = SEN_PARENT (goal->parent)->everything->head; ev_itr != NULL;
       ev_itr = ev_itr->next)
    {
      sentence * ev_sen = ev_itr->value;
      unsigned char * ev_cmp_text = die_spaces_die (ev_sen->text);
      if (!ev_cmp_text)
	return -1;

      if (!strcmp (ev_cmp_text, cmp_text))
	{
	  if (ev_sen->premise || ev_sen->subproof)
	    {
	      if (ev_sen->value_type == VALUE_TYPE_ERROR)
		is_valid = 0;
	    }
	  else if (ev_sen->value_type != VALUE_TYPE_TRUE)
	    is_valid = 0;

	  sentence_set_line_no (sen, ev_sen->line_num);

	  if (is_valid)
	    {
	      gtk_widget_modify_bg (ev_sen->eventbox, GTK_STATE_NORMAL,
				    the_app->bg_colors[BG_COLOR_EMERALD]);
	      sentence_set_value (sen, VALUE_TYPE_TRUE);
	    }
	  else
	    {
	      gtk_widget_modify_bg (ev_sen->eventbox, GTK_STATE_NORMAL,
				    the_app->bg_colors[BG_COLOR_CRIMSON]);
	      sentence_set_value (sen, VALUE_TYPE_REF);
	    }

	  char * sb_text = (char *) calloc (30, sizeof (char *));
	  CHECK_ALLOC (sb_text, -1);
	  int offset = 0;
	  offset += sprintf (sb_text, "The goal was met at line %i",
			     ev_sen->line_num);
	  if (!is_valid)
	    {
	      offset += sprintf (sb_text + offset,
				 ", however there are errors leading up to it.");
	    }
	  sen_parent_set_sb ((sen_parent *) goal, sb_text);
	  free (ev_cmp_text);
	  free (cmp_text);
	  return 0;
	}
      else
	{
	  if (ev_sen->premise || ev_sen->subproof)
	    {
	      if (ev_sen->value_type == VALUE_TYPE_ERROR)
		is_valid = 0;
	    }
	  else if (ev_sen->value_type != VALUE_TYPE_TRUE)
	    {
	      is_valid = 0;
	    }
	}

      free (ev_cmp_text);
    }

  free (cmp_text);

  sentence_set_value (sen, VALUE_TYPE_FALSE);
  sen_parent_set_sb ((sen_parent *) goal, "This goal has not been met.");
  return 0;
}

/* Checks all of the sentences in a goal.
 *  input:
 *    goal - the goal for which all of the sentences are being checked.
 *  output:
 *    0 on success, -1 on error.
 */
int
goal_check_all (goal_t * goal)
{
  item_t * gl_itr;
  int ret_check;

  for (gl_itr = goal->goals->head; gl_itr != NULL; gl_itr = gl_itr->next)
    {
      ret_check = goal_check_line (goal, gl_itr->value);
      if (ret_check == -1)
	return -1;
    }

  return 0;
}

/* Adds a line to a goal.
 *  input:
 *    goal - the goal for which a line is being added.
 *    sd - the sentence data from which to initialize the line.
 *  output:
 *    0 on success, -1 on error.
 */
int
goal_add_line (goal_t * goal, sen_data * sd)
{
  sentence * sen;
  sen = sentence_init (sd, (sen_parent *) goal,
		       goal->goals->tail);
  if (!sen)
    return -1;

  item_t * itm;
  itm = ls_push_obj (goal->goals, sen);
  if (!itm)
    return -1;

  gtk_box_pack_start (GTK_BOX (goal->container), sen->panel,
		      FALSE, FALSE, 0);
  gtk_widget_show_all (sen->panel);

  int ret;
  ret = aris_proof_set_changed (goal->parent, 1);
  if (ret < 0)
    return -1;

  return 0;
}

/* Removes a line from a goal.
 *  input:
 *    goal - the goal from which a line is being removed.
 *  output:
 *    0 on success, -1 on error.
 */
int
goal_rem_line (goal_t * goal)
{
  int ret;

  sen_parent_rem_sentence ((sen_parent *) goal, goal->focused->value);

  ret = aris_proof_set_changed (goal->parent, 1);
  if (ret < 0)
    return -1;

  return 0;
}

/* Updates a goal's title to match its parent's file name.
 *  input:
 *    goal - The goal for which to update the title.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
goal_update_title (goal_t * goal)
{
  GFile * file;
  char * base_name, * new_title;

  new_title = calloc (strlen (goal->parent->cur_file) + 20, sizeof (char));
  CHECK_ALLOC (new_title, -1);

  file = g_file_new_for_path (goal->parent->cur_file);
  base_name = g_file_get_basename (file);
  sprintf (new_title, "GNU Aris - %s - Goals", base_name);

  gtk_window_set_title (GTK_WINDOW (SEN_PARENT (goal)->window), new_title);

  return 0;
}

/*  Functions for handling the aris proof guis.

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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "aris-proof.h"
#include "sen-parent.h"
#include "sentence.h"
#include "rules.h"
#include "rules-table.h"
#include "app.h"
#include "menu.h"
#include "proof.h"
#include "sen-data.h"
#include "var.h"
#include "goal.h"
#include "callbacks.h"
#include "list.h"
#include "process.h"
#include "sexpr-process.h"
#include "conf-file.h"
#include "aio.h"
#include "vec.h"

#define SUBPROOFS_DISABLED 0

// Main menu array.

static const char * head_text[] =
  {
    N_("File"), N_("Edit"), N_("Proof"),
    N_("Rules"), N_("Font"), N_("Help")
  };

// Number of submenus for each menu.

static int num_subs[NUM_MENUS] = { FILE_MENU_SIZE, EDIT_MENU_SIZE, PROOF_MENU_SIZE,
				   RULES_MENU_SIZE, FONT_MENU_SIZE, HELP_MENU_SIZE };

/* Initializes an aris proof.
 *  input:
 *    none.
 *  output:
 *    a newly initialized aris proof, or NULL on error.
 */
aris_proof *
aris_proof_init ()
{
  aris_proof * ap;
  ap = (aris_proof *) calloc (1, sizeof (aris_proof));
  CHECK_ALLOC (ap, NULL);

  ap->vars = init_list ();
  if (!ap->vars)
    return NULL;

  sen_parent_init (SEN_PARENT (ap), _("GNU Aris - Untitled"),
		   640, 320, aris_proof_create_menu, SEN_PARENT_TYPE_PROOF);
  ap->boolean = the_app->boolean;

  if (the_app->boolean || SUBPROOFS_DISABLED)
    {
      GList * gl;
      GtkWidget * edit_menu, * edit_submenu;
      GtkWidget * edit_sub, * edit_end;

      gl = gtk_container_get_children (GTK_CONTAINER (SEN_PARENT (ap)->menubar));
      edit_menu = (GtkWidget *) g_list_nth_data (gl, EDIT_MENU);
      edit_submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (edit_menu));

      gl = gtk_container_get_children (GTK_CONTAINER (edit_submenu));

      edit_sub = (GtkWidget *) g_list_first (gl)->next->next->data;
      edit_end = (GtkWidget *) g_list_first (gl)->next->next->next->data;

      gtk_widget_set_sensitive (edit_sub, FALSE);
      gtk_widget_set_sensitive (edit_end, FALSE);
    }

  ap->cur_file = NULL;
  ap->edited = 0;

  ap->fin_prem = NULL;

  ap->selected = init_list ();
  if (!ap->selected)
    return NULL;

  ap->undo_stack = init_vec (sizeof (undo_info));
  if (!ap->undo_stack)
    return NULL;
  ap->undo_pt = -1;

  aris_proof_set_sb (ap, _("Ready"));

  int ret;

  ret = aris_proof_post_init (ap);
  if (ret < 0)
    return NULL;

  ap->goal = goal_init (ap);
  if (!ap->goal)
    return NULL;

  gtk_accel_map_add_entry ("<ARIS-WINDOW>/Contents", GDK_KEY_F1, 0);

  if (the_app->fonts[FONT_TYPE_CUSTOM])
    {
      aris_proof_set_font (ap, FONT_TYPE_CUSTOM);

      // Set the small font size menu item to be sensitive.

      GList * gl;
      GtkWidget * font_menu, * font_submenu;

      gl = gtk_container_get_children (GTK_CONTAINER (SEN_PARENT (ap)->menubar));
      font_menu = (GtkWidget *) g_list_nth_data (gl, FONT_MENU);
      font_submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (font_menu));

      gl = gtk_container_get_children (GTK_CONTAINER (font_submenu));

      gtk_widget_set_sensitive ((GtkWidget *) gl->data, TRUE);
    }
  else
    {
      aris_proof_set_font (ap, FONT_TYPE_SMALL);
    }

  g_signal_connect (G_OBJECT (SEN_PARENT (ap)->window), "delete-event",
		    G_CALLBACK (window_delete), (gpointer) ap);
  g_signal_connect (G_OBJECT (SEN_PARENT (ap)->window), "focus-in-event",
		    G_CALLBACK (window_focus_in), (gpointer) ap);

  gtk_widget_show_all (SEN_PARENT (ap)->window);

  return ap;
}

/* Initializes the separator and first sentence.
 *  input:
 *    ap - aris proof to finish initialization of.
 *  output:
 *    0 on success, -1 on error.
 */
int
aris_proof_post_init (aris_proof * ap)
{
  sen_data * sd;
  sentence * sen;

  // Initialize the separator
  SEN_PARENT (ap)->separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);

  sd = SEN_DATA_DEFAULT (1, 0, 0);
  if (!sd)
    return -1;

  sen = sentence_init (sd, (sen_parent *) ap, NULL);
  if (!sen)
    return -1;

  gtk_grid_attach_next_to (GTK_GRID (SEN_PARENT (ap)->container),
			   sen->panel, NULL, GTK_POS_BOTTOM, 1, 1);

  item_t * itm = ls_push_obj (SEN_PARENT (ap)->everything, sen);
  if (!itm)
    return -1;

  ap->fin_prem = SEN_PARENT (ap)->focused = SEN_PARENT (ap)->everything->head;

  gtk_grid_attach_next_to (GTK_GRID (SEN_PARENT (ap)->container),
			   SEN_PARENT (ap)->separator, NULL, GTK_POS_BOTTOM, 1, 1);

  // Clear the undo stack.
  ap->undo_pt = -1;
  while (ap->undo_stack->num_stuff > ap->undo_pt + 1)
    aris_proof_undo_stack_pop (ap);

  return 0;
}

/* Initialize an aris proof from a data proof.
 *  input:
 *    proof - a data proof from which to initialize the aris proof.
 *  output:
 *    an initialized aris proof, or NULL on error.
 */
aris_proof *
aris_proof_init_from_proof (proof_t * proof)
{
  aris_proof * ap;
  ap = (aris_proof *) calloc (1, sizeof (aris_proof));
  CHECK_ALLOC (ap, NULL);

  ap->vars = init_list ();
  if (!ap->vars)
    return NULL;

  sen_parent_init (SEN_PARENT (ap), _("GNU Aris - "),
		   640, 320, aris_proof_create_menu, SEN_PARENT_TYPE_PROOF);

  ap->boolean = proof->boolean;
  ap->cur_file = NULL;
  ap->edited = 0;
  ap->selected = init_list ();
  if (!ap->selected)
    return NULL;

  if (ap->boolean || SUBPROOFS_DISABLED)
    {
      GList * gl;
      GtkWidget * edit_menu, * edit_submenu;
      GtkWidget * edit_sub, * edit_end;

      gl = gtk_container_get_children (GTK_CONTAINER (SEN_PARENT (ap)->menubar));
      edit_menu = (GtkWidget *) g_list_nth_data (gl, EDIT_MENU);
      edit_submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (edit_menu));

      gl = gtk_container_get_children (GTK_CONTAINER (edit_submenu));

      edit_sub = (GtkWidget *) g_list_first (gl)->next->next->data;
      edit_end = (GtkWidget *) g_list_first (gl)->next->next->next->data;

      gtk_widget_set_sensitive (edit_sub, FALSE);
      gtk_widget_set_sensitive (edit_end, FALSE);
    }

  ap->fin_prem = NULL;

  SEN_PARENT (ap)->font = FONT_TYPE_SMALL;
  aris_proof_set_sb (ap, _("Ready"));

  g_signal_connect (G_OBJECT (SEN_PARENT (ap)->window), "delete-event",
		    G_CALLBACK (window_delete), (gpointer) ap);
  g_signal_connect (G_OBJECT (SEN_PARENT (ap)->window), "focus-in-event",
		    G_CALLBACK (window_focus_in), (gpointer) ap);

  ap->undo_stack = init_vec (sizeof (undo_info));
  if (!ap->undo_stack)
    return NULL;
  ap->undo_pt = -1;

  item_t * ev_itr;
  int first = 1;

  ev_itr = proof->everything->head;
  for (; ev_itr != NULL; ev_itr = ev_itr->next)
    {
      sen_data * sd;
      sentence * sen;

      sd = ev_itr->value;

      if (first == 1)
	{
	  SEN_PARENT (ap)->separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	  item_t * itm;

	  itm = sen_parent_ins_sentence ((sen_parent *) ap, sd, NULL, 0);

	  ap->fin_prem = SEN_PARENT (ap)->focused = SEN_PARENT (ap)->everything->head;

	  gtk_grid_attach_next_to (GTK_GRID (SEN_PARENT (ap)->container),
				   SEN_PARENT (ap)->separator, NULL, GTK_POS_BOTTOM,
				   1, 1);
	  first = 0;
	}
      else
	{
	  sen = aris_proof_create_sentence (ap, sd, 0);
	  if (!sen)
	    return NULL;

	  sentence_set_bg (sen, BG_COLOR_DEFAULT);
	}
    }

  ap->goal = goal_init_from_list (ap, proof->goals);
  if (!ap->goal)
    return NULL;

  gtk_widget_show_all (SEN_PARENT (ap)->window);
  gtk_widget_grab_focus (((sentence *) SEN_PARENT (ap)->everything->head->value)->entry);

  return ap;
}

/* Destroy an aris proof.
 *  input:
 *    ap - the aris proof to destroy.
 *  output:
 *    none.
 */
void
aris_proof_destroy (aris_proof * ap)
{
  if (ap->yanked)
    {
      item_t * yank_itr;
      for (yank_itr = ap->yanked->head; yank_itr; yank_itr = yank_itr->next)
	{
	  sen_data * y_sd;
	  y_sd = yank_itr->value;
	  sen_data_destroy (y_sd);
	}

      destroy_list (ap->yanked);
      ap->yanked = NULL;
    }

  ap->fin_prem = NULL;
  ap->sb_text = NULL;

  item_t * var_itr, * n_var_itr;
  var_itr = ap->vars->head;

  for (var_itr = ap->vars->head; var_itr != NULL; var_itr = n_var_itr)
    {
      n_var_itr = var_itr->next;
      if (((variable *) var_itr->value)->text)
	free (((variable *) var_itr->value)->text);

      free (var_itr->value);
      var_itr->value = var_itr->next = var_itr->prev = NULL;
      free (var_itr);
    }

  free (ap->vars);
  ap->vars = NULL;

  goal_destroy (ap->goal);
  if (ap->selected)
    {
      destroy_list (ap->selected);
      ap->selected = NULL;
    }

  sen_parent_destroy ((sen_parent *) SEN_PARENT (ap));
}

/* Creates the menu of an aris proof - used in sen_parent_init.
 *  input:
 *    ap - the aris proof, or sentence parent to initialize the menu of.
 *  output:
 *    none.
 */
void
aris_proof_create_menu (sen_parent * ap)
{
  GtkWidget * submenu, * menu;
  int i, j;
  int got_radio = 0;

  i = 0;

  conf_obj * main_menus_menu[] = {
    (conf_obj[]) {
      main_menu_conf[CONF_MENU_NEW],
      main_menu_conf[CONF_MENU_OPEN],
      menu_separator,
      main_menu_conf[CONF_MENU_SAVE],
      main_menu_conf[CONF_MENU_SAVE_AS],
      menu_separator,
      main_menu_conf[CONF_MENU_CLOSE],
      main_menu_conf[CONF_MENU_QUIT]
    },
    (conf_obj[]) {
      main_menu_conf[CONF_MENU_ADD_PREM],
      main_menu_conf[CONF_MENU_ADD_CONC],
      main_menu_conf[CONF_MENU_ADD_SUB],
      main_menu_conf[CONF_MENU_END_SUB],
      menu_separator,
      main_menu_conf[CONF_MENU_UNDO],
      main_menu_conf[CONF_MENU_REDO],
      menu_separator,
      main_menu_conf[CONF_MENU_COPY],
      main_menu_conf[CONF_MENU_KILL],
      main_menu_conf[CONF_MENU_INSERT]
    },
    (conf_obj[]) {
      main_menu_conf[CONF_MENU_EVAL_LINE],
      main_menu_conf[CONF_MENU_EVAL_PROOF],
      menu_separator,
      main_menu_conf[CONF_MENU_GOAL],
      main_menu_conf[CONF_MENU_BOOLEAN],
      main_menu_conf[CONF_MENU_IMPORT]
    },
    (conf_obj[]) {
      main_menu_conf[CONF_MENU_TOGGLE_RULES],
      menu_separator
    },
    (conf_obj[]) {
      main_menu_conf[CONF_MENU_SMALL],
      main_menu_conf[CONF_MENU_MEDIUM],
      main_menu_conf[CONF_MENU_LARGE],
      main_menu_conf[CONF_MENU_CUSTOM],
    },
    (conf_obj[]) {
      main_menu_conf[CONF_MENU_CONTENTS],
      main_menu_conf[CONF_MENU_ABOUT],
    }
  };

  ap->menubar = gtk_menu_bar_new ();

  for (i = 0; i < NUM_MENUS; i++)
    {
      submenu = gtk_menu_new ();
      menu = gtk_menu_item_new_with_label (head_text[i]);
      gtk_menu_set_accel_group (GTK_MENU (submenu), ap->accel);
      conf_obj * cur_data = main_menus_menu[i];
      for (j = 0; j < num_subs[i]; j++)
	{
	  construct_menu_item (cur_data[j], G_CALLBACK (menu_activate),
			       submenu, &got_radio);
	}
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), submenu);
      gtk_menu_shell_append (GTK_MENU_SHELL (ap->menubar), menu);
    }
}

/* Sets an aris proof to be changed or not changed.
 *  input:
 *    ap - the aris proof to set the changed status of.
 *    changed - 0 if being changed, 1 if being saved.
 *  output:
 *    0 on success, -1 on error.
 */
int
aris_proof_set_changed (aris_proof * ap, int changed, undo_info ui)
{
  char * new_title;
  const char * title = gtk_window_get_title (GTK_WINDOW (SEN_PARENT (ap)->window));
  int pos = 0;

  new_title = (char *) calloc (strlen (title) + 4, sizeof (char));
  CHECK_ALLOC (new_title, -1);
  pos = sprintf (new_title, "%s", title);
  if (changed && !ap->edited)
    {
      // The data has been modified, so add an asterisk to the title.
      sprintf (new_title + pos, "*");
      ap->edited = 1;
    }
  if (!changed && ap->edited)
    {
      // We are saving the data, so remove the asterisk.
      new_title[pos - 1] = '\0';
      ap->edited = 0;
    }
  gtk_window_set_title (GTK_WINDOW (SEN_PARENT (ap)->window), (const char *) new_title);
  free (new_title);

  if (ui.type != -1)
    aris_proof_undo_stack_push (ap, ui);

  return 0;
}

/* Adjusts the line number of each sentence in an aris proof.
 *  input:
 *    ap - the aris proof to adjust the line numbers of.
 *    itm - the iterator in ap->everything to begin at.
 *    mod - modifier for each sentence, either 1 or -1.
 *  output:
 *    0 on success, -1 on error.
 */
int
aris_proof_adjust_lines (aris_proof * ap, item_t * itm, int mod)
{
  int line_mod = 1;
  line_mod *= mod;

  int cur_line = ((sentence *) itm->value)->line_num;
  item_t * ev_itr;

  for (ev_itr = itm->next; ev_itr; ev_itr = ev_itr->next)
    {
      int new_line_no, ret;
      sentence * ev_sen;

      ev_sen = ev_itr->value;
      if (!ev_sen)
	exit (EXIT_FAILURE);

      new_line_no = ev_sen->line_num + line_mod;
      ret = sentence_set_line_no (ev_sen, new_line_no);
      if (ret == -1)
	return -1;
    }

  return 0;
}

/* Copies an aris proof into a data proof.
 *  input:
 *    ap - the aris proof to copy in.
 *  output:
 *    a data proof initialized from the aris proof.
 */
proof_t *
aris_proof_to_proof (aris_proof * ap)
{
  proof_t * proof;
  sen_data * sd;
  item_t * ev_itr, * g_itr, * itm;
  sentence * sen;

  proof = proof_init ();
  if (!proof)
    return NULL;

  for (g_itr = SEN_PARENT (ap->goal)->everything->head; g_itr; g_itr = g_itr->next)
    {
      sen = g_itr->value;
      unsigned char * entry_text = sen->text;

      itm = ls_ins_obj (proof->goals, entry_text, proof->goals->tail);
      if (!itm)
	return NULL;
    }

  for (ev_itr = SEN_PARENT (ap)->everything->head; ev_itr; ev_itr = ev_itr->next)
    {
      sen = ev_itr->value;
      sd = sentence_copy_to_data (sen);
      if (!sd)
	return NULL;

      itm = ls_ins_obj (proof->everything, sd, proof->everything->tail);
      if (!itm)
	return NULL;
    }

  proof->boolean = ap->boolean;

  return proof;
}

/* Creates a sentence for an aris proof.
 *  input:
 *    ap - the aris proof to create a sentence for.
 *    sd - the sentence data to initialize the new sentence from.
 *    undo - whether or not to create undo information
 *  output:
 *    the newly created sentence, or NULL on error.
 */
sentence *
aris_proof_create_sentence (aris_proof * ap, sen_data * sd, int undo)
{
  sentence * sen;
  item_t * itm, * fcs;
  item_t * foc_1, * foc_2;
  int new_order;

  // Is the new sentence a premise?

  if (sd->premise)
    {
      foc_1 = SEN_PARENT (ap)->focused;
      foc_2 = ap->fin_prem;
    }
  else
    {
      foc_1 = ap->fin_prem;
      foc_2 = SEN_PARENT (ap)->focused;
    }

  fcs = (SENTENCE (SEN_PARENT (ap)->focused->value)->premise) ? foc_1 : foc_2;

  if (!fcs)
    {
      fprintf (stderr, "Data corruption.\n");
      exit (EXIT_FAILURE);
    }

  new_order = SENTENCE (fcs->value)->line_num;

  // Increment this, since non-premises are attached at their line number + 1;
  if (!sd->premise)  new_order++;

  if (sd->depth == -1)
    {
      sd->depth = ((sentence *) fcs->value)->depth - 1;
    }
  else if (sd->depth == DEPTH_DEFAULT)
    {
      sd->depth = ((sentence *) fcs->value)->depth;
      if (sd->subproof)
	sd->depth++;
    }

  itm = sen_parent_ins_sentence ((sen_parent *) ap, sd, fcs, new_order);
  if (!itm)
    return NULL;

  if (sd->premise && fcs == ap->fin_prem)
    ap->fin_prem = itm;

  int ret;
  ret = aris_proof_adjust_lines (ap, itm, 1);
  if (ret < 0)
    return NULL;

  sen = itm->value;

  undo_info ui;
  ui.type = -1;
  if (undo)
    ui = undo_info_init_one (ap, sen, UIT_ADD_SEN);

  ret = aris_proof_set_changed (ap, 1, ui);
  if (ret < 0)
    return NULL;

  if (sd->rule == RULE_LM && sd->file)
    {
      int file_len, alloc_size;
      GtkWidget * menu_item, * menu, * submenu;
      GList * gl;
      char * label;

      file_len = strlen (sd->file);
      alloc_size = file_len + 4 + 1 + (int) log10 (sen->line_num);
      label = (char *) calloc (alloc_size, sizeof (char));
      CHECK_ALLOC (label, NULL);

      sprintf (label, "%i - %s",
	       sen->line_num, sd->file);
      menu_item = gtk_menu_item_new_with_label (label);

      gl = gtk_container_get_children (GTK_CONTAINER (SEN_PARENT (ap)->menubar));
      menu = g_list_nth_data (gl, RULES_MENU);
      submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu));
      gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menu_item);
      gtk_widget_show (menu_item);
    }
  
  return sen;
}

/* Convenience function to create a new premise for an aris proof.
 *  input:
 *    ap - the aris proof to create a premise for.
 *  output:
 *    the newly created premise, or NULL on error.
 */
sentence *
aris_proof_create_new_prem (aris_proof * ap)
{
  sentence * sen;
  sen_data * sd;

  sd = SEN_DATA_DEFAULT (1, 0, DEPTH_DEFAULT);
  if (!sd)
    return NULL;

  sen = aris_proof_create_sentence (ap, sd, 1);
  if (!sen)
    return NULL;

  sen_data_destroy (sd);

  return sen;
}

/* Convenience function to create a new conclusion for an aris proof.
 *  input:
 *    ap - the aris proof to create a conclusion for.
 *  output:
 *    the newly created conclusion, or NULL on error.
 */
sentence *
aris_proof_create_new_conc (aris_proof * ap)
{
  sentence * sen;
  sen_data * sd;

  sd = SEN_DATA_DEFAULT (0, 0, DEPTH_DEFAULT);
  if (!sd)
    return NULL;

  sen = aris_proof_create_sentence (ap, sd, 1);
  if (!sen)
    return NULL;

  sen_data_destroy (sd);

  return sen;
}

/* Convenience function to create a new subproof for an aris proof.
 *  input:
 *    ap - the aris proof to create a subproof for.
 *  output:
 *    the newly created subproof, or NULL on error.
 */
sentence *
aris_proof_create_new_sub (aris_proof * ap)
{
  sentence * sen;
  sen_data * sd;

  sd = SEN_DATA_DEFAULT (0, 1, DEPTH_DEFAULT);
  if (!sd)
    return NULL;

  sen =  aris_proof_create_sentence (ap, sd, 1);
  if (!sen)
    return NULL;

  sen_data_destroy (sd);

  return sen;
}

/* Convenience function to end a subproof for an aris proof.
 *  input:
 *    ap - the aris proof to end a subproof for.
 *  output:
 *    the newly createed sentence, or NULL on error.
 */
sentence *
aris_proof_end_sub (aris_proof * ap)
{
  if (((sentence *) SEN_PARENT (ap)->focused->value)->depth == 0)
    return NULL;

  sentence * sen;
  sen_data * sd;

  sd = SEN_DATA_DEFAULT (0, 0, -1);
  if (!sd)
    return NULL;

  sen =  aris_proof_create_sentence (ap, sd, 1);
  if (!sen)
    return NULL;

  sen_data_destroy (sd);

  return sen;
}

/* Removes a sentence from an aris proof.
 *  input:
 *    ap - the aris proof from which to remove a sentence.
 *    sen - the sentence to remove.
 *  output:
 *    0 on success, -1 on error.
 */
int
aris_proof_remove_sentence (aris_proof * ap, sentence * sen)
{
  item_t * target = sen_parent_rem_sentence ((sen_parent *) ap, sen);
  if (!target)
    return -1;

  if (ap->fin_prem == target)
    ap->fin_prem = SEN_PARENT (ap)->focused;

  int ret = aris_proof_adjust_lines (ap, target, -1);
  if (ret < 0)
    return -1;

  return 0;
}

/* Sets the font of an aris proof.
 *  input:
 *    ap - the aris proof to set the font of.
 *    font - the index in the_app->fonts of the font to set.
 *  output:
 *    none.
 */
void
aris_proof_set_font (aris_proof * ap, int font)
{
  sen_parent_set_font (SEN_PARENT (ap), font);
  sen_parent_set_font ((sen_parent *) ap->goal, font);
}

/* Sets the status bar text of an aris proof.
 *  input:
 *    ap - the aris proof to set the status bar text of.
 *    text - the text to set the status bar text to.
 *  output:
 *    none.
 */
void
aris_proof_set_sb (aris_proof * ap, char * sbtext)
{
  sen_parent_set_sb (SEN_PARENT (ap), sbtext);
  ap->sb_text = sbtext;
}

/* Sets the file name of an aris proof.
 *  input:
 *    ap - the aris proof to set the file name of.
 *    filename - the file name to set the aris proof's file name to.
 *  output:
 *    0 on success, -1 on error.
 */
int
aris_proof_set_filename (aris_proof * ap, const char * filename)
{
  char * new_title, * base_name;

  new_title = (char *) calloc (strlen (filename) + 12, sizeof (char));
  CHECK_ALLOC (new_title, -1);
  ap->cur_file = (char *) filename;

  GFile * file = g_file_new_for_path (filename);
  base_name = g_file_get_basename (file);
  sprintf (new_title, "GNU Aris - %s", base_name);
  free (base_name);

  gtk_window_set_title (GTK_WINDOW (SEN_PARENT (ap)->window), new_title);
  free (new_title);

  int ret;
  ret = goal_update_title (ap->goal);
  if (ret == -1)
    return -1;

  return 0;
}

/* Copys the selected line(s) from a proof.
 *  input:
 *    ap - The aris proof from which sentences are being copied.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
aris_proof_copy (aris_proof * ap)
{
  if (ap->yanked)
    {
      item_t * yank_itr;
      for (yank_itr = ap->yanked->head; yank_itr; yank_itr = yank_itr->next)
	{
	  sen_data * y_sd;
	  y_sd = yank_itr->value;
	  sen_data_destroy (y_sd);
	}
      ls_clear (ap->yanked);
    }
  else
    {
      ap->yanked = init_list ();
      if (!ap->yanked)
	return -1;
    }

  item_t * sel_itr;
  int is_subproof = 0;
  int org_depth = -1;
 
  if (ap->selected->num_stuff == 0)
    {
      sel_itr = SEN_PARENT (ap)->focused;
      if (((sentence *) sel_itr->value)->subproof)
	{
	  is_subproof = 1;
	  org_depth = ((sentence *) sel_itr->value)->depth;
	}
    }
  else
    {
      sel_itr = ap->selected->head;
    }

  for (; sel_itr; sel_itr = sel_itr->next)
    {
      sentence * sen;
      sen_data * sd;
      item_t * ret_chk;

      sen = sel_itr->value;
      if (sen->depth < org_depth)
	break;

      sd = sentence_copy_to_data (sen);
      if (!sd)
	return -1;

      if (sel_itr->prev && sen->depth < ((sentence *) sel_itr->prev->value)->depth)
	sd->depth = -1;
      else
	sd->depth = DEPTH_DEFAULT;

      ret_chk = ls_push_obj (ap->yanked, sd);
      if (!ret_chk)
	return -1;

      if (ap->selected->num_stuff == 0 && (!is_subproof || (sen->depth < org_depth)))
	break;
    }

  return 0;
}


/* Kills the selected line(s) from an aris proof.
 *  input:
 *    ap - The aris proof from which sentences are being copied.
 *  output:
 *    0 on success, -1 on memory error, 1 if the sentence is the first.
 */
int
aris_proof_kill (aris_proof * ap)
{
  int ret_chk;
  ret_chk = aris_proof_copy (ap);
  if (ret_chk == -1)
    return -1;

  list_t * sens;
  item_t * sel_itr, * start_itr;
  int is_subproof = 0;
  int org_depth = -1;

  if (ap->selected->num_stuff == 0)
    {
      sel_itr = SEN_PARENT (ap)->focused;
      if (SENTENCE (sel_itr->value)->subproof)
	{
	  is_subproof = 1;
	  org_depth = SENTENCE (sel_itr->value)->depth;
	}
    }
  else
    {
      sel_itr = ap->selected->head;
    }

  for (start_itr = sel_itr; start_itr; start_itr = start_itr->next)
    {
      sentence * sen;
      sen = start_itr->value;

      if (sen->line_num == 1)
	return 1;
    }

  undo_info ui;
  list_t * ls;
  ls = init_list ();
  if (!ls)
    return -1;

  for (; sel_itr; sel_itr = sel_itr->next)
    {
      sentence * sen;
      int sen_depth;
      sen_data * sd;

      sen = sel_itr->value;
      sd = sentence_copy_to_data (sen);
      ls_push_obj (ls, sd);

      sen_depth = sen->depth;
      if (sen_depth < org_depth)
	break;

      ret_chk = aris_proof_remove_sentence (ap, sen);
      if (ret_chk == -1)
	return -1;

      if (ap->selected->num_stuff == 0 && (!is_subproof || (sen_depth < org_depth)))
	break;
    }

  ui = undo_info_init (ap, ls, UIT_REM_SEN);
  if (ui.type == -1)
    return -1;

  int ret;
  ret = aris_proof_set_changed (ap, 1, ui);
  if (ret < 0)
    return -1;

  return 0;
}

/* Yanks any copied or killed lines to an aris proof.
 *  input:
 *    ap - The aris proof to which the lines are being yanked.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
aris_proof_yank (aris_proof * ap)
{
  if (!ap->yanked)
    return 0;

  list_t * ls;
  ls = init_list ();
  if (!ls)
    return -1;

  item_t * yank_itr;
  int ret;
  for (yank_itr = ap->yanked->head; yank_itr; yank_itr = yank_itr->next)
    {
      sentence * sen;
      sen_data * sd;
      sd = yank_itr->value;
      ls_push_obj (ls, sd);

      sen = aris_proof_create_sentence (ap, sd, 0);
      if (!sen)
	return -1;
    }

  undo_info ui;
  ui = undo_info_init (ap, ls, UIT_ADD_SEN);
  if (ui.type == -1)
    return -1;

  ret = aris_proof_set_changed (ap, 1, ui);
  if (ret < 0)
    return -1;

  return 0;
}

/* Attempts to toggle boolean mode for an aris proof.
 *  input:
 *    ap - The aris proof for which boolean mode is being toggled.
 *  output:
 *    0 on success, -1 on error.
 */
int
aris_proof_toggle_boolean_mode (aris_proof * ap)
{
  int new_bool;
  new_bool = (ap->boolean) ? 0 : 1;

  // Check each line in the proof, to confirm that none of the lines use
  // unallowed rules.
  item_t * ev_itr;

  for (ev_itr = SEN_PARENT (ap)->everything->head; ev_itr; ev_itr = ev_itr->next)
    {
      sentence * ev_sen;
      int sen_check;

      ev_sen = ev_itr->value;

      if (ev_sen->subproof)
	{
	  aris_proof_set_sb (ap, _("Subproofs may not be used in boolean mode."));
	  return -1;
	}

      if (ev_sen->rule == -1)
	continue;
      sen_check = sentence_check_boolean_rule (ev_sen, new_bool);

      if (!sen_check)
	{
	  if (new_bool)
	    {
	      aris_proof_set_sb (ap, _("No prohibited rules must be used for boolean mode."));
	      return -1;
	    }
	}
    }

  ap->boolean = new_bool;
  rules_table_set_boolean_mode (the_app->rt, new_bool);

  if (new_bool)
    aris_proof_set_sb (ap, _("Boolean mode enabled."));
  else
    aris_proof_set_sb (ap, _("Boolean mode disabled."));

  /*
  if (!ap->edited)
    aris_proof_set_changed (ap, 0);
  */

  return 0;
}

/* Imports a proof into the current proof.
 *  input:
 *    ap - The current proof.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
aris_proof_import_proof (aris_proof * ap)
{
  GtkFileFilter * file_filter;
  file_filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (file_filter, "Aris Files");
  gtk_file_filter_add_pattern (file_filter, "*.tle");

  GtkWidget * file_chooser;
  file_chooser =
    gtk_file_chooser_dialog_new (_("Select a file to Open..."),
				 GTK_WINDOW (SEN_PARENT (ap)->window),
				 GTK_FILE_CHOOSER_ACTION_OPEN,
				 "_Cancel", GTK_RESPONSE_CANCEL,
				 "_Open", GTK_RESPONSE_ACCEPT,
				 NULL);
  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (file_chooser), FALSE);
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (file_chooser), file_filter);

  int ret;
  char * filename;

  ret = gtk_dialog_run (GTK_DIALOG (file_chooser));
  if (ret != GTK_RESPONSE_ACCEPT)
    {
      gtk_widget_destroy (file_chooser);
      return 0;
    }

  filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));
  gtk_widget_destroy (file_chooser);

  proof_t * proof;

  proof = aio_open (filename);
  if (!proof)
    {
      gtk_widget_destroy (file_chooser);
      return -1;
    }

  item_t * ev_itr, * pf_itr, * ev_conc = NULL;
  int ref_num = 0;
  short * refs;

  refs = (short *) calloc (proof->everything->num_stuff, sizeof (int));
  CHECK_ALLOC (refs, -1);

  for (pf_itr = proof->everything->head; pf_itr;
       pf_itr = pf_itr->next)
    {
      sen_data * sd;
      char * pf_text;

      sd = pf_itr->value;
      if (!sd->premise)
	break;

      pf_text = sd->text;

      for (ev_itr = SEN_PARENT (ap)->everything->head; ev_itr; ev_itr = ev_itr->next)
	{
	  char * ev_text;
	  sentence * ev_sen;

	  ev_sen = ev_itr->value;
	  if (!ev_sen->premise)
	    {
	      if (!ev_conc)
		ev_conc = ev_itr;
	      break;
	    }

	  ev_text = ev_sen->text;

	  if (!strcmp (ev_text, pf_text))
	    {
	      refs[ref_num++] = (short) ev_sen->line_num;
	      break;
	    }
	}

      if (!ev_itr || !((sentence *) ev_itr->value)->premise)
	{
	  sentence * sen_chk;
	  sen_chk = aris_proof_create_sentence (ap, sd, 1);
	  if (!sen_chk)
	    return -1;
	  refs[ref_num++] = (short) sen_chk->line_num;
	}
    }

  refs[ref_num] = -1;

  if (!ev_conc)
    {
      for (ev_itr = SEN_PARENT(ap)->everything->head; ev_itr;
	   ev_itr = ev_itr->next)
	{
	  sentence * ev_sen;

	  ev_sen = ev_itr->value;
	  if (!ev_sen->premise)
	    {
	      if (!ev_conc)
		ev_conc = ev_itr;
	      break;
	    }
	}
    }

  for (pf_itr = proof->goals->head; pf_itr; pf_itr = pf_itr->next)
    {
      unsigned char * pf_text;
      pf_text = pf_itr->value;

      for (ev_itr = ev_conc; ev_itr; ev_itr = ev_itr->next)
	{
	  sentence * ev_sen;
	  unsigned char * ev_text;
	  ev_sen = ev_itr->value;
	  ev_text = ev_sen->text;

	  if (!strcmp (pf_text, ev_text))
	    break;
	}

      if (!ev_itr)
	{
	  sen_data * sd;
	  sentence * sen_chk;

	  sd = sen_data_init (-1, RULE_LM, pf_text, refs,
			      0, filename, 0, DEPTH_DEFAULT, NULL);
	  if (!sd)
	    return -1;

	  sen_chk = aris_proof_create_sentence (ap, sd, 1);
	  if (!sen_chk)
	    return -1;
	}
    }

  free (refs);
  proof_destroy (proof);
  return 0;
}

/* Initialize and undo information object.
 *  input:
 *    ap - the aris proof object to which this undo information belongs.
 *    sens - a list of sentences that have been modified.
 *    type - the type of action that this was generated by.
 *  output:
 *    A newly initialized undo information object.
 */
undo_info
undo_info_init (aris_proof * ap, list_t * sens, int type)
{
  undo_info ret;
  ret.type = -1;

  item_t * it, * nit;
  ret.ls = init_list ();
  for (it = sens->head; it;)
    {
      nit = it->next;
      sen_data * sd = it->value;
      ls_push_obj (ret.ls, sd);
      free (it);
      it = nit;
    }
  free (sens);

  ret.type = type;
  ret.stamp = time (NULL);

  return ret;
}

/* Initialize an undo information object from a single sentence.
 *  input:
 *    ap - the aris proof to which this object belongs.
 *    sen - the sentence that was modified.
 *    type - the type of action that generated this.
 *  output:
 *    A newly initialized undo information object.
 */
undo_info
undo_info_init_one (aris_proof * ap, sentence * sen, int type)
{
  undo_info ret;
  ret.type = -1;

  list_t * sens;

  sens = init_list ();

  sen_data * sd;
  unsigned char * text;
  text = strdup (sen->text);
  if (!text)
    return ret;

  sd = sentence_copy_to_data (sen);
  if (!sd)
    return ret;

  if (sd->text)
    free (sd->text);

  sd->text = text;

  ls_push_obj (sens, sd);

  ret = undo_info_init (ap, sens, type);

  return ret;
}

/* Destroy an undo information object.
 *  input:
 *    ui - the undo information object to destroy.
 *  output:
 *    None.
 */
void
undo_info_destroy (undo_info ui)
{
  item_t * it;
  sen_data * sd;
  if (ui.ls)
    {
      for (it = ui.ls->head; it;)
	{
	  item_t * next = it->next;
	  sd = (sen_data *) it->value;
	  sen_data_destroy (sd);
	  free (it);
	  it = next;
	}

      free (ui.ls);
    }
  ui.ls = NULL;
}

/* Push an undo informatoin object onto an undo stack.
 *  input:
 *    ap - the aris proof that owns the undo stack that to which an object is being pushed.
 *    ui - the undo information object to push onto the stack.
 *  output:
 *    0 on success, 1 on minor error, -1 on memory error.
 */
int
aris_proof_undo_stack_push (aris_proof * ap, undo_info ui)
{
  if (ui.type == -1)
    return 1;

  if (ui.type == UIT_MOD_TEXT
      && ap->undo_stack->num_stuff > 0
      && ap->undo_pt >= 0)
    {
      undo_info * last;
      last = vec_nth (ap->undo_stack, ap->undo_pt);
      if (last->type == UIT_MOD_TEXT)
	{
	  sen_data * lsd = (sen_data *) last->ls->head->value;
	  sen_data * usd = (sen_data *) ui.ls->head->value;
	  if (ui.stamp - last->stamp <= UNDO_INT
	      && lsd->line_num == usd->line_num)
	    {
	      last->stamp = ui.stamp;
	      undo_info_destroy (ui);
	      return 0;
	    }
	}
    }

  int rc;
  while (ap->undo_stack->num_stuff > ap->undo_pt + 1)
    rc = aris_proof_undo_stack_pop (ap);

  rc = vec_add_obj (ap->undo_stack, &ui);
  if (rc == -1)
    return -1;

  ap->undo_pt++;

  return 0;
}

/* Pop an object off of an undo stack.
 *  input:
 *    ap - the aris proof off of which to pop an object.
 *  output:
 *    0 on success.
 */
int
aris_proof_undo_stack_pop (aris_proof * ap)
{
  if (ap->undo_stack->num_stuff == 0)
    return 0;

  undo_info * ui;
  ui = vec_nth (ap->undo_stack, ap->undo_stack->num_stuff - 1);

  undo_info_destroy (*ui);

  vec_pop_obj (ap->undo_stack);

  return 0;
}

/* A macro to assist with undo/redo.
 */

#define UNDO_SEN(u,a,i,s,d) if (u) {			\
    SEN_PARENT(a)->focused = i;				\
    aris_proof_remove_sentence (a,s);			\
  }							\
  else {						\
    if (i) { i = i->prev;}				\
    else { i = SEN_PARENT (a)->everything->tail; }	\
    SEN_PARENT (a)->focused = i;			\
    aris_proof_create_sentence (a, d, 0);		\
  }

/* Undo or redo an action from an aris proof.
 *  input:
 *    ap - the aris proof from which to undo or redo an action.
 *    undo - 1 if this is an undo operation, 0 if it is a redo.
 *  outpu:
 *    0 on success, 1 on minor error, -1 on memory error.
 */
int
aris_proof_undo (aris_proof * ap, int undo)
{
  undo_info * ui;
  int pt;

  if (undo)
    {
      // If undo_pt < 0, then it is pointing past the bottom of the
      // stack, and therefore there is nothing to undo.
      if (ap->undo_pt < 0)
	return 1;
      pt = ap->undo_pt;
    }
  else
    {
      // Since the undo_pt is a position, if it is the last one item
      // in the stack, then there isn't anything to redo.
      if (ap->undo_pt + 1 == ap->undo_stack->num_stuff)
	return 1;
      pt = ap->undo_pt + 1;
    }

  ui = vec_nth (ap->undo_stack, pt);

  ap->undo_pt += (undo) ? -1 : 1;

  if (!ui
      || ui->type == UIT_ADD_GOAL
      || ui->type == UIT_REM_GOAL
      || ui->type == -1)
    return 1;

  item_t * it;

  for (it = ui->ls->head; it; it = it->next)
    {
      sen_data * sd = (sen_data *) it->value;
      int ln;
      item_t * itm;
      sentence * sen;
      GtkTextBuffer * buffer;

      for (itm = SEN_PARENT(ap)->everything->head; itm; itm = itm->next)
	{
	  sen = (sentence *) itm->value;
	  if (sen->line_num >= sd->line_num)
	    break;
	}

      switch (ui->type)
	{
	case UIT_MOD_TEXT:
	  // Goto line ui->sd->line_num;
	  // Set text to ui->sd->text;

	  SEN_PARENT(ap)->undo = 1;
	  if (sen->text)
	    free (sen->text);
	  sen->text = strdup (sd->text);

	  if (!sen->text)
	    return -1;

	  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sen->entry));
	  gtk_text_buffer_set_text (buffer, "", -1);
	  sentence_paste_text (sen);
	  SEN_PARENT(ap)->undo = 0;

	  break;

	case UIT_ADD_SEN:
	  // Remove the sentence.
	  UNDO_SEN (undo, ap, itm, sen, sd);
	  break;

	case UIT_REM_SEN:
	  UNDO_SEN (!undo, ap, itm, sen, sd);
	  // Add the sentence back in.
	  // Move itm to the sentence just before the one to insert.
	  // In other words, the sentence with line_num == sd->line_num - 1;
	  break;
	}
    }

  return 0;
}

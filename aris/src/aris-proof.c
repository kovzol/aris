/*  Functions for handling the aris proof guis.

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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

#define SUBPROOFS_DISABLED 0

/* Menu Data Arrays */

enum MENU_ORDER {
  FILE_MENU = 0,
  EDIT_MENU,
  PROOF_MENU,
  RULES_MENU,
  FONT_MENU,
  HELP_MENU,
  NUM_MENUS
};

enum MENU_SIZE {
  FILE_MENU_SIZE = 8,
  EDIT_MENU_SIZE = 8,
  PROOF_MENU_SIZE = 5,
  RULES_MENU_SIZE = 2,
  FONT_MENU_SIZE = 4,
  HELP_MENU_SIZE = 2
};

// Main menu array.

static mid_t head_text[NUM_MENUS] =
  {
    {N_("File"), NULL, NULL, -1, -1, PARENT, -1},
    {N_("Edit"), NULL, NULL, -1, -1, PARENT, -1},
    {N_("Proof"), NULL, NULL, -1, -1, PARENT, -1},
    {N_("Rules"), NULL, NULL, -1, -1, PARENT, -1},
    {N_("Font"), NULL, NULL, -1, -1, PARENT, -1},
    {N_("Help"), NULL, NULL, -1, -1, PARENT, -1}
  };

// Number of submenus for each menu.

static int num_subs[NUM_MENUS] = { FILE_MENU_SIZE, EDIT_MENU_SIZE, PROOF_MENU_SIZE,
				   RULES_MENU_SIZE, FONT_MENU_SIZE, HELP_MENU_SIZE };

// The file menu array.

static mid_t file_text[FILE_MENU_SIZE] =
  {
    {N_("New"), N_("Begin a new proof."), GTK_STOCK_NEW, GDK_KEY_n, GDK_CONTROL_MASK, STANDARD, MENU_NEW},
    {N_("Open"), N_("Open a proof."), GTK_STOCK_OPEN, GDK_KEY_o, GDK_CONTROL_MASK, STANDARD, MENU_OPEN},
    {NULL, NULL, NULL, -1, -1, SEPARATOR, -1},
    {N_("Save"), N_("Save the current proof."), GTK_STOCK_SAVE, GDK_KEY_s, GDK_CONTROL_MASK, STANDARD, MENU_SAVE},
    {N_("Save As"), N_("Save this proof under a different name."), GTK_STOCK_SAVE_AS, GDK_KEY_s, GDK_CONTROL_MASK | GDK_SHIFT_MASK, STANDARD, MENU_SAVE_AS },
    {NULL, NULL, NULL, -1, -1, SEPARATOR, -1},
    {N_("Close"), N_("Close the current proof."), GTK_STOCK_CLOSE, GDK_KEY_w, GDK_CONTROL_MASK, STANDARD, MENU_CLOSE},
    {N_("Quit"), N_("Quit GNU Aris."), GTK_STOCK_QUIT, GDK_KEY_q, GDK_CONTROL_MASK, STANDARD, MENU_QUIT}
  };

// The edit menu array.

static mid_t edit_text[EDIT_MENU_SIZE] =
  {
    {N_("Add Premise"), N_("Add a new premise to the current proof."), GTK_STOCK_YES, GDK_KEY_p, GDK_CONTROL_MASK, STANDARD, MENU_ADD_PREM},
    {N_("Add Conclusion"), N_("Add a new conclusion to the current proof."), GTK_STOCK_NO, GDK_KEY_j, GDK_CONTROL_MASK, STANDARD, MENU_ADD_CONC},
    {N_("Add Subproof"), N_("Add a new subproof to the current proof."), GTK_STOCK_MEDIA_NEXT, GDK_KEY_b, GDK_CONTROL_MASK, STANDARD, MENU_ADD_SUB},
    {N_("End Subproof"), N_("End the current subproof."), GTK_STOCK_MEDIA_PREVIOUS, GDK_KEY_d, GDK_CONTROL_MASK, STANDARD, MENU_END_SUB},
    {NULL, NULL, NULL, -1, -1, SEPARATOR, -1},
    {N_("Copy Line"), N_("Copy the current line in the current proof."), GTK_STOCK_COPY, GDK_KEY_g, GDK_CONTROL_MASK, STANDARD, MENU_COPY},
    {N_("Kill Line"), N_("Kill (Cut) the current line in the current proof."), GTK_STOCK_CUT, GDK_KEY_k, GDK_CONTROL_MASK, STANDARD, MENU_KILL},
    {N_("Insert Line"), N_("Insert a copied/killed line after the current line in the current proof."), GTK_STOCK_PASTE, GDK_KEY_i, GDK_CONTROL_MASK, STANDARD, MENU_INSERT}
  };

// The proof menu array.

static mid_t proof_text[PROOF_MENU_SIZE] =
  {
    {N_("Evaluate Line"), N_("Evaluate the current line in the current proof."), GTK_STOCK_EXECUTE, GDK_KEY_e, GDK_CONTROL_MASK, STANDARD, MENU_EVAL_LINE},
    {N_("Evaluate Proof"), N_("Evaluate the current proof."), GTK_STOCK_SELECT_ALL, GDK_KEY_f, GDK_CONTROL_MASK, STANDARD, MENU_EVAL_PROOF},
    {NULL, NULL, NULL, -1, -1, SEPARATOR, -1},
    {N_("Goal..."), N_("Check/Modify the current goal(s) for the current proof."), GTK_STOCK_INDEX, GDK_KEY_l, GDK_CONTROL_MASK,  STANDARD, MENU_GOAL},
    {N_("Toggle Boolean Mode"), N_("Toggle Boolean mode for the current proof."), GTK_STOCK_CONVERT, GDK_KEY_m, GDK_CONTROL_MASK, STANDARD, MENU_BOOLEAN}
  };

// The rules menu array.

static mid_t rules_text[RULES_MENU_SIZE] =
  {
    {N_("Toggle Rules"), N_("Show/Hide the rules tablet."), GTK_STOCK_REFRESH, GDK_KEY_r, GDK_CONTROL_MASK, STANDARD, MENU_TOGGLE_RULES},
    {NULL, NULL, NULL, -1, -1, SEPARATOR, -1}
  };

// The font menu array.

static mid_t font_text[FONT_MENU_SIZE] =
  {
    {N_("Small"), N_("Set the font size to small."), GTK_STOCK_ZOOM_OUT, GDK_KEY_minus, GDK_CONTROL_MASK, RADIO, MENU_SMALL},
    {N_("Medium"), N_("Set the font size to medium."), GTK_STOCK_ZOOM_100, GDK_KEY_0, GDK_CONTROL_MASK, RADIO, MENU_MEDIUM},
    {N_("Large"), N_("Set the font size to large."), GTK_STOCK_ZOOM_IN, GDK_KEY_equal, GDK_CONTROL_MASK, RADIO, MENU_LARGE},
    {N_("Custom..."), N_("Set the font size manually."), GTK_STOCK_ZOOM_FIT, -1, 0, RADIO, MENU_CUSTOM}
  };

// The help menu array.

static mid_t help_text[HELP_MENU_SIZE] =
  {
    {N_("Contents"), N_("Display help for GNU Aris."), GTK_STOCK_HELP, GDK_KEY_F1, 0, STANDARD, MENU_CONTENTS},
    {N_("About GNU Aris"), N_("Display information about GNU Aris."), GTK_STOCK_ABOUT, -1, 0, STANDARD, MENU_ABOUT}
  };

// The array of arrays.

static mid_t * menu_text[NUM_MENUS] = { file_text, edit_text, proof_text, rules_text, font_text, help_text };

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

  aris_proof_set_sb (ap, _("Ready"));

  int ret;

  ret = aris_proof_post_init (ap);
  if (ret < 0)
    return NULL;

  ap->goal = goal_init (ap);
  if (!ap->goal)
    return NULL;

  aris_proof_set_font (ap, FONT_TYPE_SMALL);

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
  SEN_PARENT (ap)->separator = gtk_hseparator_new ();

  sd = SEN_DATA_DEFAULT (1, 0, 0);
  if (!sd)
    return -1;

  sen = sentence_init (sd, (sen_parent *) ap, NULL);
  if (!sen)
    return -1;

  gtk_box_pack_start (GTK_BOX (SEN_PARENT (ap)->container),
		      sen->panel, FALSE, FALSE, 0);

  item_t * itm = ls_ins_obj (SEN_PARENT (ap)->everything, sen,
			     SEN_PARENT (ap)->everything->head);
  if (!itm)
    return -1;

  ap->fin_prem = SEN_PARENT (ap)->focused = SEN_PARENT (ap)->everything->head;

  gtk_box_pack_start (GTK_BOX (SEN_PARENT (ap)->container),
		      SEN_PARENT (ap)->separator, FALSE, FALSE, 0);

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
	  SEN_PARENT (ap)->separator = gtk_hseparator_new ();
	  item_t * itm;

	  itm = sen_parent_ins_sentence ((sen_parent *) ap, sd, NULL, 0);

	  ap->fin_prem = SEN_PARENT (ap)->focused = SEN_PARENT (ap)->everything->head;

	  gtk_box_pack_start (GTK_BOX (SEN_PARENT (ap)->container),
			      SEN_PARENT (ap)->separator, FALSE, FALSE, 0);
	  first = 0;
	}
      else
	{
	  sen = aris_proof_create_sentence (ap, sd);
	  if (!sen)
	    return NULL;

	  sentence_set_bg (sen, BG_COLOR_WHITE);
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

  ap->menubar = gtk_menu_bar_new ();

  for (i = 0; i < 6; i++)
    {
      submenu = gtk_menu_new ();
      menu = gtk_menu_item_new_with_label (head_text[i].label);
      mid_t * cur_data = menu_text[i];
      for (j = 0; j < num_subs[i]; j++)
	{
	  construct_menu_item (cur_data[j], G_CALLBACK (menu_activate),
			       submenu, ap->accel, &got_radio);
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
aris_proof_set_changed (aris_proof * ap, int changed)
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
      new_line_no = ev_sen->line_num + line_mod;
      ret = sentence_set_line_no (ev_sen, new_line_no);
      if (ret == -1)
	return -1;

      int i;
      for (i = 0; i < ev_sen->depth; i++)
	{
	  // Check if any index is greater than or equal to
	  // the new item's line number.
	  if (ev_sen->indices[i] >= cur_line)
	    ev_sen->indices[i] = ev_sen->indices[i] + line_mod;
	}
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

  for (g_itr = ap->goal->goals->head; g_itr; g_itr = g_itr->next)
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
 *  output:
 *    the newly created sentence, or NULL on error.
 */
sentence *
aris_proof_create_sentence (aris_proof * ap, sen_data * sd)
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

  new_order = SENTENCE (fcs->value)->line_num;
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

  ret = aris_proof_set_changed (ap, 1);
  if (ret < 0)
    return NULL;

  sen = itm->value;
  
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

  sen = aris_proof_create_sentence (ap, sd);
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

  sen = aris_proof_create_sentence (ap, sd);
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

  sen =  aris_proof_create_sentence (ap, sd);
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

  sen =  aris_proof_create_sentence (ap, sd);
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
aris_proof_set_sb (aris_proof * ap, char * sb_text)
{
  sen_parent_set_sb (SEN_PARENT (ap), sb_text);
  ap->sb_text = sb_text;
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

  gtk_window_set_title (GTK_WINDOW (SEN_PARENT (ap)->window), new_title);

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

      ret_chk = ls_ins_obj (ap->yanked, sd, ap->yanked->tail);
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

  start_itr = sel_itr;

  for (start_itr = sel_itr; start_itr; start_itr = start_itr->next)
    {
      sentence * sen;
      sen = start_itr->value;

      if (sen->line_num == 1)
	return 1;
    }

  for (; sel_itr; sel_itr = sel_itr->next)
    {
      sentence * sen;
      int sen_depth;

      sen = sel_itr->value;
      sen_depth = sen->depth;
      if (sen_depth < org_depth)
	break;

      ret_chk = aris_proof_remove_sentence (ap, sen);
      if (ret_chk == -1)
	return -1;

      if (ap->selected->num_stuff == 0 && (!is_subproof || (sen_depth < org_depth)))
	break;
    }

  int ret;
  ret = aris_proof_set_changed (ap, 1);
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

  item_t * yank_itr;
  int ret;
  for (yank_itr = ap->yanked->head; yank_itr; yank_itr = yank_itr->next)
    {
      sentence * sen;
      sen_data * sd;
      sd = yank_itr->value;
      sen = aris_proof_create_sentence (ap, sd);
      if (!sen)
	return -1;
    }

  ret = aris_proof_set_changed (ap, 1);
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

  aris_proof_set_changed (ap, 0);

  return 0;
}

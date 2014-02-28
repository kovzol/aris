/*  Functions for handling the aris proof guis.

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
#include "undo.h"

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
          if (!itm)
            return NULL;

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
      main_menu_conf[CONF_MENU_EXPORT_LATEX],
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

  int cur_line = sentence_get_line_no ((sentence *) itm->value);
  item_t * ev_itr;

  for (ev_itr = itm->next; ev_itr; ev_itr = ev_itr->next)
    {
      int new_line_no, ret, old_ln;
      sentence * ev_sen;

      ev_sen = ev_itr->value;
      if (!ev_sen)
        exit (EXIT_FAILURE);

      old_ln = sentence_get_line_no (ev_sen);
      new_line_no = old_ln + line_mod;
      ret = sentence_update_line_no (ev_sen, new_line_no);
      if (ret == -1)
        return -1;

      int i;

      for (i = 0; i < SEN_DEPTH(ev_sen); i++)
        {
          // Update all of the indices that are greater than the removed/inserted line.
          if (SEN_IND(ev_sen,i) >= cur_line)
            sentence_set_index (ev_sen, i, SEN_IND(ev_sen,i) + line_mod);
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

  for (g_itr = SEN_PARENT (ap->goal)->everything->head; g_itr; g_itr = g_itr->next)
    {
      sen = g_itr->value;
      unsigned char * entry_text = sentence_get_text (sen);

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

  fcs = SEN_PREM(SEN_PARENT (ap)->focused->value) ? foc_1 : foc_2;

  if (!fcs)
    {
      fprintf (stderr, "Data corruption.\n");
      exit (EXIT_FAILURE);
    }

  new_order = sentence_get_line_no (fcs->value);

  // Increment this, since non-premises are attached at their line number + 1;
  if (!sd->premise)  new_order++;

  if (sd->depth == -1)
    {
      sd->depth = SEN_DEPTH(fcs->value) - 1;
    }
  else if (sd->depth == DEPTH_DEFAULT)
    {
      sd->depth = SEN_DEPTH(fcs->value);
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

  //fprintf (stderr, "create_sentence: sen->line_num == %i\n", sentence_get_line_no (sen));

  undo_info ui;
  ui.type = -1;
  if (undo)
    ui = undo_info_init_one (ap, sen, UIT_ADD_SEN);

  ret = aris_proof_set_changed (ap, 1, ui);
  if (ret < 0)
    return NULL;

  if (sd->rule == RULE_LM && sd->file)
    {
      int file_len, alloc_size, ln;
      GtkWidget * menu_item, * menu, * submenu;
      GList * gl;
      char * label;

      ln = sentence_get_line_no (sen);
      file_len = strlen (sd->file);
      alloc_size = file_len + 4 + 1 + (int) log10 (ln);
      label = (char *) calloc (alloc_size, sizeof (char));
      CHECK_ALLOC (label, NULL);

      sprintf (label, "%i - %s", ln, sd->file);
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
  if (SEN_DEPTH(SEN_PARENT (ap)->focused->value) == 0)
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
  if (sentence_get_line_no (sen) == 1)
    return 1;

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
  if (ap->cur_file)
    {
      free (ap->cur_file);
      ap->cur_file = NULL;
    }

  char * new_title, * base_name;

  new_title = (char *) calloc (strlen (filename) + 12, sizeof (char));
  CHECK_ALLOC (new_title, -1);
  ap->cur_file = strdup (filename);
  CHECK_ALLOC (ap->cur_file, -1);

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
  // First, clear out the old copied sentences.
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

  /* Here is what should be happening:
   *  If nothing is selected, add the current line to ap->selected.
   *  foreach elm in ap->selected:
   *    If it is a subproof, then add each of its children to sel_list.
   *    else, just add elm to sel_list.
   *  Then iterate through each element in sel_list, and copy it.
   */

  list_t * sel_list;
  item_t * sel_itr, * itm;

  sel_list = init_list ();
  if (!sel_list)
    return -1;

  if (!ap->selected)
    {
      ap->selected = init_list ();
      if (!ap->selected)
        return -1;
    }

  if (ls_empty (ap->selected))
    ls_push_obj (ap->selected, (sentence *) SEN_PARENT (ap)->focused->value);

  /* TODO: Make sure that if a sentence is selected, and then its parent is selected,
     that the sentence is removed from selected. */

  for (sel_itr = ap->selected->head; sel_itr; sel_itr = sel_itr->next)
    {
      sentence * sen = sel_itr->value;
      ls_push_obj (sel_list, sen);

      if (!SEN_SUB (sen))
        continue;

      // Iterate through ap.
      int sen_depth;
      sen_depth = SEN_DEPTH(sen);

      item_t * ev_itr;
      ev_itr = ls_find (SEN_PARENT(ap)->everything, sen);
      for (ev_itr = ev_itr->next; ev_itr; ev_itr = ev_itr->next)
        {
          sentence * ev_sen;
          ev_sen = ev_itr->value;
          if (SEN_DEPTH(ev_sen) < sen_depth)
            break;

          itm = ls_push_obj (sel_list, ev_sen);
          if (!itm)
            return -1;
        }
    }

  /* For subproofs:
   *  Determine the line number of the premise.
   *  for each line:
   *    for each reference:
   *      if it is before the premise, then leave it alone.
   *      else, (within the subproof) set it to an offset from the current line.
   */

  /* The sentences should have the same depth, with the exception of subproofs.
   */

  /* This needs to be a stack of line numbers, the top of which should
   *  always be the current subproof's line.
   */

  vec_t * sub_lines;
  sub_lines = init_vec (sizeof (int));
  if (!sub_lines)
    return -1;

  item_t * n_itr;
  for (sel_itr = sel_list->head; sel_itr;)
    {
      sentence * sen;
      sen_data * sd;
      item_t * ret_chk;

      sen = sel_itr->value;
      sd = sentence_copy_to_data (sen);
      if (!sd)
        return ERROR_CODE_MEMORY;

      if (SEN_SUB(sen))
        {
          int sub_line = sd->line_num, rc;
          rc = vec_add_obj (sub_lines, &sub_line);
          if (rc < 0)
            return -1;
        }
      else if (sel_itr->prev && SEN_DEPTH(sen) < SEN_DEPTH(sel_itr->prev->value))
        {
          vec_pop_obj (sub_lines);
        }

      // This clears up potential problems with subproofs.
      sd->depth = sub_lines->num_stuff;

      if (sd->depth > 0)
        {
          int i, * sub_line;
          sub_line = vec_nth (sub_lines, sub_lines->num_stuff - 1);
          for (i = 0; sd->refs[i] != REF_END; i++)
            {
              if (sd->refs[i] < *sub_line)
                continue;

              sd->refs[i] -= sd->line_num;
            }
        }

      // This may cause problems.  I'm not sure yet.  It doesn't look like it.
      if (sel_itr->prev && SEN_DEPTH(sen) < SEN_DEPTH(sel_itr->prev->value))
        sd->depth = -1;
      else
        sd->depth = DEPTH_DEFAULT;

      ret_chk = ls_push_obj (ap->yanked, sd);
      if (!ret_chk)
        return -1;

      n_itr = sel_itr->next;
      //free (sel_itr);
      sel_itr = n_itr;
    }

  for (sel_itr = sel_list->head; sel_itr;)
    {
      n_itr = sel_itr->next;
      free (sel_itr);
      sel_itr = n_itr;
    }

  destroy_vec (sub_lines);
  free (sel_list);

  aris_proof_clear_selected (ap);
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
  
  item_t * sel_itr = ap->yanked->head;

  undo_info ui;
  list_t * ls, * sen_ls;
  ls = init_list ();
  if (!ls)
    return -1;

  sen_ls = init_list ();
  if (!sen_ls)
    return -1;

  item_t * push_chk;

  /* Since refs will be changing, set up undo information and
   *  the list of sentences before removing anything.
   */

  for (; sel_itr; sel_itr = sel_itr->next)
    {
      sen_data * sd = sel_itr->value;

      // This is really inefficient.
      item_t * ev_itr = ls_nth (SEN_PARENT (ap)->everything, sd->line_num - 1);

      sentence * sen;
      sen = ev_itr->value;
      sen_data * undo_sd;

      // This will fix the differences with depth and refs imposed by copy.
      undo_sd = sentence_copy_to_data (sen);
      push_chk = ls_push_obj (ls, undo_sd);
      if (!push_chk)
        return -1;

      push_chk = ls_push_obj (sen_ls, sen);
      if (!push_chk)
        return -1;
    }

  item_t * n_itr;

  for (sel_itr = sen_ls->head; sel_itr;)
    {
      sentence * sen = sel_itr->value;
      ret_chk = aris_proof_remove_sentence (ap, sen);
      if (ret_chk == -1)
        return -1;
      if (ret_chk == 1)
        return 1;
      n_itr = sel_itr->next;
      free (sel_itr);
      sel_itr = n_itr;
    }
  free (sen_ls);

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
  int ret, line_num;
  line_num = sentence_get_line_no ((sentence *)SEN_PARENT(ap)->focused->value);
  for (yank_itr = ap->yanked->head; yank_itr; yank_itr = yank_itr->next)
    {
      line_num++;
      sentence * sen;
      sen_data * sd;
      sd = yank_itr->value;

      if (sd->refs)
        {
          int i;
          for (i = 0; sd->refs[i] != REF_END; i++)
            {
              if (sd->refs[i] > 0)
                continue;
              sd->refs[i] += line_num;
            }
        }

      sen = aris_proof_create_sentence (ap, sd, 0);
      if (!sen)
        return -1;
      ls_push_obj (ls, sentence_copy_to_data (sen));
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

int
aris_proof_clear_selected (aris_proof * ap)
{
  if (!ap->selected)
    return 0;

  item_t * itm, * n_itr;
  for (itm = ap->selected->head; itm; )
    {
      sentence * sen;
      sen = itm->value;

      sentence_set_selected (sen, 0);
      n_itr = itm->next;
      free (itm);
      itm = n_itr;
    }

  free (ap->selected);
  ap->selected = NULL;

  return 0;
}

int
aris_proof_select_sentence (aris_proof * ap, sentence * sen)
{
  item_t * push_chk;
  push_chk = ls_push_obj (ap->selected, sen);
  if (!push_chk)
    return -1;
  return 0;
}

int
aris_proof_deselect_sentence (aris_proof * ap, sentence * sen)
{
  ls_rem_obj_value (ap->selected, sen);
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

      if (SEN_SUB(ev_sen))
        {
          aris_proof_set_sb (ap, _("Subproofs may not be used in boolean mode."));
          return -1;
        }

      if (sentence_get_rule (ev_sen) == -1)
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
    return -1;

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
          int ln;

          ev_sen = ev_itr->value;
          if (!SEN_PREM(ev_sen))
            {
              if (!ev_conc)
                ev_conc = ev_itr;
              break;
            }

          ln = sentence_get_line_no (ev_sen);
          ev_text = sentence_get_text (ev_sen);

          if (!strcmp (ev_text, pf_text))
            {
              refs[ref_num++] = (short) ln;
              break;
            }
        }

      if (!ev_itr || !SEN_PREM(ev_itr->value))
        {
          sentence * sen_chk;
          int ln;

          sen_chk = aris_proof_create_sentence (ap, sd, 1);
          if (!sen_chk)
            return -1;
          ln = sentence_get_line_no (sen_chk);
          refs[ref_num++] = (short) ln;
        }
    }

  refs[ref_num] = REF_END;

  if (!ev_conc)
    {
      for (ev_itr = SEN_PARENT(ap)->everything->head; ev_itr;
           ev_itr = ev_itr->next)
        {
          sentence * ev_sen;

          ev_sen = ev_itr->value;
          if (SEN_PREM(ev_sen))
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
          ev_text = sentence_get_text (ev_sen);

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

  int rc;
  undo_op op;
  op = undo_determine_op (undo, ui->type);
  // Use ui->type and undo to determine new op.

  rc = op (ap, ui);
  return rc;
}

int
aris_proof_to_latex (aris_proof * ap)
{
  proof_t * proof;
  proof = aris_proof_to_proof (ap);
  if (!proof)
    return ERROR_CODE_MEMORY;

  GtkFileFilter * file_filter;
  file_filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (file_filter, "LaTeX Files");
  gtk_file_filter_add_pattern (file_filter, "*.tex");

  char * filename;
  GtkWidget * file_chooser;
  file_chooser =
    gtk_file_chooser_dialog_new (_("Select a file to Save to..."),
                                 GTK_WINDOW (SEN_PARENT (ap)->window),
                                 GTK_FILE_CHOOSER_ACTION_SAVE,
                                 "_Cancel", GTK_RESPONSE_CANCEL,
                                 "_Save", GTK_RESPONSE_ACCEPT,
                                 NULL);
  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (file_chooser),
                                        FALSE);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (file_chooser), TRUE);
  gtk_file_chooser_set_create_folders (GTK_FILE_CHOOSER (file_chooser), TRUE);
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (file_chooser), file_filter);
  if (gtk_dialog_run (GTK_DIALOG (file_chooser)) == GTK_RESPONSE_ACCEPT)
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));

  gtk_widget_destroy (file_chooser);

  int rc;
  rc = convert_proof_latex (proof, filename);
  if (rc == ERROR_CODE_MEMORY)
    return ERROR_CODE_MEMORY;

  proof_destroy (proof);

  return 0;
}

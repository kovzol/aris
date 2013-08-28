/* Functions to handle callbacks for Aris.

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
#include <time.h>
#include <math.h>

#include "callbacks.h"
#include "app.h"
#include "rules.h"
#include "rules-table.h"
#include "aris-proof.h"
#include "menu.h"
#include "aio.h"
#include "sen-data.h"
#include "process.h"
#include "sexpr-process.h"
#include "goal.h"
#include "list.h"
#include "sentence.h"
#include "sen-parent.h"
#include "config.h"
#include "vec.h"
#include "var.h"
#include "interop-isar.h"
#include "proof.h"
#include "conf-file.h"

/* Check gtk+ documentation for more information about signals. */

/* Calls rule_toggled with data.
 *  input:
 *    data - the index of the rule being toggled.
 */
G_MODULE_EXPORT void
toggled (GtkToggleButton * tg, gpointer data)
{
  int tgld = GPOINTER_TO_INT (data);
  
  rule_toggled (tgld);
}

/* Calls rules_table_focused (in rules-table.h)
 *  output:
 *    TRUE, to prevent propagation.
 */
G_MODULE_EXPORT gboolean
rules_table_focus (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  rules_table_focused ();
  return TRUE;
}

/* Calls menu_activated (end of this file) on the focused gui with data.
 *  input:
 *    data - the menu id of the activated menu item.
 */
G_MODULE_EXPORT void
menu_activate (GtkMenuItem * menuitem, gpointer data)
{
  //printf ("Got menu activate signal\n");

  int item_id;

  item_id = GPOINTER_TO_INT (data);

  menu_activated (the_app->focused, item_id);

  return;
}

/* Calls gui_destroy (later in this file) on a gui specified by data.
 *  input:
 *    data - the aris proof to be destroyed.
 */
G_MODULE_EXPORT gboolean
window_delete (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  aris_proof * ap = (aris_proof *) data;
  gui_destroy (ap);
  return FALSE;
}

/* Calls the_app_set_focus (app.h) on a gui specified by data.
 *  input:
 *    data - the aris proof to be focused on.
 */
G_MODULE_EXPORT gboolean
window_focus_in (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  aris_proof * ap = (aris_proof *) data;

  the_app_set_focus (ap);

  return FALSE;
}

/* Calls sentence_in (sentence.h) on a sentence specified by data.
 *  input:
 *    data - the sentence to be focused on.
 */
G_MODULE_EXPORT gboolean
sentence_focus_in (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  // Construct the data.
  sentence * sen = (sentence *) data;

  // Call the cc callback.
  sentence_in (sen);

  return FALSE;
}

/* Calls sentence_out (sentence.h) on a sentence specified by data.
 *  input:
 *    data - the sentence that is losing focus.
 */
G_MODULE_EXPORT gboolean
sentence_focus_out (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  // Construct the data
  sentence * sen = (sentence *) data;

  // Call the cc callback
  sentence_out (sen);

  return FALSE;
}

/* Prevents a sentence from being focused on if the user hits ctrl+mouse-1.
 */
G_MODULE_EXPORT gboolean
sentence_btn_press (GtkWidget * widget, GdkEventButton * event, gpointer data)
{
  if ((event->state == GDK_CONTROL_MASK && event->button == 1)
      || (event->state == GDK_SHIFT_MASK && event->button == 1))
    {
      return TRUE;
    }
  return FALSE;
}

/* Calls select_reference on a sentence if the user hits ctrl+mouse-1.
 *  input:
 *    data - the sentence being selected.
 *  output:
 *    TRUE if the reference was selected, FALSE otherwise.
 */
G_MODULE_EXPORT gboolean
sentence_btn_release (GtkWidget * widget, GdkEventButton * event, gpointer data)
{
  //printf ("Sentence Button Release\n");

  if (event->state == (GDK_CONTROL_MASK | GDK_BUTTON1_MASK) && event->button == 1)
    {
      // Only want to do anything if the ctrl key is pressed.
      sentence * sen = (sentence *) data;

      select_reference (sen);

      return TRUE;
    }

  if (event->state == (GDK_SHIFT_MASK | GDK_BUTTON1_MASK) && event->button == 1)
    {
      sentence * sen = (sentence *) data;

      // Will eventually select sentence.
      //sentence_set_bg (sen, BG_COLOR_RED_ORANGE);
      select_sentence (sen);

      return TRUE;
    }

  return FALSE;
}

/* Calls sentence_key (sentence.h) on a sentence specified by data.
 *  input:
 *    data - the sentence on which to process the key press.
 *  output:
 *    TRUE if the user pressed UP or DOWN, FALSE otherwise.
 */
G_MODULE_EXPORT gboolean
sentence_key_press (GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  int ctrl;
  sentence * sen;
  int prop;

  sen = (sentence *) data;

  ctrl = event->state & GDK_CONTROL_MASK;
  prop = sentence_key (sen, event->keyval, ctrl);

  if (prop == 0)
    return TRUE;
  return FALSE;
}

/* Calls sentence_text_changed (sentence.h) on a sentence specified by data.
 *  input:
 *    data - the sentence being changed.
 */
G_MODULE_EXPORT void
sentence_changed (GtkTextView * edit, gpointer data)
{
  sentence * sen = (sentence *) data;

  sentence_text_changed (sen);
}

/* Used to prevent a bug in sentence initialization.
 * Without this, the focus adjustment in the sentence's parent's scrolled window
 *  wouldn't scroll to a sentence when it was initialized.
 * With this, it sets focus to it as soon as it is properly mapped,
 *  allowing the scrolled window to recognize it immediately.
 * After this is called, it disconnects the signal handler.
 */
G_MODULE_EXPORT void
sentence_mapped (GtkWidget * widget, GdkRectangle * rect, gpointer data)
{
  sentence * sen = data;

  gtk_widget_grab_focus (sen->entry);
  g_signal_handler_disconnect (sen->entry, sen->sig_id);
}

/* Processes the goal's menu being activated; uses the goal of the focused gui.
 *  input:
 *    data - the menu id of the menu item being activated.
 */
G_MODULE_EXPORT void
goal_menu_activate (GtkMenuItem * item, gpointer data)
{
  int menu_id = GPOINTER_TO_INT (data);
  goal_t * goal = the_app->focused->goal;
  sen_data * sd = SEN_DATA_DEFAULT (1, 0, 0);

  switch (menu_id)
    {
    case MENU_ADD_PREM:
      goal_add_line (goal, sd);
      break;
    case MENU_KILL:
      goal_rem_line (goal);
      break;
    case MENU_EVAL_LINE:
      if (goal->focused)
	goal_check_line (goal, goal->focused->value);
      break;
    case MENU_EVAL_PROOF:
      goal_check_all (goal);
      break;
    case MENU_GOAL:
      gtk_widget_hide (goal->window);
      aris_proof_set_sb (goal->parent, _("Goal Window Hidden."));
      break;
    }
}

/* Hides the goal specified by data.
 *  input:
 *    data - the goal to hide.
 */
G_MODULE_EXPORT gboolean
goal_delete (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  goal_t * goal = (goal_t *) data;
  gtk_widget_hide (goal->window);
  return TRUE;
}

/* Presents the parent of the goal specified by data to the user.
 *  input:
 *    data - the goal being focused on.
 */
G_MODULE_EXPORT gboolean
goal_focus_in (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  goal_t * goal = (goal_t *) data;

  the_app_set_focus (goal->parent);

  return FALSE;
}

/* Handles an activation of the rules table's menu.
 *  input:
 *    data - the menu id of the menu item being activated.
 */
G_MODULE_EXPORT void
rule_menu_activated (GtkMenuItem * menuitem, gpointer data)
{
  int menu_id = GPOINTER_TO_INT (data);
  GList * gl;
  GtkWidget * font_menu, * font_submenu, * font_sel;
  GtkWidget * font_small, * font_medium, * font_large;
  rules_table * rt = the_app->rt;
  int new_font, cur_font;

  if (menu_id >= MENU_SMALL && menu_id <= MENU_CUSTOM)
    {
      gl = gtk_container_get_children (GTK_CONTAINER (rt->menubar));
      font_menu = (GtkWidget *) g_list_nth_data (gl, 1);
      font_submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (font_menu));

      gl = gtk_container_get_children (GTK_CONTAINER (font_submenu));
      font_sel = (GtkWidget *) g_list_nth_data (gl, rt->font);

      font_small = (GtkWidget *) g_list_first (gl)->data;
      font_medium = (GtkWidget *) g_list_first(gl)->next->data;
      font_large = (GtkWidget *) g_list_first(gl)->next->next->data;
    }

  switch (menu_id)
    {
    case MENU_NEW:
      gui_new ();
      break;

    case MENU_OPEN:
      gui_open (the_app->rt->window);
      break;

    case MENU_SUBMIT:
      gui_submit_show (the_app->rt->window);
      break;

    case MENU_QUIT:
      app_quit ();
      break;
    case MENU_SMALL:
      if (rt->font != FONT_TYPE_SMALL)
	{
	  gtk_widget_set_sensitive (font_sel, TRUE);
	  rules_table_set_font (rt, FONT_TYPE_SMALL);
	  gtk_widget_set_sensitive (font_small, FALSE);
	}
      break;
    case MENU_MEDIUM:
      if (rt->font != FONT_TYPE_MEDIUM)
	{
	  gtk_widget_set_sensitive (font_sel, TRUE);
	  rules_table_set_font (rt, FONT_TYPE_MEDIUM);
	  gtk_widget_set_sensitive (font_medium, FALSE);
	}
      break;
    case MENU_LARGE:
      if (rt->font != FONT_TYPE_LARGE)
	{
	  gtk_widget_set_sensitive (font_sel, TRUE);
	  rules_table_set_font (rt, FONT_TYPE_LARGE);
	  gtk_widget_set_sensitive (font_large, FALSE);
	}
      break;
    case MENU_CUSTOM:
      if (gtk_widget_get_sensitive (font_sel))
	{
	  FONT_GET_SIZE (the_app->fonts[FONT_TYPE_CUSTOM], cur_font);
	}
      else
	cur_font = 0;

      new_font = gui_set_custom (rt->window, cur_font);

      if (new_font > 0)
	{
	  gtk_widget_set_sensitive (font_sel, TRUE);
	  if (the_app->fonts[FONT_TYPE_CUSTOM])
	    pango_font_description_free (the_app->fonts[FONT_TYPE_CUSTOM]);
	  INIT_FONT (the_app->fonts[FONT_TYPE_CUSTOM], new_font);
	  rules_table_set_font (rt, FONT_TYPE_CUSTOM);
	}
      break;
    case MENU_CONTENTS:
      gui_help ();
      break;

    case MENU_CUSTOMIZE:
      gui_customize_show (rt->window);
      break;

    case MENU_ABOUT:
      gui_about (the_app->rt->window);
      break;
    }
}

/* Calls app_quit (app.h).
 *  output:
 *    TRUE to prevent propogation.
 */
G_MODULE_EXPORT gboolean
rules_table_deleted (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  // app_quit () will not return if it succeeds.
  // If it fails, then the user stopped it, so return true.
  app_quit ();
  return TRUE;
}

/* Minimizes the open proofs when the rules table is minimized.
 *  output:
 *    FALSE to continue propogation.
 */
G_MODULE_EXPORT gboolean
rules_table_state (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  GdkEventWindowState * ev = (GdkEventWindowState *) event;

  if (ev->changed_mask == GDK_WINDOW_STATE_ICONIFIED)
    {
      item_t * app_itr;

      for (app_itr = the_app->guis->head; app_itr; app_itr = app_itr->next)
	{
	  aris_proof * ap;
	  ap = app_itr->value;

	  gtk_window_iconify (GTK_WINDOW (ap->window));
	}
    }

  return FALSE;
}

/* Destroys an aris proof, checking if it has unsaved changes.
 *  input:
 *    ap - the aris proof to destroy.
 *  output:
 *    -1 if the user cancels the destruction, 0 otherwise.
 */
int
gui_destroy (aris_proof * ap)
{
  if (ap->edited)
    {
      GtkWidget * confirm_prompt;
      int result;

      confirm_prompt =
	gtk_message_dialog_new_with_markup (GTK_WINDOW (SEN_PARENT (ap)->window),
					    GTK_DIALOG_MODAL
					    | GTK_DIALOG_DESTROY_WITH_PARENT,
					    GTK_MESSAGE_WARNING,
					    GTK_BUTTONS_OK_CANCEL,
					    "<b>Unsaved Changes</b>");

      gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (confirm_prompt),
						  _("There are unsaved changes to this proof.\nAre you sure that you want to exit?"));

      result = gtk_dialog_run (GTK_DIALOG (confirm_prompt));
      gtk_widget_destroy (confirm_prompt);
      if (result != GTK_RESPONSE_OK)
	return -1;
    }

  int ret;
  the_app_rem_gui (ap);
  aris_proof_destroy (ap);

  return 0;
}

/* Creates a new aris proof gui.
 *  input:
 *    none.
 *  output:
 *    0 on success, -1 on error.
 */
int
gui_new ()
{
  aris_proof * new_ap;
  int ret;

  new_ap = aris_proof_init (NULL);
  if (!new_ap)
    return -1;

  ret = the_app_add_gui (new_ap);
  if (ret < 0)
    return -1;

  return 0;
}

/* Opens a file to an aris proof.
 *  input:
 *    window - the parent window for the GtkFileChooserDialog.
 *  output:
 *    0 on success, -1 on error.
 */
int
gui_open (GtkWidget * window)
{
  GtkFileFilter * file_filter;
  file_filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (file_filter, "Aris Files");
  gtk_file_filter_add_pattern (file_filter, "*.tle");

  GtkWidget * file_chooser;
  file_chooser =
    gtk_file_chooser_dialog_new (_("Select a file to Open..."),
				 GTK_WINDOW (window),
				 GTK_FILE_CHOOSER_ACTION_OPEN,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				 NULL);
  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (file_chooser), FALSE);
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (file_chooser), file_filter);

  // First time, it happened in here.
  // It tried loading the dialog, but didn't get far enough.
  // free(): invalid pointer 874f10
  // gtk_dialog_run

  if (gtk_dialog_run (GTK_DIALOG (file_chooser)) == GTK_RESPONSE_ACCEPT)
    {
      char * filename;
      aris_proof * new_ap;
      proof_t * proof;
      int ret;

      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));

      proof = aio_open (filename);
      if (!proof)
	{
	  gtk_widget_destroy (file_chooser);
	  return -1;
	}

      new_ap = aris_proof_init_from_proof (proof);
      if (!new_ap)
	{
	  gtk_widget_destroy (file_chooser);
	  return -1;
	}

      ret = the_app_add_gui (new_ap);
      if (ret < 0)
	{
	  gtk_widget_destroy (file_chooser);
	  return -1;
	}

      ret = aris_proof_set_filename (new_ap, filename);
      if (ret < 0)
	{
	  gtk_widget_destroy (file_chooser);
	  return -1;
	}

      gui_save (new_ap, 0);
      new_ap->edited = 0;
    }

  gtk_widget_destroy (file_chooser);
  return 0;
}

/* Saves an aris proof to a file.
 *  input:
 *    ap - the aris proof being saved.
 *    save_as - 1 if this is being called by 'save as', 0 otherwise.
 *  output:
 *    0 on success, -1 on error.
 */
int
gui_save (aris_proof * ap, int save_as)
{
  GtkFileFilter * file_filter;
  file_filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (file_filter, "Aris Files");
  gtk_file_filter_add_pattern (file_filter, "*.tle");

  char * filename;
  filename = ap->cur_file;

  if (save_as || !filename)
    {
      GtkWidget * file_chooser;
      file_chooser =
	gtk_file_chooser_dialog_new (_("Select a file to Save to..."),
				     GTK_WINDOW (SEN_PARENT (ap)->window),
				     GTK_FILE_CHOOSER_ACTION_SAVE,
				     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				     NULL);
      gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (file_chooser),
					    FALSE);
      gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (file_chooser), TRUE);
      gtk_file_chooser_set_create_folders (GTK_FILE_CHOOSER (file_chooser), TRUE);
      gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (file_chooser), file_filter);
      if (gtk_dialog_run (GTK_DIALOG (file_chooser)) == GTK_RESPONSE_ACCEPT)
	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));

      gtk_widget_destroy (file_chooser);
    }

  if (filename)
    {
      proof_t * proof;
      int ret;

      proof = aris_proof_to_proof (ap);
      if (!proof)
	return -1;

      ret = aio_save (proof, filename);
      if (ret < 0)
	return -1;

      ret = aris_proof_set_changed (ap, 0);
      if (ret < 0)
	return -1;

      ret = aris_proof_set_filename (ap, filename);
      if (ret < 0)
	return -1;
    }

  return 0;
}

/* Evaluates a sentence.
 *  input:
 *    ap - the aris proof containing the sentence being evaluated.
 *    sen - the sentence being evaluated.
 *  output:
 *    0 on success, -1 on memory error, non-zero on generic error.
 */
int
evaluate_line (aris_proof * ap, sentence * sen)
{
  item_t * ev_itr, * ret_chk;
  int ret;
  list_t * lines;
  sen_data * tmp_sd;

  lines = init_list ();
  if (!lines)
    return -1;

  for (ev_itr = SEN_PARENT (ap)->everything->head;
       ev_itr != SEN_PARENT (ap)->focused; ev_itr = ev_itr->next)
    {
      sentence * ev_sen = ev_itr->value;
      if (ev_sen->text[0] == '\0')
	continue;

      if (!ev_sen->sexpr)
	{
	  ret = check_text (ev_sen->text);
	  if (ret == -1)
	    return -1;

	  if (ret != 0)
	    continue;

	  unsigned char * tmp_str, * sexpr_sen;
	  tmp_str = die_spaces_die (ev_sen->text);
	  if (!tmp_str)
	    return -1;

	  sexpr_sen = convert_sexpr (tmp_str);
	  if (!sexpr_sen)
	    return -1;
	  free (tmp_str);
	  ev_sen->sexpr = sexpr_sen;
	}

      ret = sentence_can_select_as_ref (sen, ev_sen);

      if (ret == ev_sen->line_num)
	{
	  int arb = (ev_sen->premise || ev_sen->rule == RULE_EI || ev_sen->subproof) ?
	    0 : 1;
	  ret = sexpr_collect_vars_to_proof (ap->vars, ev_sen->sexpr, arb);
	  if (ret == -1)
	    return -1;
	}
    }

  // Construct lines.

  for (ev_itr = SEN_PARENT (ap)->everything->head;
       ev_itr; ev_itr = ev_itr->next)
    {
      sentence * ev_sen;
      ev_sen = ev_itr->value;

      tmp_sd = sentence_copy_to_data (ev_sen);
      if (!tmp_sd)
	return -1;

      ret_chk = ls_push_obj (lines, tmp_sd);
      if (!ret_chk)
	return -1;
    }

  // Get the sentence data.
  sen_data * sd;
  sd = sentence_copy_to_data (sen);
  if (!sd)
    return -1;

  char * ret_str;
  ret_str = sen_data_evaluate (sd, &ret, ap->vars, lines);
  if (!ret_str)
    return -1;

  sentence_set_value (sen, ret);
  aris_proof_set_sb (ap, ret_str);

  for (ev_itr = lines->head; ev_itr;)
    {
      sen_data * sd;
      item_t * n_itr;

      sd = ev_itr->value;
      sen_data_destroy (sd);

      n_itr = ev_itr->next;
      ev_itr->next = ev_itr->prev = NULL;
      free (ev_itr);
      ev_itr = n_itr;
    }

  free (lines);

  return ret;
}

/* Evaluates an aris proof.
 *  input:
 *    ap - the aris proof to evaluate.
 *  output:
 *    0 on success, -1 on error.
 */
int
evaluate_proof (aris_proof * ap)
{
  item_t * ev_itr;
  sentence * sen;
  int ret;

  for (ev_itr = SEN_PARENT (ap)->everything->head; ev_itr; ev_itr = ev_itr->next)
    {
      sentence * ev_sen = ev_itr->value;

      if (!ev_sen->sexpr)
	{
	  if (ev_sen->text[0] == '\0')
	    continue;

	  ret = check_text (ev_sen->text);
	  if (ret == -1)
	    return -1;

	  if (ret != 0)
	    continue;

	  unsigned char * tmp_str, * sexpr;
	  tmp_str = die_spaces_die (ev_sen->text);
	  if (!tmp_str)
	    return -1;

	  sexpr = convert_sexpr (tmp_str);
	  if (!sexpr)
	    return -1;
	  free (tmp_str);
	  ev_sen->sexpr = sexpr;
	}
    }

  for (ev_itr = SEN_PARENT (ap)->everything->head; ev_itr; ev_itr = ev_itr->next)
    {
      sen = ev_itr->value;
      ret = evaluate_line (ap, sen);
      if (ret == -1)
	return -1;

      ls_clear (ap->vars);
    }

  return 0;
}

/* Toggles the goal window.
 *  input:
 *    ap - the aris proof for which the goal window is being toggled.
 *  output:
 *    0 on success.
 */
int
gui_goal_check (aris_proof * ap)
{
  int vis = gtk_widget_get_visible (ap->goal->window);
  if (!vis)
    {
      gtk_widget_show_all (ap->goal->window);
      aris_proof_set_sb (ap, _("Goal Window Shown."));
    }
  else
    {
      gtk_widget_hide (ap->goal->window);
      aris_proof_set_sb (ap, _("Goal Window Hidden."));
    }
  return 0;
}

/* Toggles the rules window.
 *  input:
 *    ap - the currently focused aris proof.
 *  output:
 *    0 on success.
 */
int
gui_toggle_rules (aris_proof * ap)
{
  int vis;
  int x, y;
  int width;

  g_object_get (G_OBJECT (the_app->rt->window), "visible", &vis, NULL);

  if (vis)
    {
      gtk_widget_hide (the_app->rt->window);
      aris_proof_set_sb (ap, _("Rules Tablet Hidden."));
    }
  else
    {

      gtk_widget_show_all (the_app->rt->window);
      rules_table_align (the_app->rt, ap);
    }

  return 0;
}

/* Sets a custom font size to a gui.
 *  input:
 *    window - the parent window for the dialog.
 *    cur_font - the current font of the gui.
 *  output:
 *    the new font size, or 0 if none was selected.
 */
int
gui_set_custom (GtkWidget * window, int cur_font)
{
  GtkWidget * dialog, * content, * spinner;
  int run, ret;

  dialog = gtk_dialog_new_with_buttons (_("Set Font Size"),
					GTK_WINDOW (window),
					GTK_DIALOG_MODAL,
					GTK_STOCK_OK, GTK_RESPONSE_OK,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					NULL);

  content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  spinner = gtk_spin_button_new_with_range (8.0, 36.0, 1.0);
  if (cur_font > 0)
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinner), cur_font);

  gtk_container_add (GTK_CONTAINER (content), spinner);

  gtk_widget_show (spinner);

  run = gtk_dialog_run (GTK_DIALOG (dialog));
  ret = (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (spinner));

  gtk_widget_destroy (dialog);

  if (run == GTK_RESPONSE_OK)
    return ret;

  return 0;
}

/* Displays the about dialog for aris.
 *  input:
 *    window - the parent window of the about dialog.
 *  output:
 *    0 on success.
 */
int
gui_about (GtkWidget * window)
{
  const char * authors[] = {
    _("Ian Dunn <dunni@gnu.org>"),
    NULL
  };

  const char * artists[] = {
    _("Ian Dunn <dunni@gnu.org>"),
    NULL
  };

  const char * documenters[] = {
    _("Ian Dunn <dunni@gnu.org>"),
    NULL
  };

  const char * license = _("This program is free software; you can redistribute it \
and/or modify it under the terms of the GNU General Public License as published \
by the Free Software Foundation; either version 3 of the license, or \
(at your option) any later version.\n\n \
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY \
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License along \
with this program; if not, write to the Free Software Foundation, Inc., 51 \
Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.");

  gtk_show_about_dialog (GTK_WINDOW (window),
			 "program-name", _("Aris"),
			 "version", VERSION,
			 "authors", authors,
			 "artists", artists,
			 "documenters", documenters,
			 "license", license,
			 "comments", _("  GNU Aris is a logical proof program, which supports both propositional and predicate logic, as well as Boolean algebra and arithmetical logic in the form of abstract sequences."),
			 "logo", the_app->icon,
			 "wrap-license", TRUE,
			 "website", _("http://www.gnu.org/software/aris/"),
			 NULL);

  return 0;
}

/* Displays the help documentation for aris.
 *  input:
 *    none.
 *  output:
 *    0 on success, -1 if the help documentation wasn't found.
 */
int
gui_help ()
{
  gboolean ret = gtk_show_uri (NULL, the_app->help_file, GDK_CURRENT_TIME, NULL);

  if (!ret)
    {
      gtk_show_uri (NULL, "http://www.gnu.org/software/aris/manual/",
		    GDK_CURRENT_TIME, NULL);
    }

  return 0;
}

/* Runs the customization dialog.
 *  input:
 *   window - The parent window of the dialog.
 *  output:
 *   0 on success, -1 on memory error, -2 on file error.
 */
int
gui_customize_show (GtkWidget * window)
{

  GtkWidget * content, * dialog, * main_kc_table, * notebook,
    * ip_table, * font_table, * goal_kc_table;

  dialog = gtk_dialog_new_with_buttons (_("Customize"),
					GTK_WINDOW (window),
					GTK_DIALOG_MODAL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					NULL);

  content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  notebook = gtk_notebook_new ();

  /* Set up the customization run through. */

  int i, j;

  GtkWidget * tables[] = {main_kc_table, goal_kc_table,
			  font_table, ip_table};
  conf_obj * confs[] = {main_menu_conf, goal_menu_conf,
			 display_conf, internal_conf};

  int sizes[] = {25, 5, 10, 1};
  char * labels[] = {_("Main Keys"), _("Goal Keys"), _("Display"), _("Internal")};

  // Run it for all of the configuration tables.
  for (j = 0; j < 4; j++)
    {
      tables[j] = gtk_table_new (13, 4, FALSE);
      for (i = 0; i < sizes[j]; i++)
	{
	  GtkWidget * label;
	  conf_obj cur_obj;

	  cur_obj = confs[j][i];

	  confs[j][i].widget = confs[j][i].value_func (&confs[j][i], 1);
	  label = gtk_label_new (confs[j][i].label);
	  gtk_widget_set_tooltip_text (label, confs[j][i].tooltip);

	  gtk_widget_set_tooltip_text (confs[j][i].widget,
				       confs[j][i].tooltip);

	  int col;
	  col = i / 13;
	  col *= 2;

	  gtk_table_attach_defaults (GTK_TABLE (tables[j]), label,
				     col, col + 1,
				     i % 13, i % 13 + 1);

	  gtk_table_attach_defaults (GTK_TABLE (tables[j]), confs[j][i].widget,
				     col + 1, col + 2,
				     i % 13, i % 13 + 1);
	}

      GtkWidget * tbl_label;

      tbl_label = gtk_label_new (labels[j]);
      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), tables[j],
				tbl_label);
    }

  gtk_container_add (GTK_CONTAINER (content), notebook);
  gtk_widget_show_all (dialog);

  int problem = 1;

  while (problem)
    {
      if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
	  problem = 0;
	  /* First, confirm that each entry is unique. */
	  for (j = 0; j < 2; j++)
	    {
	      int k;
	      conf_obj obj_i, obj_k;

	      for (i = 0; i < sizes[j]; i++)
		{
		  obj_i = confs[j][i];
		  const char * str_i;
		  str_i = gtk_entry_get_text (GTK_ENTRY (obj_i.widget));

		  if (str_i[0] == '\0')
		    continue;

		  for (k = 0; k < sizes[j]; k++)
		    {
		      if (i == k)
			continue;

		      obj_k = confs[j][k];

		      const char * str_k;
		      str_k = gtk_entry_get_text (GTK_ENTRY (obj_k.widget));

		      if (str_k[0] == '\0')
			continue;

		      if (!strcmp (str_i, str_k))
			{
			  // The same key has been assigned to
			  // two different commands.

			  GtkWidget * err_dialog;

			  err_dialog
			    = gtk_message_dialog_new (GTK_WINDOW (dialog),
						      GTK_DIALOG_MODAL,
						      GTK_MESSAGE_ERROR,
						      GTK_BUTTONS_CLOSE,
						      _("There are two commands with the same key\nPlease change this."));
			  gtk_dialog_run (GTK_DIALOG (err_dialog));
			  problem = 1;
			  gtk_widget_destroy (err_dialog);
			  break;
			}
		    }

		  if (problem == 1)
		    break;
		}

	      if (problem == 1)
		break;
	    }

	  if (problem == 1)
	    continue;

	  problem = 0;

	  FILE * conf_file;
	  char * path, * home_dir;
	  int alloc_size, path_pos = 0;

	  home_dir = getenv ("HOME");
	  if (!home_dir)
	    home_dir = getenv ("HOMEPATH");

#ifdef WIN32
	  alloc_size += strlen (getenv ("HOMEDRIVE")) + 1;
#endif
	  path = (char *) calloc (alloc_size + 1, sizeof (char));
	  CHECK_ALLOC (path, -1);

#ifdef WIN32
	  path_pos += sprintf (path, "%s\\", getenv ("HOMEDRIVE"));
#endif

	  sprintf (path + path_pos, "%s/%s", home_dir, CONF_FILE);

	  conf_file = fopen (path, "w");
	  if (!conf_file)
	    {
	      perror (NULL);
	      gtk_widget_destroy (dialog);
	      return -2;
	    }

	  for (j = 0; j < 4; j++)
	    {
	      tables[j] = gtk_table_new (13, 4, FALSE);

	      for (i = 0; i < sizes[j]; i++)
		{
		  GtkWidget * label;
		  conf_obj cur_obj;
		  char * print_str;

		  cur_obj = confs[j][i];
		  print_str = cur_obj.value_func (&cur_obj, 0);

		  if (print_str)
		    {
		      fprintf (conf_file, "%s", print_str);
		      free (print_str);
		    }
		}
	    }

	  fclose (conf_file);

	  the_app_read_config_file (the_app);
	}
      else
	{
	  problem = 0;
	}
    }

  gtk_widget_destroy (dialog);

  return 0;
}

/* Runs the submission dialog.
 *  input:
 *   ap - the proof being submitted.
 *  output:
 *   0 on success, -1 on memory error.
 */
int
gui_submit_show (GtkWidget * window)
{
  // Dialog [Submit / Cancel]
  //  - VBox
  //    - HBox
  //      - GtkLabel "Homework: "
  //      - GtkEdit [      ]
  //    - HBox
  //      - GtkLabel "Email: "
  //      - GtkEdit [      ]
  //    - HBox
  //      - GtkLabel "Instructor's Email (optional): "
  //      - GtkEdit [      ]

  GtkWidget * dialog, * content, * table;
  GtkWidget * user_label, * user_entry, * instr_label, * instr_entry;

  dialog = gtk_dialog_new_with_buttons (_("Submit Proofs"),
					GTK_WINDOW (window),
					GTK_DIALOG_MODAL,
					GTK_STOCK_OK, GTK_RESPONSE_OK,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					NULL);

  content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  table = gtk_table_new (4 + the_app->guis->num_stuff, 2, FALSE);

  user_label = gtk_label_new (_("Your Email: "));
  user_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), user_label, 0, 1, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (table), user_entry, 1, 2, 0, 1);

  instr_label = gtk_label_new (_("(Optional) Instructor's Email: "));
  instr_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), instr_label, 0, 1, 1, 2);
  gtk_table_attach_defaults (GTK_TABLE (table), instr_entry, 1, 2, 1, 2);

  GtkWidget * sep_label;

  sep_label = gtk_label_new (_("    "));
  gtk_table_attach_defaults (GTK_TABLE (table), sep_label, 0, 2, 2, 3);

  GtkWidget * proof_label, * hw_label;

  hw_label = gtk_label_new (_("Problem"));
  proof_label = gtk_label_new (_("File Name"));

  gtk_table_attach_defaults (GTK_TABLE (table), hw_label, 0, 1, 3, 4);
  gtk_table_attach_defaults (GTK_TABLE (table), proof_label, 1, 2, 3, 4);

  item_t * g_itr;
  int i = 4;

  for (g_itr = the_app->guis->head; g_itr; g_itr = g_itr->next)
    {
      GtkWidget * label, * entry;
      aris_proof * ap;

      ap = g_itr->value;

      if (ap->cur_file)
	{

	  label = gtk_label_new (ap->cur_file);
	  entry = gtk_entry_new ();

	  gtk_table_attach_defaults (GTK_TABLE (table), entry, 0, 1, i, i + 1);
	  gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, i, i + 1);
	  
	  i++;
	}
    }

  if (i == 4)
    {
      GtkWidget * label;
      label = gtk_label_new (_("Please save your work first."));
      gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 2, i, i + 1);
      gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
    }

  gtk_container_add (GTK_CONTAINER (content), table);
  gtk_widget_show_all (table);

  int response;

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      const char * user_email, * instr_email;
      struct submit_ent * entries;

      user_email = gtk_entry_get_text (GTK_ENTRY (user_entry));
      instr_email = gtk_entry_get_text (GTK_ENTRY (instr_entry));

      entries = (struct submit_ent *) calloc (the_app->guis->num_stuff,
					       sizeof (struct submit_ent));
      CHECK_ALLOC (entries, -1);

      GList * gl;
      i = 0;

      gl = gtk_container_get_children (GTK_CONTAINER (table));

      while (gl)
	{
	  GtkWidget * wid;

	  wid = gl->data;

	  if (GTK_IS_LABEL (wid)
	      && !strcmp (gtk_label_get_label (GTK_LABEL (wid)),
			  _("File Name")))
	    {
	      break;
	    }

	  GtkWidget * ent;

	  ent = gl->next->data;

	  entries[i].hw = strdup (gtk_entry_get_text (GTK_ENTRY (ent)));
	  entries[i].file_name = strdup (gtk_label_get_label (GTK_LABEL (wid)));

	  i++;
	  gl = gl->next->next;
	}

      int rc;
      rc = the_app_submit (user_email, instr_email, entries);
      if (rc == -3)
	{
	  GtkWidget * error_dialog;
	  error_dialog = gtk_message_dialog_new (GTK_WINDOW (window),
						 GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 "Unable to submit files to server: %s",
						 the_app->ip_addr);
	  gtk_dialog_run (GTK_DIALOG (error_dialog));
	  gtk_widget_destroy (error_dialog);
	}

      for (i = 0; i < the_app->guis->num_stuff; i++)
	{
	  free (entries[i].hw);
	  free (entries[i].file_name);
	}

      free (entries);
    }

  gtk_widget_destroy (dialog);

  return 0;
}

/* Handles the menu activation for an aris proof.
 *  input:
 *    ap - the aris proof for which the menu activation is being handled.
 *    menu_id - the menu id of the menu item being activated.
 *  output:
 *    0 on success, -1 on error.
 */
int
menu_activated (aris_proof * ap, int menu_id)
{
  // Get the 'radio' menu items.
  GList * gl;
  GtkWidget * font_menu, * font_submenu, * font_sel;
  GtkWidget * font_small, * font_medium, * font_large;
  int ret;
  sentence * sen;
  int new_font, cur_font;
  item_t * ev_itr;
  int arb;

  if (menu_id >= MENU_SMALL && menu_id <= MENU_CUSTOM)
    {
      gl = gtk_container_get_children (GTK_CONTAINER (SEN_PARENT (ap)->menubar));
      font_menu = (GtkWidget *) g_list_nth_data (gl, FONT_MENU);
      font_submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (font_menu));

      gl = gtk_container_get_children (GTK_CONTAINER (font_submenu));
      font_sel = (GtkWidget *) g_list_nth_data (gl, SEN_PARENT (ap)->font);

      font_small = (GtkWidget *) g_list_first (gl)->data;
      font_medium = (GtkWidget *) g_list_first (gl)->next->data;
      font_large = (GtkWidget *) g_list_first (gl)->next->next->data;
    }

  switch (menu_id)
    {
    case MENU_NEW:
      ret = gui_new (ap);
      break;

    case MENU_OPEN:
      ret = gui_open (SEN_PARENT (ap)->window);
      break;

    case MENU_SAVE:
      ret = gui_save (ap, 0);
      if (ret < 0)
	return -1;

      aris_proof_set_sb (ap, _("Proof Saved."));
      break;

    case MENU_SAVE_AS:
      ret = gui_save (ap, 1);
      if (ret < 0)
	return -1;

      aris_proof_set_sb (ap, _("Proof Saved."));
      break;

    case MENU_CLOSE:
      gui_destroy (ap);
      break;

    case MENU_QUIT:
      app_quit ();
      break;

    case MENU_ADD_PREM:
      if (the_app->verbose)
	printf ("Inserting premise\n");

      // The second time, it happened in here.
      // double free or corruption
      // pango_glyph_string_free
      // gtk_text_layout_free_line_display
      sen = aris_proof_create_new_prem (ap);
      if (!sen)
	return -1;

      aris_proof_set_sb (ap, _("Premise Created."));
      break;

    case MENU_ADD_CONC:
      sen = aris_proof_create_new_conc (ap);
      if (!sen)
	return -1;

      aris_proof_set_sb (ap, _("Conclusion Created."));
      break;

    case MENU_ADD_SUB:
      sen = aris_proof_create_new_sub (ap);
      if (!sen)
	return -1;

      aris_proof_set_sb (ap, _("Subproof Created."));
      break;

    case MENU_END_SUB:
      sen = aris_proof_end_sub (ap);
      if (!sen)
	return -1;

      aris_proof_set_sb (ap, _("Subproof Ended."));
      break;

    case MENU_COPY:
      ret = aris_proof_copy (ap);
      if (ret < 0)
	return -1;

      aris_proof_set_sb (ap, _("Sentence Copied."));
      break;

    case MENU_KILL:
      ret = aris_proof_kill (ap);
      if (ret < 0)
	return -1;

      if (ret == 1)
	aris_proof_set_sb (ap, _("The first sentence can not be killed."));
      else
	aris_proof_set_sb (ap, _("Sentence Killed."));
      break;

    case MENU_INSERT:
      ret = aris_proof_yank (ap);
      aris_proof_set_sb (ap, _("Sentence Inserted."));
      break;

    case MENU_EVAL_LINE:
      ls_clear (ap->vars);
      evaluate_line (ap, SENTENCE (SEN_PARENT (ap)->focused->value));
      break;

    case MENU_EVAL_PROOF:
      ret = evaluate_proof (ap);
      break;

    case MENU_GOAL:
      gui_goal_check (ap);
      break;

      /*
    case MENU_SUBMIT:
      gui_submit_show (ap);
      break;
      */

    case MENU_BOOLEAN:
      aris_proof_toggle_boolean_mode (ap);
      break;

    case MENU_IMPORT:
      aris_proof_import_proof (ap);
      break;

    case MENU_TOGGLE_RULES:
      gui_toggle_rules (ap);
      break;

    case MENU_SMALL:
      if (SEN_PARENT (ap)->font != FONT_TYPE_SMALL)
	{
	  gtk_widget_set_sensitive (font_sel, TRUE);
	  aris_proof_set_font (ap, FONT_TYPE_SMALL);
	  aris_proof_set_sb (ap, _("Font Type Set to Small."));
	  gtk_widget_set_sensitive (font_small, FALSE);
	}

      break;

    case MENU_MEDIUM:
      if (SEN_PARENT (ap)->font != FONT_TYPE_MEDIUM)
	{
	  gtk_widget_set_sensitive (font_sel, TRUE);
	  aris_proof_set_font (ap, FONT_TYPE_MEDIUM);
	  aris_proof_set_sb (ap, _("Font Type Set to Medium."));
	  gtk_widget_set_sensitive (font_medium, FALSE);
	}
      break;

    case MENU_LARGE:
      if (SEN_PARENT (ap)->font != FONT_TYPE_LARGE)
	{
	  gtk_widget_set_sensitive (font_sel, TRUE);
	  aris_proof_set_font (ap, FONT_TYPE_LARGE);
	  aris_proof_set_sb (ap, _("Font Type Set to Large."));
	  gtk_widget_set_sensitive (font_large, FALSE);
	}
      break;

    case MENU_CUSTOM:
      if (gtk_widget_get_sensitive (font_sel))
	{
	  FONT_GET_SIZE (the_app->fonts[FONT_TYPE_CUSTOM], cur_font);
	}
      else
	cur_font = 0;

      new_font = gui_set_custom (SEN_PARENT (ap)->window, cur_font);

      if (new_font > 0)
	{

	  gtk_widget_set_sensitive (font_sel, TRUE);
	  if (the_app->fonts[FONT_TYPE_CUSTOM])
	    pango_font_description_free (the_app->fonts[FONT_TYPE_CUSTOM]);
	  INIT_FONT (the_app->fonts[FONT_TYPE_CUSTOM], new_font);
	  aris_proof_set_font (ap, FONT_TYPE_CUSTOM);
	}
      break;

    case MENU_CONTENTS:
      ret = gui_help ();
      if (ret < 0)
	return -1;

      aris_proof_set_sb (ap, _("Displaying Help."));
      break;


    case MENU_ABOUT:
      gui_about (SEN_PARENT (ap)->window);
      break;
    }

  return 0;
}

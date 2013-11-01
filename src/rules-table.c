/* Functions for handling the rules table.

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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rules-table.h"
#include "rules.h"
#include "app.h"
#include "sentence.h"
#include "aris-proof.h"
#include "callbacks.h"
#include "sentence.h"
#include "sen-parent.h"
#include "menu.h"
#include "list.h"
#include "proof.h"
#include "aio.h"
#include "conf-file.h"

/* Begin menu arrays */
static const char * menu_heads[] =
  {
    N_("File"), N_("Font"), N_("Help")
  };

static int num_subs[3] = { 6, 4, 3 };

/* End of menu arrays */

/* Initializes a rules group.
 *  input:
 *    num_rules - the number of rules in this group.
 *    label - the label of this rules group.
 *    parent - the parent of this rules group (the rules table).
 *  output:
 *    the newly intialized rules group, or NULL on error.
 */
rules_group *
rules_group_init (int num_rules, char * label, rules_table * parent)
{
  rules_group * rg;
  rg = (rules_group *) calloc (1, sizeof (rules_group));
  if (!rg)
    {
      perror (NULL);
      return NULL;
    }

  rg->frame = gtk_frame_new (label);
  gtk_box_pack_start (GTK_BOX (parent->layout), rg->frame, FALSE, FALSE, 0);
  rg->table = gtk_table_new (num_rules / 4, 4, TRUE);
  gtk_container_add (GTK_CONTAINER (rg->frame), rg->table);

  return rg;
}

/* Initializes the rules for each rules group.
 *  input:
 *    rt - the rules table that is being initialized.
 *    modif - the offset from the start of the rules list at which to start.
 *    table - the table that belongs to the current rules group.
 *    limit - the amount of rules in this rules group.
 *    sens - whether or not to set the rules to be insensitive.
 *  output:
 *    none.
 */
void
rules_table_help_init (rules_table * rt, int modif,
		       GtkWidget * table, int limit, int sens)
{
  int i, mod;
  for (i = 0; i < limit; i++)
    {
      mod = i + modif;
      rt->rules[mod] = gtk_toggle_button_new_with_label (rules_list[mod]);
      gtk_table_attach_defaults (GTK_TABLE (table), rt->rules[mod],
				 (i % 4), (i % 4) + 1, i / 4, (i / 4) + 1);
      g_signal_connect (G_OBJECT (rt->rules[mod]), "toggled",
			G_CALLBACK (toggled), GINT_TO_POINTER (mod));
      gtk_widget_set_tooltip_text (rt->rules[mod], rules_names[mod]);
      if (sens)
	gtk_widget_set_sensitive (rt->rules[mod], FALSE);
    }
}

/* Initializes a rules table.
 *  input:
 *    boolean - whether or not this is in boolean mode.
 *  output:
 *    the newly initialized rules table, or NULL on error.
 */
rules_table *
rules_table_init (int boolean)
{
  rules_table * rt;
  int i, mod;

  rt = (rules_table *) calloc (1, sizeof (rules_table));
  if (!rt)
    {
      perror (NULL);
      return NULL;
    }

  rt->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  rt->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (rt->window), rt->accel);

  rt->vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (rt->window), rt->vbox);

  rt->menubar = gtk_menu_bar_new ();
  rules_table_create_menu (rt);
  gtk_accel_map_add_entry ("<ARIS-WINDOW>/Contents", GDK_KEY_F1, 0);

  gtk_box_pack_start (GTK_BOX (rt->vbox), rt->menubar, FALSE, FALSE, 0);

  rt->layout = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (rt->vbox), rt->layout, FALSE, FALSE, 0);

  rt->boolean = boolean;

  rt->infer = rules_group_init (8, "Inference", rt);
  if (!rt->infer)
    return NULL;

  rt->equiv = rules_group_init (10, "Equivalence", rt);
  if (!rt->equiv)
    return NULL;

  rt->pred = rules_group_init (9, "Predicate", rt);
  if (!rt->pred)
    return NULL;

  rt->misc = rules_group_init (4, "Miscellaneous", rt);
  if (!rt->misc)
    return NULL;

  rt->boole = rules_group_init (4, "Boolean", rt);
  if (!rt->boole)
    return NULL;

  rules_table_help_init (rt, 0, rt->infer->table, 8, boolean);
  rules_table_help_init (rt, 8, rt->equiv->table, 10, 0);
  rules_table_help_init (rt, 18, rt->pred->table, 9, boolean);
  rules_table_help_init (rt, 27, rt->misc->table, 4, boolean);
  rules_table_help_init (rt, 31, rt->boole->table, 4, 0);

  if (boolean)
    {
      gtk_widget_set_sensitive (rt->rules[RULE_IM], FALSE);
      gtk_widget_set_sensitive (rt->rules[RULE_EQ], FALSE);
      gtk_widget_set_sensitive (rt->rules[RULE_EP], FALSE);
    }


  g_signal_connect (rt->window, "delete-event",
		    G_CALLBACK (rules_table_deleted), NULL);

  g_signal_connect (G_OBJECT (rt->window), "focus-in-event",
		    G_CALLBACK (rules_table_focus), NULL);

  g_signal_connect (G_OBJECT (rt->window), "window-state-event",
		    G_CALLBACK (rules_table_state), rt);

  rt->toggled = -1;
  rt->user = 1;

  gtk_window_set_resizable (GTK_WINDOW (rt->window), 0);
  gtk_window_set_title (GTK_WINDOW (rt->window), "Rules Table");

  return rt;
}

/* Destroys a rules table.
 *  input:
 *    rt - the rules table to destroy.
 *  output:
 *    none.
 */
void
rules_table_destroy (rules_table * rt)
{
  rt->toggled = -1;
  gtk_widget_destroy (rt->window);
}

/* Toggles a rule.
 *  input:
 *    index - the rules index of the rule being toggled.
 *  output:
 *    none.
 */
void
rule_toggled (int index)
{
  sentence * sen;
  int user;

  if (the_app->focused)
    sen = SEN_PARENT (the_app->focused)->focused->value;
  else
    sen = NULL;

  user = the_app->rt->user;

  // The current rule is being untoggled.

  if (index == the_app->rt->toggled)
    {
      the_app->rt->toggled = -1;

      if (user)
	{
	  // Only change the currently focused rule if the user
	  // initiated this.

	  if (the_app->focused)
	    {
	      sen->rule = -1;
	      gtk_label_set_text (GTK_LABEL (sen->rule_box), NULL);

	      undo_info ui;
	      ui.type = -1;
	      int ret = 0;
	      ret = aris_proof_set_changed (the_app->focused, 1, ui);
	      if (ret < 0)
		return;

	      // If the sentence has a file, free the memory used by it.
	      if (sen->file)
		{
		  GtkWidget * menu_item, * menu, * submenu;
		  GList * gl, * gl_itr;
		  gl = gtk_container_get_children (GTK_CONTAINER (SEN_PARENT (the_app->focused)->menubar));
		  menu = g_list_nth_data (gl, RULES_MENU);
		  submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu));

		  gl_itr = gtk_container_get_children (GTK_CONTAINER (submenu));

		  GtkWidget * new_menu;

		  new_menu = gtk_menu_new ();

		  for (; gl_itr; gl_itr = g_list_next (gl_itr))
		    {
		      if (GTK_IS_SEPARATOR_MENU_ITEM (gl_itr->data))
			continue;

		      const char * label =
			gtk_menu_item_get_label (GTK_MENU_ITEM (gl_itr->data));
		      int chk, line_num;
		      chk = sscanf (label, "%i", &line_num);
		      if (chk != 1)
			continue;

		      if (line_num == sen->line_num)
			{
			  gtk_widget_destroy (gl_itr->data);
			  break;
			}
		    }

		  if (sen->proof)
		    proof_destroy (sen->proof);
		  free (sen->file);
		  sen->file = NULL;
		}
	    }
	}

      return;
    }

  // The currently focused sentence is a premise,
  // so just untoggle the old rule.

  if (!sen || sen->premise)
    {
      the_app->rt->toggled = index;
      the_app->rt->user = 0;
      TOGGLE_BUTTON (the_app->rt->rules[index]);
      the_app->rt->user = user;
      return;
    }

  // Now that the rule is being changed, untoggle the old rule.

  if (the_app->rt->toggled != -1)
    {
      the_app->rt->user = 0;
      TOGGLE_BUTTON (the_app->rt->rules[the_app->rt->toggled]);
      the_app->rt->user = user;
    }

  // If the user is initiating this,
  // then set the rule for the currently focused sentence.

  if (user)
    {
      sen->rule = index;
      gtk_label_set_text (GTK_LABEL (sen->rule_box), rules_list[index]);

      undo_info ui;
      ui.type = -1;
      int ret = aris_proof_set_changed (the_app->focused, 1, ui);
      if (ret < 0)
	return;

      if (index == RULE_LM)
	{
	  // Evaluate the external file.
	  // Bring up an open file dialog box.
	  // Grab the file, and run aio_open on it.
	  // Possibly set a menu item in the rules menu for it.
	  GtkFileFilter * file_filter;
	  file_filter = gtk_file_filter_new ();
	  gtk_file_filter_set_name (file_filter, "Aris Files");
	  gtk_file_filter_add_pattern (file_filter, "*.tle");

	  GtkWidget * file_chooser;
	  file_chooser =
	    gtk_file_chooser_dialog_new ("Select a file to Open...",
					 GTK_WINDOW (the_app->rt->window),
					 GTK_FILE_CHOOSER_ACTION_OPEN,
					 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					 NULL);
	  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (file_chooser),
						FALSE);
	  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (file_chooser), file_filter);

	  if (gtk_dialog_run (GTK_DIALOG (file_chooser)) == GTK_RESPONSE_ACCEPT)
	    {
	      char * filename;
	      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));

	      int file_len;
	      file_len = strlen (filename);
	      sen->file = (unsigned char *) calloc (file_len + 1, sizeof (char));

	      strcpy (sen->file, filename);

	      sen->proof = aio_open (filename);
	      if (!sen->proof)
		{
		  gtk_widget_destroy (file_chooser);
		  return;
		}

	      if (the_app->focused)
		{

		  GtkWidget * menu_item, * menu, * submenu;
		  GList * gl;
		  char * label;

		  label = (char *) calloc (file_len
					   + 4
					   + (int) log10 (sen->line_num)
					   + 1,
					   sizeof (char));

		  sprintf (label, "%i - %s",
			   sen->line_num, sen->file);
		  menu_item = gtk_menu_item_new_with_label (label);

		  gl = gtk_container_get_children (GTK_CONTAINER (SEN_PARENT (the_app->focused)->menubar));
		  menu = g_list_nth_data (gl, RULES_MENU);
		  submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu));
		  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menu_item);
		  gtk_widget_show (menu_item);
		}
	    }
	  else
	    {
	      the_app->rt->toggled = index;
	      the_app->rt->user = 0;
	      TOGGLE_BUTTON (the_app->rt->rules[index]);
	      the_app->rt->user = user;
	    }

	  gtk_widget_destroy (file_chooser);
	}
    }

  the_app->rt->toggled = index;
}

/* Create the menu for the rules table.
 *  input:
 *    rt - the rules table for which to create the menu.
 *  output:
 *    none.
 */
void
rules_table_create_menu (rules_table * rt)
{
  GtkWidget * rtm_file, * rtm_file_sub;
  int toss_me = 0, i, j;

  conf_obj * rule_menus_menu[] = {
    (conf_obj[]) {
      main_menu_conf[CONF_MENU_NEW],
      main_menu_conf[CONF_MENU_OPEN],
      menu_separator,
      main_menu_conf[CONF_MENU_SUBMIT],
      menu_separator,
      main_menu_conf[CONF_MENU_QUIT]
    },
    (conf_obj[]) {
      main_menu_conf[CONF_MENU_SMALL],
      main_menu_conf[CONF_MENU_MEDIUM],
      main_menu_conf[CONF_MENU_LARGE],
      main_menu_conf[CONF_MENU_CUSTOM]
    },
    (conf_obj[]) {
      main_menu_conf[CONF_MENU_CONTENTS],
      main_menu_conf[CONF_MENU_CUSTOMIZE],
      main_menu_conf[CONF_MENU_ABOUT]
    },
  };

  for (i = 0; i < 3; i++)
    {
      rtm_file_sub = gtk_menu_new ();
      rtm_file = gtk_menu_item_new_with_label (menu_heads[i]);
      gtk_menu_set_accel_group (GTK_MENU (rtm_file_sub), rt->accel);

      conf_obj * data_elms = rule_menus_menu[i];

      for (j = 0; j < num_subs[i]; j++)
	{
	  conf_obj data = data_elms[j];
	  
	  construct_menu_item (data, G_CALLBACK (rule_menu_activated), rtm_file_sub, &toss_me);
	}

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (rtm_file), rtm_file_sub);
      gtk_menu_shell_append (GTK_MENU_SHELL (rt->menubar), rtm_file);
    }
}

/* Checks if the rules table has received focus.
 *  input:
 *    none.
 *  output:
 *    0 on success.
 */
int
rules_table_focused ()
{
  if (the_app->focused)
    gtk_window_present (GTK_WINDOW (SEN_PARENT (the_app->focused)->window));
  return 0;
}

/* Aligns the rules table with the focused proof.
 *  input:
 *    rt - the rules table for the proof.
 *    ap - the newly focused proof.
 *  output:
 *    0 on success.
 */
int
rules_table_align (rules_table * rt, aris_proof * ap)
{
  if (rt->window)
    {
      int x, y;
      int width, ap_width;
      int new_x;

      gtk_window_get_position (GTK_WINDOW (SEN_PARENT (ap)->window), &x, &y);
      gtk_window_get_size (GTK_WINDOW (SEN_PARENT (ap)->window), &ap_width, NULL);
      gtk_window_get_size (GTK_WINDOW (rt->window), &width, NULL);

      new_x = (x - (width + 16) < 0)
	? x + ap_width + 16  : x - (width + 16);

      gtk_window_move (GTK_WINDOW (rt->window), new_x, y);

      gtk_window_present (GTK_WINDOW (rt->window));
    }

  return 0;
}

/* Sets the font of the rules table.
 *  input:
 *    rt - the rules table for which the font is being set.
 *    font - the index in the_app->fonts for the new font size.
 *  output:
 *    0 on success.
 */
int
rules_table_set_font (rules_table * rt, int font)
{
  if (rt->font == font && rt->font != FONT_TYPE_CUSTOM)
    return 0;

  int i;

  rt->font = font;

  gtk_widget_modify_font (gtk_frame_get_label_widget (GTK_FRAME (rt->equiv->frame)),
			  the_app->fonts[font]);

  gtk_widget_modify_font (gtk_frame_get_label_widget (GTK_FRAME (rt->infer->frame)),
			  the_app->fonts[font]);

  gtk_widget_modify_font (gtk_frame_get_label_widget (GTK_FRAME (rt->pred->frame)),
			  the_app->fonts[font]);

  gtk_widget_modify_font (gtk_frame_get_label_widget (GTK_FRAME (rt->misc->frame)),
			  the_app->fonts[font]);

  gtk_widget_modify_font (gtk_frame_get_label_widget (GTK_FRAME (rt->boole->frame)),
			  the_app->fonts[font]);

  for (i = 0; i < NUM_RULES; i++)
    {
      GList * gl;
      gl = gtk_container_get_children (GTK_CONTAINER (rt->rules[i]));
      gtk_widget_modify_font (GTK_WIDGET (gl->data), the_app->fonts[font]);
    }



  return 0;
}

/* Sets boolean mode for the rules table.
 *  input:
 *    rt - the rules table object.
 *    boolean - 1 if boolean mode is being enabled, 0 otherwise.
 *  output:
 *    0 on success.
 */
int
rules_table_set_boolean_mode (rules_table * rt, int boolean)
{
  int i, oth_sens;
  oth_sens = (boolean == 1) ? 0 : 1;

  for (i = 0; i < NUM_RULES; i++)
    {
      if (i < RULE_DM || i == RULE_EQ || i == RULE_EP
	  || (i >= RULE_UG && i <= RULE_IN))
	gtk_widget_set_sensitive (rt->rules[i], oth_sens);
    }

  return 0;
}

/* Functions for handling the application structure.

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
#include "app.h"
#include "rules-table.h"
#include "list.h"
#include "aris-proof.h"
#include "callbacks.h"
#include "icon.xpm"
#include "goal.h"
#include "sen-parent.h"

/* Initializes an app structure.
 *  input:
 *    boolean - the boolean flag from the cmd line args.
 *  output:
 *    the new app structure.
 */
aris_app *
init_app (int boolean, int verbose)
{
  aris_app * app = (aris_app *) calloc (1, sizeof (aris_app));
  if (!app)
    {
      perror (NULL);
      return NULL;
    }

  INIT_FONT (app->fonts[FONT_TYPE_SMALL], FONT_SIZE_SMALL);
  INIT_FONT (app->fonts[FONT_TYPE_MEDIUM], FONT_SIZE_MEDIUM);
  INIT_FONT (app->fonts[FONT_TYPE_LARGE], FONT_SIZE_LARGE);

  INIT_COLOR (app->bg_colors[BG_COLOR_WHITE], 255, 255, 255);
  INIT_COLOR (app->bg_colors[BG_COLOR_CYAN], 127, 255, 255);
  INIT_COLOR (app->bg_colors[BG_COLOR_VIOLET], 255, 127, 255);
  INIT_COLOR (app->bg_colors[BG_COLOR_CRIMSON], 127, 0, 0);
  INIT_COLOR (app->bg_colors[BG_COLOR_EMERALD], 0, 127, 0);
  INIT_COLOR (app->bg_colors[BG_COLOR_RED_ORANGE], 255, 13, 0);

  app->icon = gdk_pixbuf_new_from_xpm_data (aris_icon_xpm);

  app->rt = rules_table_init (boolean);
  gtk_window_set_icon (GTK_WINDOW (app->rt->window), app->icon);

  app->rt->boolean = boolean;
  app->boolean = boolean;
  app->verbose = verbose;

  app->working_dir = (char *) calloc (1024, sizeof (char));
  app->help_file = (char *) calloc (1024, sizeof (char));

  app->guis = init_list ();

  return app;
}

/* Sets focus to a gui.
 *  input:
 *    ap - the proof of the gui to set focus to.
 *  output:
 *    0 on success, -1 on error.
 */
int
the_app_set_focus (aris_proof * ap)
{
  if (!ap)
    return -1;

  the_app->focused = ap;

  item_t * itm;
  itm = ls_find (the_app->guis, ap);
  if (!itm)
    return -1;

  // This is already the first element,
  // so no need to re-order the elements.
  if (!itm->prev)
    return 0;

  itm->prev->next = itm->next;
  if (itm->next)
    itm->next->prev = itm->prev;
  else
    the_app->guis->tail = itm->prev;

  item_t * h_itm = the_app->guis->head;

  itm->next = the_app->guis->head;
  itm->prev = NULL;
  the_app->guis->head->prev = itm;
  the_app->guis->head = itm;

  if (the_app->rt)
    {
      rules_table_align (the_app->rt, ap);
      rules_table_set_boolean_mode (the_app->rt, ap->boolean);
    }

  return 0;
}

/* Adds a gui to the app's list.
 *  input:
 *    ap - the proof of the gui to add.
 *  output:
 *    0 on success, -1 on error.
 */
int
the_app_add_gui (aris_proof * ap)
{
  if (!ap)
    return -1;

  item_t * itm;
  itm = ls_ins_obj (the_app->guis, ap, the_app->guis->tail);
  if (!itm)
    return -1;

  gtk_window_set_icon (GTK_WINDOW (SEN_PARENT (ap)->window), the_app->icon);
  gtk_window_set_icon (GTK_WINDOW (ap->goal->window), the_app->icon);

  int ret;
  ret = the_app_set_focus (ap);
  if (ret < 0)
    return -1;

  return 0;
}

/* Removes a gui from the app's list.
 *  input:
 *    ap - the proof of the gui to remove.
 *  output:
 *    none.
 */
void
the_app_rem_gui (aris_proof * ap)
{
  item_t * itm, * n_itm;
  itm = ls_find (the_app->guis, ap);

  if (the_app->guis->num_stuff == 1)
    n_itm = NULL;
  else if (itm == the_app->guis->head)
    n_itm = the_app->guis->head->next;
  else if (itm == the_app->guis->tail)
    n_itm = the_app->guis->tail->prev;
  else
    n_itm = itm->prev;

  ls_rem_obj (the_app->guis, itm);
  if (n_itm)
    the_app_set_focus ((aris_proof *) n_itm->value);
  else
    {
      gtk_widget_show_all (the_app->rt->window);
      the_app->focused = NULL;
      if (the_app->rt->toggled != -1)
	{
	  TOGGLE_BUTTON (the_app->rt->rules [the_app->rt->toggled]);
	}
    }
}

/* Quits out of the application.
 *   input:
 *     None.
 *   output:
 *     0 on success, -1 if canceled.
 */
int
app_quit ()
{
  item_t * g_itr, * ng_itr;

  for (g_itr = the_app->guis->head; g_itr; g_itr = ng_itr)
    {
      ng_itr = g_itr->next;

      if (gui_destroy ((aris_proof *) g_itr->value) < 0)
	return -1;

      g_itr->next = g_itr->prev = g_itr->value = NULL;
      free (g_itr);
    }

  free (the_app->guis);
  the_app->focused = NULL;

  rules_table_destroy (the_app->rt);
  free (the_app->help_file);
  free (the_app->working_dir);

  gtk_main_quit ();

  return 0;
}

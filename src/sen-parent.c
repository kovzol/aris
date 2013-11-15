/* Functions for handling sentence parents.

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
#include "sen-parent.h"
#include "sentence.h"
#include "sen-data.h"
#include "app.h"
#include "list.h"
#include "process.h"

#include "and-conn.xpm"
#include "or-conn.xpm"
#include "not-conn.xpm"
#include "con-conn.xpm"
#include "bic-conn.xpm"
#include "unv-conn.xpm"
#include "exl-conn.xpm"
#include "tau-conn.xpm"
#include "ctr-conn.xpm"
#include "elm-conn.xpm"
#include "nil-conn.xpm"

#define INIT_CONN_PIXBUF(s,i,c,f) {			\
  s->conn_pixbufs[i]					\
  = gdk_pixbuf_scale_simple (the_app->conn_pixbufs[i],	\
			     f, f,			\
			     GDK_INTERP_BILINEAR);	\
  g_object_set_data (G_OBJECT (s->conn_pixbufs[i]),	\
		     _("conn"), c);			\
  }

/* Initializes a sentence parent.
 *  input:
 *    sp - the sentence parent to initialize.
 *    title - the title of the sentence parent's window.
 *    width - the width of the sentence parent's window.
 *    height - the height of the sentence parent's window.
 *    menu_func - initializes the sentence parent's menu bar.
 *    type - the type of sentence parent.
 *  output:
 *    none.
 */
void
sen_parent_init (sen_parent * sp, const char * title,
		 int width, int height,
		 void (* menu_func) (sen_parent *),
		 int type)
{
  sp->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (sp->window), title);
  gtk_widget_set_size_request (sp->window, width, height);

  if (the_app->fonts[FONT_TYPE_CUSTOM])
    sp->font = FONT_TYPE_CUSTOM;
  else
    sp->font = FONT_TYPE_SMALL;

  sen_parent_init_conns (sp);

  sp->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (sp->window), sp->accel);

  sp->vbox = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER (sp->window), sp->vbox);

  // Call menu_func on sp.
  menu_func (sp);
  gtk_grid_attach (GTK_GRID (sp->vbox), sp->menubar, 0, 0, 1, 1);

  // Initialize the scrolled window.
  sp->scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_halign (sp->scrolledwindow, GTK_ALIGN_FILL);
  gtk_widget_set_valign (sp->scrolledwindow, GTK_ALIGN_FILL);
  gtk_widget_set_hexpand (sp->scrolledwindow, TRUE);
  gtk_widget_set_vexpand (sp->scrolledwindow, TRUE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sp->scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_grid_attach (GTK_GRID (sp->vbox), sp->scrolledwindow, 0, 1, 1, 1);

  GtkAdjustment * f_adj;
  f_adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (sp->scrolledwindow));

  // Initialize the viewport.
  sp->viewport = gtk_viewport_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (sp->scrolledwindow), sp->viewport);

  // Initialize the main container.
  sp->container = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (sp->container), 4);
  gtk_container_add (GTK_CONTAINER (sp->viewport), sp->container);
  gtk_container_set_focus_vadjustment (GTK_CONTAINER (sp->container), f_adj);

  // Initialize the status bar.
  sp->statusbar = gtk_statusbar_new ();
  gtk_grid_attach (GTK_GRID (sp->vbox), sp->statusbar, 0, 2, 1, 1);

  sp->everything = init_list ();

  sp->focused = NULL;

  sp->type = type;
  sp->undo = 0;
}

int
sen_parent_init_conns (sen_parent * sp)
{
  int font;

  font
    = pango_font_description_get_size (the_app->fonts[sp->font]);
  font /= PANGO_SCALE;

  INIT_CONN_PIXBUF (sp, 0, AND, font);
  INIT_CONN_PIXBUF (sp, 1, OR, font);
  INIT_CONN_PIXBUF (sp, 2, NOT, font);
  INIT_CONN_PIXBUF (sp, 3, CON, font);
  INIT_CONN_PIXBUF (sp, 4, BIC, font);
  INIT_CONN_PIXBUF (sp, 5, UNV, font);
  INIT_CONN_PIXBUF (sp, 6, EXL, font);
  INIT_CONN_PIXBUF (sp, 7, TAU, font);
  INIT_CONN_PIXBUF (sp, 8, CTR, font);
  INIT_CONN_PIXBUF (sp, 9, ELM, font);
  INIT_CONN_PIXBUF (sp, 10, NIL, font);

  return 0;
}

/* Destroys a sentence parent.
 *  input:
 *    sp - the sentence parent to destroy.
 *  output:
 *    none.
 */
void
sen_parent_destroy (sen_parent * sp)
{
  item_t * ev_itr, * nev_itr;

  for (ev_itr = sp->everything->head; ev_itr; ev_itr = nev_itr)
    {
      nev_itr = ev_itr->next;
      sentence_destroy ((sentence *) ev_itr->value);
      ev_itr->prev = ev_itr->next = ev_itr->value = NULL;
      free (ev_itr);
    }

  free (sp->everything);
  sp->everything = NULL;

  sp->font = -1;
  sp->focused = NULL;
  gtk_widget_destroy (sp->window);
}

/* Sets the font of a sentence parent.
 *  input:
 *    sp - the sentence parent of which to set the font.
 *    new_font - the index in the_app->fonts to which the font is being set.
 *  output:
 *    none.
 */
void
sen_parent_set_font (sen_parent * sp, int new_font)
{
  if (sp->font == new_font && new_font != FONT_TYPE_CUSTOM)
    return;

  // Resize the connectives.

  int font;

  font = pango_font_description_get_size (the_app->fonts[new_font]);
  font /= PANGO_SCALE;

  int i;
  GdkPixbuf * old_conns[NUM_CONNS];

  double w, h;
  w = h = 1.0;

  for (i = 0; i < NUM_CONNS; i++)
    {
      old_conns[i] = sp->conn_pixbufs[i];

      if (i == 5 || i == 6)
	{
	  w = 1.0;
	  h = 1.5;
	}

      sp->conn_pixbufs[i] = gdk_pixbuf_scale_simple
	(the_app->conn_pixbufs[i], ((double) font) * w,
	 ((double) font) * h, GDK_INTERP_BILINEAR);

      g_object_set_data (G_OBJECT (sp->conn_pixbufs[i]),
			 _("conn"), (gpointer) conn_list[i]);
    }

  item_t * ev_itr;

  for (ev_itr = sp->everything->head; ev_itr; ev_itr = ev_itr->next)
    sentence_set_font (ev_itr->value, new_font);

  sp->font = new_font;
}

GdkPixbuf *
sen_parent_get_conn_by_type (sen_parent * sp, char * type)
{
  GdkPixbuf * ret = NULL;
  int i;
  for (i = 0; i < NUM_CONNS; i++)
    {
      if (!strcmp (type, conn_list[i])
	  || !strcmp (type, conn_list_back[i]))
	break;
    }

  ret = (i == NUM_CONNS) ? NULL : sp->conn_pixbufs[i];
  return ret;
}

/* Inserts a sentence into a sentence parent.
 *  input:
 *    sp - the sentence parent into which the sentence is being inserted.
 *    sd - the sentence data from which to initialize the sentence.
 *    fcs - the item after which to insert the new sentence.
 *    new_order - the new order in the container in which this sentence will be.
 *  output:
 *    the iterator in sp->everything that the sentence will be, or NULL on error.
 */
item_t *
sen_parent_ins_sentence (sen_parent * sp, sen_data * sd,
			 item_t * fcs, int new_order)
{
  item_t * itm;
  sentence * sen;

  sen = sentence_init (sd, sp, fcs);
  if (!sen)
    return NULL;

  if (sp->type == SEN_PARENT_TYPE_PROOF && !SEN_PREM(sen))
    new_order = sentence_get_line_no (sen);

  itm = ls_ins_obj (sp->everything, sen, fcs);
  sp->focused = itm;

  gtk_grid_insert_row (GTK_GRID (sp->container), new_order);
  gtk_grid_attach (GTK_GRID (sp->container), sen->panel, 0, new_order, 1, 1);

  gtk_widget_show_all (sen->panel);

  return itm;
}

/* Removes a sentence from a sentence parent.
 *  input:
 *    sp - the sentence parent from which a sentence is being removed.
 *    sen - the sentence that is being removed.
 *  output:
 *    the item that was removed.
 */
item_t *
sen_parent_rem_sentence (sen_parent * sp, sentence * sen)
{
  item_t * ev_itr, * target = NULL;
  int row_num;

  row_num = sentence_get_line_no (sen);
  target = ls_find (sp->everything, sen);

  // Only need to start this past the target sentence.
  for (ev_itr = target->next; ev_itr; ev_itr = ev_itr->next)
    {
      sentence * ev_sen = ev_itr->value;

      sentence_rem_ref (ev_sen, sen);
    }

  item_t * new_focus;
  if (sp->everything->num_stuff == 1)
    new_focus = NULL;
  else if (target == sp->everything->head)
    new_focus = sp->everything->head->next;
  else if (target == sp->everything->tail)
    new_focus = sp->everything->tail->prev;
  else
    new_focus = target->prev;

  if (target == sp->focused)
    {
      if (new_focus)
	sen_parent_set_focus (sp, new_focus);
    }

  ls_rem_obj (sp->everything, target);
  sentence_destroy (sen);

  gtk_grid_remove_row (GTK_GRID (sp->container), row_num);

  return new_focus;
}

/* Sets an item as the focus of a sentence parent.
 *  input:
 *    sp - the sentence parent which is having its focus changed.
 *    focus - the new focus.
 *  output:
 *    none.
 */
void
sen_parent_set_focus (sen_parent * sp, item_t * focus)
{
  sp->focused = focus;
  gtk_widget_grab_focus (((sentence *) focus->value)->entry);
}

/* Sets the status bar message of a sentence parent.
 *  input:
 *    sp - the sentence parent for which to set the status bar text.
 *    sb_text - the status bar text being set.
 *  output:
 *    none.
 */
void
sen_parent_set_sb (sen_parent * sp, char * sb_text)
{
  unsigned int context_id;
  context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (sp->statusbar),
					     "feedback");
  gtk_statusbar_push (GTK_STATUSBAR (sp->statusbar),
		      context_id,
		      (const char *) sb_text);
}

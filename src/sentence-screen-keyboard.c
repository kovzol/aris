/* Functions for handling the screen keyboard. 

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

#include "app.h"
#include "aris-proof.h"
#include "goal.h"
#include "sentence-screen-keyboard.h"
#include "sen-parent.h"


void callback_and_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data)
{
  callback_sentence_screen_keyboard(widget, def_event, data, AND_TYPE);
}

void callback_or_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data)
{
  callback_sentence_screen_keyboard(widget, def_event, data, OR_TYPE);
}

void callback_not_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data)
{
  callback_sentence_screen_keyboard(widget, def_event, data, NOT_TYPE);
}

void callback_con_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data)
{
  callback_sentence_screen_keyboard(widget, def_event, data, CON_TYPE);
}

void callback_bic_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data)
{
  callback_sentence_screen_keyboard(widget, def_event, data, BIC_TYPE);
}

void callback_unv_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data)
{
  callback_sentence_screen_keyboard(widget, def_event, data, UNV_TYPE);
}

void callback_exl_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data)
{
  callback_sentence_screen_keyboard(widget, def_event, data, EXL_TYPE);
}

void callback_tau_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data)
{
  callback_sentence_screen_keyboard(widget, def_event, data, TAU_TYPE);
}

void callback_ctr_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data)
{
  callback_sentence_screen_keyboard(widget, def_event, data, CTR_TYPE);
}

void callback_elm_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data)
{
  callback_sentence_screen_keyboard(widget, def_event, data, ELM_TYPE);
}

void callback_nil_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data)
{
  callback_sentence_screen_keyboard(widget, def_event, data, NIL_TYPE);
}

void callback_sentence_screen_keyboard(GtkWidget *widget, GdkEvent *def_event, gpointer *data, int type)
{
  sen_parent *sp = (sen_parent *)data;
  
  if (the_app->focused)
  {
	  if (the_app->focused->goal->toggle)
		  sp = SEN_PARENT (the_app->focused->goal);
	  else
		  sp = SEN_PARENT (the_app->focused);
  }

  GdkEvent *event = gdk_event_new(GDK_KEY_PRESS);

  event->key.window = gtk_widget_get_window(sp->window);
  event->key.send_event = FALSE;
  event->key.time = 0;
  event->key.state = GDK_CONTROL_MASK;
  switch (type)
  {
  case AND_TYPE:
    event->key.keyval = GDK_KEY_7;
    event->key.string = g_strdup("7");
    break;
  case OR_TYPE:
    event->key.keyval = GDK_KEY_backslash;
    event->key.string = g_strdup("\\");
    break;
  case NOT_TYPE:
    event->key.keyval = GDK_KEY_grave;
    event->key.string = g_strdup("^");
    break;
  case CON_TYPE:
    event->key.keyval = GDK_KEY_4;
    event->key.string = g_strdup("4");
    break;
  case BIC_TYPE:
    event->key.keyval = GDK_KEY_5;
    event->key.string = g_strdup("5");
    break;
  case UNV_TYPE:
    event->key.keyval = GDK_KEY_2;
    event->key.string = g_strdup("2");
    break;
  case EXL_TYPE:
    event->key.keyval = GDK_KEY_3;
    event->key.string = g_strdup("3");
    break;
  case TAU_TYPE:
    event->key.keyval = GDK_KEY_1;
    event->key.string = g_strdup("1");
    break;
  case CTR_TYPE:
    event->key.keyval = GDK_KEY_6;
    event->key.string = g_strdup("6");
    break;
  case ELM_TYPE:
    event->key.keyval = GDK_KEY_semicolon;
    event->key.string = g_strdup(";");
    break;
  case NIL_TYPE:
    event->key.keyval = GDK_KEY_period;
    event->key.string = g_strdup(".");
    break;
  default:; // should not happen
  }

  event->key.length = strlen(event->key.string);
  gtk_main_do_event(event);
  gdk_event_free(event);
  gtk_window_present(GTK_WINDOW(sp->window)); // Set sentence window focus
}

GtkWidget *init_button(sen_parent *sp, GtkWidget *grid, char *type, int r, int c)
{
  GdkPixbuf *symbol_pixbuf = gdk_pixbuf_scale_simple(sen_parent_get_conn_by_type(sp, type), 12, 12, GDK_INTERP_BILINEAR);
  GtkWidget *image = gtk_image_new_from_pixbuf(symbol_pixbuf);

  GtkWidget *button = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(button), image);
  gtk_grid_attach(GTK_GRID(grid), button, c, r, 1, 1);
  return button;
}

void keyboard_delete(){
	gtk_widget_hide (the_app->keyboard);
}

// Aligns the keyboard to the right of the main window
void align_keyboard (sen_parent *sp)
{

  int x, y;
  int width, ap_width;
  int new_x;

  gtk_window_get_position (GTK_WINDOW (sp->window), &x, &y);
  gtk_window_get_size (GTK_WINDOW (sp->window), &ap_width, NULL);
  gtk_window_get_size (GTK_WINDOW (the_app->keyboard), &width, NULL);
  
  new_x = (width + 16 < x) ?  x + ap_width + 16: x ;

  gtk_window_move (GTK_WINDOW (the_app->keyboard), new_x, y);
}

void init_sentence_screen_keyboard(sen_parent *sp)
{
  GtkWidget *window_sentence_screen_keyboard = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  the_app->keyboard = window_sentence_screen_keyboard;
  gtk_window_set_title(GTK_WINDOW(window_sentence_screen_keyboard), "Keyboard");
  gtk_window_set_default_size(GTK_WINDOW(window_sentence_screen_keyboard), 150, 190);
  gtk_window_set_resizable(GTK_WINDOW(window_sentence_screen_keyboard), FALSE);
  g_signal_connect (the_app->keyboard, "delete-event", G_CALLBACK (keyboard_delete),NULL);

  GtkWidget *grid1 = gtk_grid_new(); // Make a grid for the buttons
  gtk_grid_set_column_spacing((GtkGrid *)grid1, 5);
  gtk_grid_set_row_spacing((GtkGrid *)grid1, 5);

  GtkWidget *button_and = init_button(sp, grid1, AND, 1, 1);
  g_signal_connect(G_OBJECT(button_and), "button-press-event", G_CALLBACK(callback_and_screen_keyboard), (gpointer)sp);
  GtkWidget *button_or = init_button(sp, grid1, OR, 1, 2);
  g_signal_connect(G_OBJECT(button_or), "button-press-event", G_CALLBACK(callback_or_screen_keyboard), (gpointer)sp);
  GtkWidget *button_not = init_button(sp, grid1, NOT, 1, 3);
  g_signal_connect(G_OBJECT(button_not), "button-press-event", G_CALLBACK(callback_not_screen_keyboard), (gpointer)sp);
  GtkWidget *button_con = init_button(sp, grid1, CON, 2, 1);
  g_signal_connect(G_OBJECT(button_con), "button-press-event", G_CALLBACK(callback_con_screen_keyboard), (gpointer)sp);
  GtkWidget *button_bic = init_button(sp, grid1, BIC, 2, 2);
  g_signal_connect(G_OBJECT(button_bic), "button-press-event", G_CALLBACK(callback_bic_screen_keyboard), (gpointer)sp);
  GtkWidget *button_unv = init_button(sp, grid1, UNV, 3, 1);
  g_signal_connect(G_OBJECT(button_unv), "button-press-event", G_CALLBACK(callback_unv_screen_keyboard), (gpointer)sp);
  GtkWidget *button_exl = init_button(sp, grid1, EXL, 3, 2);
  g_signal_connect(G_OBJECT(button_exl), "button-press-event", G_CALLBACK(callback_exl_screen_keyboard), (gpointer)sp);
  GtkWidget *button_tau = init_button(sp, grid1, TAU, 4, 1);
  g_signal_connect(G_OBJECT(button_tau), "button-press-event", G_CALLBACK(callback_tau_screen_keyboard), (gpointer)sp);
  GtkWidget *button_ctr = init_button(sp, grid1, CTR, 4, 2);
  g_signal_connect(G_OBJECT(button_ctr), "button-press-event", G_CALLBACK(callback_ctr_screen_keyboard), (gpointer)sp);
  GtkWidget *button_elm = init_button(sp, grid1, ELM, 5, 1);
  g_signal_connect(G_OBJECT(button_elm), "button-press-event", G_CALLBACK(callback_elm_screen_keyboard), (gpointer)sp);
  GtkWidget *button_nil = init_button(sp, grid1, NIL, 5, 2);
  g_signal_connect(G_OBJECT(button_nil), "button-press-event", G_CALLBACK(callback_nil_screen_keyboard), (gpointer)sp);
  gtk_container_add(GTK_CONTAINER(window_sentence_screen_keyboard), grid1);
  gtk_widget_show_all(window_sentence_screen_keyboard);
  align_keyboard(sp);
}

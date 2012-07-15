/* The construct_menu_item function.

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

#include "menu.h"

/* Constructs a menu item from given data.
 *  input:
 *    data - the menu item data to construct this menu item from.
 *    func - the callback function to call when this item is activated.
 *    parent - the parent window of the menu item.
 *    accel - the accelerator group of the parent window.
 *    got_radio - keeps track of how many 'radio items' there are in the group.
 *  output:
 *    returns the newly construced GtkMenuItem.
 */
GtkWidget *
construct_menu_item (mid_t data, GCallback func, GtkWidget * parent, GtkAccelGroup * accel, int * got_radio)
{
  GtkWidget * item, * image;

  if (data.type == SEPARATOR)
    {
      item = gtk_separator_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (parent), item);
      return item;
    }

  item = gtk_image_menu_item_new_with_label (data.label);
  gtk_widget_set_tooltip_text (item, data.tooltip);
  image = gtk_image_new_from_stock (data.stock_id, GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
  if (data.keyval > 0)
    {
      gtk_widget_add_accelerator (item, "activate",
				  accel, data.keyval, data.mods,
				  GTK_ACCEL_VISIBLE);
    }
  g_signal_connect (item, "activate",
		    G_CALLBACK (func),
		    GINT_TO_POINTER (data.id));

  if (*got_radio == 0 && data.type == RADIO)
    {
      gtk_widget_set_sensitive (item, FALSE);
      (*got_radio)++;
    }

  gtk_menu_shell_append (GTK_MENU_SHELL (parent), item);

  return item;
}

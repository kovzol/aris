/* The construct_menu_item function.

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

#include "process.h"
#include "menu.h"
#include "conf-file.h"

/* Constructs a menu item from given data.
 *  input:
 *    data - the configuration object to get the data from.
 *    func - the callback function to call when this item is activated.
 *    parent - the parent menu shell of the menu item.
 *    got_radio - keeps track of how many 'radio items' there are in the group.
 *  output:
 *    returns the newly construced GtkMenuItem.
 */
GtkWidget *
construct_menu_item (conf_obj data, GCallback func,
		     GtkWidget * parent, int * got_radio)
{
  GtkWidget * item, * image;
  char * path;
  int alloc_size;

  if (!data.label)
    {
      item = gtk_separator_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (parent), item);
      return item;
    }

  if (data.stock_id)
    {
      item = gtk_image_menu_item_new_with_label (data.label);
      image = gtk_image_new_from_stock (data.stock_id,
					GTK_ICON_SIZE_MENU);
      gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
				     image);
    }
  else
    {
      item = gtk_menu_item_new_with_label (data.label);
    }

  gtk_widget_set_tooltip_text (item, data.tooltip);

  alloc_size = strlen (WIN_PATH) + strlen (data.label) + 3;
  path = (char *) calloc (alloc_size + 1, sizeof (char));
  CHECK_ALLOC (path, NULL);
  sprintf (path, "<%s>/%s", WIN_PATH, data.label);
  gtk_menu_item_set_accel_path (GTK_MENU_ITEM (item), path);

  g_signal_connect (item, "activate",
		    G_CALLBACK (func),
		    GINT_TO_POINTER (data.id));

  if (*got_radio == 0
      && (data.id >= MENU_SMALL && data.id <= MENU_CUSTOM))
    {
      gtk_widget_set_sensitive (item, FALSE);
      (*got_radio)++;
    }

  gtk_menu_shell_append (GTK_MENU_SHELL (parent), item);

  return item;
}

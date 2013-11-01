/*  Menu data types.

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

#ifndef ARIS_MENU_H
#define ARIS_MENU_H

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "typedef.h"

#define WIN_PATH "ARIS-WINDOW"

GtkWidget * construct_menu_item (conf_obj data, GCallback func,
				 GtkWidget * parent, int * got_radio);

#endif  /*  ARIS_MENU_H  */

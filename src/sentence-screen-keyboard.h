/* The sentence screen keyboard.

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

#ifndef ARIS_SENTENCE_SCREEN_KEYBOARD_H
#define ARIS_SENTENCE_SCREEN_KEYBOARD_H

#include <gtk/gtk.h>

#define AND_TYPE 1
#define OR_TYPE 2
#define NOT_TYPE 3
#define CON_TYPE 4
#define BIC_TYPE 5
#define UNV_TYPE 6
#define EXL_TYPE 7
#define TAU_TYPE 8
#define CTR_TYPE 9
#define ELM_TYPE 10
#define NIL_TYPE 11





void init_sentence_screen_keyboard(sen_parent *sp);
void callback_sentence_screen_keyboard(GtkWidget *widget, GdkEvent * event, gpointer *data, int type);
void align_keyboard (sen_parent *sp);
#endif  /*  ARIS_SENTENCE_SCREEN_KEYBOARD_H  */

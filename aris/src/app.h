/* Defines the app data structure.

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

#ifndef ARIS_APP_H
#define ARIS_APP_H

#include "pound.h"

typedef struct aris_app aris_app;
typedef struct aris_proof aris_proof;
typedef struct list list_t;
typedef struct item item_t;
typedef struct rules_table rules_table;

// Font sizes, in points.
enum FONT_SIZE {
  FONT_SIZE_SMALL = 8,
  FONT_SIZE_MEDIUM = 16,
  FONT_SIZE_LARGE = 24
};

// The index of each background color in the_app->bg_colors.
enum BG_COLORS {
  BG_COLOR_WHITE = 0,
  BG_COLOR_CYAN,
  BG_COLOR_VIOLET,
  BG_COLOR_CRIMSON,
  BG_COLOR_EMERALD,
  BG_COLOR_RED_ORANGE,
  NUM_BG_COLORS
};

// The index of each font type in the_app->fonts.
enum FONT_TYPES {
  FONT_TYPE_SMALL = 0,
  FONT_TYPE_MEDIUM,
  FONT_TYPE_LARGE,
  FONT_TYPE_CUSTOM,
  NUM_FONT_TYPES
};

// The structure for the main app.

struct aris_app {

  FONT_TYPE fonts[NUM_FONT_TYPES];      // Keeps track of each font size.

  COLOR_TYPE bg_colors[NUM_BG_COLORS];  // Keeps track of each background color.

  GdkPixbuf * icon;      // The icon of the application.
  list_t * guis;         // The list of guis in the application
  aris_proof * focused;  // The focused gui.
  rules_table * rt;      // The rules table.

  char * working_dir;    // The working directory when aris was called.
  char * help_file;      // The help file location.

  int boolean : 1;
  int verbose : 1;
};

aris_app * init_app (int boolean, int verbose);
int the_app_set_focus (aris_proof * ag);
int the_app_add_gui (aris_proof * ag);
void the_app_rem_gui (aris_proof * ag);
int app_quit ();

/* The main application */
aris_app * the_app;

#endif /* ARIS_APP_H */

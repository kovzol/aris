/* Defines the app data structure.

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

#ifndef ARIS_APP_H
#define ARIS_APP_H

#include "pound.h"
#include "typedef.h"
#include "process.h"

// Font sizes, in points.
enum FONT_SIZE {
  FONT_SIZE_SMALL = 8,
  FONT_SIZE_MEDIUM = 16,
  FONT_SIZE_LARGE = 24
};

// The index of each background color in the_app->bg_colors.
enum BG_COLORS {
  BG_COLOR_DEFAULT = 0,
  BG_COLOR_CONC,
  BG_COLOR_REF,
  BG_COLOR_BAD,
  BG_COLOR_GOOD,
  BG_COLOR_SEL,
  NUM_BG_COLORS
};

// The index of each font type in the_app->fonts.
// This will eventually be removed.
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

  GdkPixbuf * conn_pixbufs[NUM_CONNS];   /* Holds the default
                                     connective pixbufs. */

  GdkPixbuf * icon;      // The icon of the application.
  list_t * guis;         // The list of guis in the application
  aris_proof * focused;  // The focused gui.
  rules_table * rt;      // The rules table.

  char * working_dir;    // The working directory when aris was called.
  char * help_file;      // The help file location.
  char * ip_addr;        // The IP Address of the grade server.
  char * grade_pass;     // The password to the grade server.
  char * grade_dir;      // The directory in the ftp server to cd to.

  int boolean : 1;  // Whether boolean mode was specified.
  int verbose : 1;  // Whether verbose was specified.
};

// The structure for submission entries.
// Might not need this anymore.

struct submit_ent {
  char * file_name;
  char * hw;
};

#define CONF_FILE ".aris"

aris_app * init_app (int boolean, int verbose);
int the_app_read_config_file (aris_app * app);
int the_app_read_default_config (aris_app * app);
int the_app_make_default_config_file (char * path);
int the_app_set_focus (aris_proof * ag);
int the_app_add_gui (aris_proof * ag);
void the_app_rem_gui (aris_proof * ag);
int app_quit ();
int the_app_submit (const char * user_email, const char * instr_email,
		    struct submit_ent * entries);

GdkPixbuf * the_app_get_conn_by_type (char * type);
int the_app_init_conn_pixbufs (aris_app * app);
int the_app_get_color_by_type (aris_app * app, char * type);
char * the_app_get_color_by_index (aris_app * app, int index);
int the_app_get_font_by_name (aris_app * app, char * name);

/* The main application */
aris_app * the_app;

#endif /* ARIS_APP_H */

/* Defines routines to read from the configuration file.

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

#ifndef ARIS_CONF_FILE_H
#define ARIS_CONF_FILE_H

#include <stdio.h>
#include <gtk/gtk.h>
#include "typedef.h"
#include "menu.h"
#include "app.h"

/* Configuration objec types */

enum CONF_OBJ_TYPES {
  CONF_OBJ_MENU = 0,
  CONF_OBJ_FONT,
  CONF_OBJ_GRADE,
  CONF_OBJ_COLOR
};

enum CONF_MENU_ID {
  CONF_MENU_NEW=0,
  CONF_MENU_OPEN,
  CONF_MENU_SAVE,
  CONF_MENU_SAVE_AS,
  CONF_MENU_CLOSE,
  CONF_MENU_QUIT,
  CONF_MENU_ADD_PREM,
  CONF_MENU_ADD_CONC,
  CONF_MENU_ADD_SUB,
  CONF_MENU_END_SUB,
  CONF_MENU_COPY,
  CONF_MENU_KILL,
  CONF_MENU_INSERT,
  CONF_MENU_EVAL_LINE,
  CONF_MENU_EVAL_PROOF,
  CONF_MENU_GOAL,
  CONF_MENU_BOOLEAN,
  CONF_MENU_IMPORT,
  CONF_MENU_TOGGLE_RULES,
  CONF_MENU_SMALL,
  CONF_MENU_MEDIUM,
  CONF_MENU_LARGE,
  CONF_MENU_CUSTOM,
  CONF_MENU_ABOUT,
  CONF_MENU_SUBMIT,
  CONF_MENU_CUSTOMIZE,
  CONF_MENU_CONTENTS,
  NUM_CONF_MENUS
};

/* The structure for a configuration object. */
struct conf_object {
  char * label;        // The label of the object.
  char * tooltip;      // The tooltip for the label and widget.
  GtkWidget * widget;  // The widget to display alongside the label.
  int type;            // The type of configuration object this is.
  int id;              // The identification of this object.

  conf_obj_value_func value_func; // The value function.

  char * stock_id;     // The stock item to display.
};

int conf_file_read (const unsigned char * buffer, aris_app * app);

void * conf_menu_value (conf_obj * obj, int get);
void * conf_font_value (conf_obj * obj, int get);
void * conf_color_value (conf_obj * obj, int get);
void * conf_grade_value (conf_obj * obj, int get);

/* The configuration arrays */

/* The main menu configuration objects */

static conf_obj main_menu_conf[NUM_CONF_MENUS] = {
  {N_("New"), N_("Begin a new proof."), NULL, CONF_OBJ_MENU,
   MENU_NEW, conf_menu_value, GTK_STOCK_NEW},

  {N_("Open"), N_("Open a proof."), NULL, CONF_OBJ_MENU, MENU_OPEN,
   conf_menu_value, GTK_STOCK_OPEN},

  {N_("Save"), N_("Save the current proof."), NULL, CONF_OBJ_MENU,
   MENU_SAVE, conf_menu_value,  GTK_STOCK_SAVE},

  {N_("Save As"), N_("Save this proof under a different name."),
   NULL, CONF_OBJ_MENU, MENU_SAVE_AS, conf_menu_value,
   GTK_STOCK_SAVE_AS},

  {N_("Close"), N_("Close the current proof."), NULL, CONF_OBJ_MENU,
   MENU_CLOSE, conf_menu_value, GTK_STOCK_CLOSE},

  {N_("Quit"), N_("Quit GNU Aris."), NULL, CONF_OBJ_MENU, MENU_QUIT,
   conf_menu_value, GTK_STOCK_QUIT},

  {N_("Add Premise"), N_("Add a new premise to the current proof."),
   NULL, CONF_OBJ_MENU, MENU_ADD_PREM, conf_menu_value,
   GTK_STOCK_YES},

  {N_("Add Conclusion"),
   N_("Add a new conclusion to the current proof."), NULL,
   CONF_OBJ_MENU, MENU_ADD_CONC, conf_menu_value, GTK_STOCK_NO},

  {N_("Add Subproof"), N_("Add a new subproof to the current proof."),
   NULL, CONF_OBJ_MENU, MENU_ADD_SUB, conf_menu_value,
   GTK_STOCK_MEDIA_NEXT},

  {N_("End Subproof"), N_("End the current subproof."), NULL,
   CONF_OBJ_MENU, MENU_END_SUB, conf_menu_value,
   GTK_STOCK_MEDIA_PREVIOUS},

  {N_("Copy Line"), N_("Copy the current line in the current proof."),
   NULL, CONF_OBJ_MENU, MENU_COPY, conf_menu_value, GTK_STOCK_COPY},

  {N_("Kill Line"),
   N_("Kill (Cut) the current line in the current proof."), NULL,
   CONF_OBJ_MENU, MENU_KILL, conf_menu_value, GTK_STOCK_CUT},

  {N_("Insert Line"), N_("Insert a copied/killed line after the\
 current line in the current proof."), NULL, CONF_OBJ_MENU,
   MENU_INSERT, conf_menu_value, GTK_STOCK_PASTE},

  {N_("Evaluate Line"),
   N_("Evaluate the current line in the current proof."), NULL,
   CONF_OBJ_MENU, MENU_EVAL_LINE, conf_menu_value, GTK_STOCK_EXECUTE},

  {N_("Evaluate Proof"), N_("Evaluate the current proof."), NULL,
   CONF_OBJ_MENU, MENU_EVAL_PROOF, conf_menu_value,
   GTK_STOCK_SELECT_ALL},

  {N_("Toggle Goals..."),
   N_("Check/Modify the current goal(s) for the current proof."),
   NULL, CONF_OBJ_MENU, MENU_GOAL, conf_menu_value, GTK_STOCK_INDEX},

  {N_("Toggle Boolean Mode"),
   N_("Toggle Boolean mode for the current proof."), NULL,
   CONF_OBJ_MENU, MENU_BOOLEAN, conf_menu_value, GTK_STOCK_CONVERT},

  {N_("Import Proof..."),
   N_("Import the premises and conclusions of a proof."), NULL,
   CONF_OBJ_MENU, MENU_IMPORT, conf_menu_value, GTK_STOCK_HARDDISK},

  {N_("Toggle Rules"), N_("Show/Hide the rules tablet."), NULL,
   CONF_OBJ_MENU, MENU_TOGGLE_RULES, conf_menu_value,
   GTK_STOCK_REFRESH},

  {N_("Small"), N_("Set the font size to small."), NULL,
   CONF_OBJ_MENU, MENU_SMALL, conf_menu_value, GTK_STOCK_ZOOM_OUT},

  {N_("Medium"), N_("Set the font size to medium."), NULL,
   CONF_OBJ_MENU, MENU_MEDIUM, conf_menu_value, GTK_STOCK_ZOOM_100},

  {N_("Large"), N_("Set the font size to large."), NULL,
   CONF_OBJ_MENU, MENU_LARGE, conf_menu_value, GTK_STOCK_ZOOM_IN},

  {N_("Custom..."), N_("Set the font size manually."), NULL,
   CONF_OBJ_MENU, MENU_CUSTOM, conf_menu_value, GTK_STOCK_ZOOM_FIT},

  {N_("About GNU Aris"), N_("Display information about GNU Aris."),
   NULL, CONF_OBJ_MENU, MENU_ABOUT, conf_menu_value, GTK_STOCK_ABOUT},

  {N_("Submit Proofs..."), N_("Submit all open proofs for grading."),
   NULL, CONF_OBJ_MENU, MENU_SUBMIT, conf_menu_value,
   GTK_STOCK_NETWORK},

  {N_("Customize"), N_("Customize GNU Aris."), NULL, CONF_OBJ_MENU,
   MENU_CUSTOMIZE, conf_menu_value, GTK_STOCK_SELECT_COLOR},

  {N_("Contents"), N_("Display help for GNU Aris."), NULL,
   CONF_OBJ_MENU, MENU_CONTENTS, conf_menu_value, GTK_STOCK_HELP},
};

/* The goal menu configuration objects */

static conf_obj goal_menu_conf[5] = {
  {"Add Goal", "Add a new goal for this proof.", NULL, CONF_OBJ_MENU,
   MENU_ADD_PREM, conf_menu_value, GTK_STOCK_ADD},

  {"Remove Goal", "Remove the current goal for this proof.", NULL,
   CONF_OBJ_MENU, MENU_KILL, conf_menu_value, GTK_STOCK_REMOVE},

  {"Check Line", "Check if the current goal has been met.", NULL,
   CONF_OBJ_MENU, MENU_EVAL_LINE, conf_menu_value, GTK_STOCK_INDEX},

  {"Check All", "Check if all goals have been met.", NULL,
   CONF_OBJ_MENU, MENU_EVAL_PROOF, conf_menu_value,
   GTK_STOCK_SELECT_ALL},

  {"Hide Goals", "Hide the goals window for this proof.", NULL,
   CONF_OBJ_MENU, MENU_GOAL, conf_menu_value, GTK_STOCK_REFRESH}
};

/* The internal configuration objects */

static conf_obj grade_conf[2] = {
  {N_("Grade IP"),
   N_("The IP Address of the Grading Server to Submit proofs to."),
   NULL, CONF_OBJ_GRADE, 0, conf_grade_value, NULL},
  {N_("Grade Password"),
   N_("The Password of the Grading Server."),
   NULL, CONF_OBJ_GRADE, 1, conf_grade_value, NULL}
};

/* The display configuration objects */

static conf_obj display_conf[10] = {
  {N_("Font Small Preset"),
   N_("The preset font size of the small option."), NULL,
   CONF_OBJ_FONT, FONT_TYPE_SMALL, conf_font_value, NULL},
  {N_("Font Medium Preset"),
   N_("The preset font size of the medium option."), NULL,
   CONF_OBJ_FONT, FONT_TYPE_MEDIUM, conf_font_value, NULL},
  {N_("Font Large Preset"),
   N_("The preset font size of the large option."), NULL,
   CONF_OBJ_FONT, FONT_TYPE_LARGE, conf_font_value, NULL},
  {N_("Default Font Size"),
   N_("The default font size to initialize GNU Aris with."),
   NULL, CONF_OBJ_FONT, FONT_TYPE_CUSTOM, conf_font_value, NULL},
  {N_("Default Color"), N_("The background color of normal lines."),
   NULL, CONF_OBJ_COLOR, BG_COLOR_DEFAULT, conf_color_value, NULL},
  {N_("Conclusion Color"), N_("The background color in which to\
 hilight the selected conclusion."), NULL, CONF_OBJ_COLOR,
   BG_COLOR_CONC, conf_color_value, NULL},
  {N_("Reference Color"), N_("The background color in which to\
 hilight the selected references."), NULL, CONF_OBJ_COLOR,
   BG_COLOR_REF, conf_color_value, NULL},
  {N_("Bad Color"), N_("The background color in which to hilight\
 mismatched parentheses and invalid goal lines."), NULL,
   CONF_OBJ_COLOR, BG_COLOR_BAD, conf_color_value, NULL},
  {N_("Good Color"), N_("The background color in which to hilight\
 matched parentheses and valid goal lines."), NULL, CONF_OBJ_COLOR,
   BG_COLOR_GOOD, conf_color_value, NULL},
  {N_("Selection Color"), N_("The background color in which to\
 hilight selected sentences."), NULL, CONF_OBJ_COLOR, BG_COLOR_SEL,\
   conf_color_value, NULL}
};

static const conf_obj menu_separator = {NULL, NULL, NULL,
					-1, -1, NULL, NULL};

/* Menu stuff. */

enum MENU_ORDER {
  FILE_MENU = 0,
  EDIT_MENU,
  PROOF_MENU,
  RULES_MENU,
  FONT_MENU,
  HELP_MENU,
  NUM_MENUS
};

enum MENU_SIZE {
  FILE_MENU_SIZE = 8,
  EDIT_MENU_SIZE = 8,
  PROOF_MENU_SIZE = 6,
  RULES_MENU_SIZE = 2,
  FONT_MENU_SIZE = 4,
  HELP_MENU_SIZE = 2
};

static conf_obj main_file_menu[FILE_MENU_SIZE];
static conf_obj main_edit_menu[EDIT_MENU_SIZE];
static conf_obj main_proof_menu[PROOF_MENU_SIZE];
static conf_obj main_rules_menu[RULES_MENU_SIZE];
static conf_obj main_font_menu[FONT_MENU_SIZE];
static conf_obj main_help_menu[HELP_MENU_SIZE];

static conf_obj * main_menus_menu[NUM_MENUS] = {
  main_file_menu,
  main_edit_menu,
  main_proof_menu,
  main_rules_menu,
  main_font_menu,
  main_help_menu
};

static conf_obj rule_file_menu[6];
static conf_obj rule_font_menu[4];
static conf_obj rule_help_menu[3];
static conf_obj * rule_menus_menu[3] = {
  rule_file_menu,
  rule_font_menu,
  rule_help_menu
};

static conf_obj * goal_menus;

/* The default configuration */

static const char * config_default ="\
(key-cmd \'New\' \'c+n\')\n\
(key-cmd \'Open\' \'c+o\')\n\
(key-cmd \'Save\' \'c+s\')\n\
(key-cmd \'Save As\' \'c+s+s\')\n\
(key-cmd \'Close\' \'c+w\')\n\
(key-cmd \'Quit\' \'c+q\')\n\
(key-cmd \'Add Premise\' \'c+p\')\n\
(key-cmd \'Add Conclusion\' \'c+j\')\n\
(key-cmd \'Add Subproof\' \'c+b\')\n\
(key-cmd \'End Subproof\' \'c+d\')\n\
(key-cmd \'Copy Line\' \'c+g\')\n\
(key-cmd \'Kill Line\' \'c+k\')\n\
(key-cmd \'Insert Line\' \'c+i\')\n\
(key-cmd \'Evaluate Line\' \'c+e\')\n\
(key-cmd \'Evaluate Proof\' \'c+f\')\n\
(key-cmd \'Toggle Goals...\' \'c+l\')\n\
(key-cmd \'Toggle Boolean Mode\' \'c+m\')\n\
(key-cmd \'Toggle Rules\' \'c+r\')\n\
(key-cmd \'Small\' \'c+-\')\n\
(key-cmd \'Medium\' \'c+0\')\n\
(key-cmd \'Large\' \'c+=\')\n\
(key-cmd \'Add Goal\' \'c+j\')\n\
(key-cmd \'Remove Goal\' \'c+k\')\n\
(key-cmd \'Check Line\' \'c+e\')\n\
(key-cmd \'Check All\' \'c+f\')\n\
(key-cmd \'Hide Goals\' \'c+l\')\n\
(font-size \'Small\' 8)\n\
(font-size \'Medium\' 16)\n\
(font-size \'Large\' 24)\n\
(font-size \'Default\' 8)\n\
(color-pref \'Default\' ffffff)\n\
(color-pref \'Conclusion\' 7fffff)\n\
(color-pref \'Reference\' ff7fff)\n\
(color-pref \'Bad\' 7f0000)\n\
(color-pref \'Good\' 007f00)\n\
(color-pref \'Selection\' ff0d00)\n\
(grade \'ip\' \'127.0.0.1\')\n\
(grade \'pass\' \'islegion\')\n";

#endif /*  ARIS_CONF_FILE_H  */

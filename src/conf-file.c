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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <math.h>

#include "vec.h"
#include "conf-file.h"
#include "process.h"
#include "menu.h"
#include "app.h"


#define READ_GRADE_ENT(c,s,k,a) if (!strcmp (c,s)) { if (a) { free (a); a = NULL; } a = strdup (k); }

/* Reads from the configuration file.
 *  input:
 *   file - the configuration file to read from.
 *   app - the app.
 *  output:
 *   0 on success, -1 on memory error, -2 on malformed config file.
 */
int
conf_file_read (const unsigned char * buffer, aris_app * app)
{
  size_t size = strlen (buffer);

  int ret_chk, pos;

  ret_chk = check_parens (buffer);
  if (ret_chk == 0)
    return -2;

  pos = 0;

  while (pos < size)
    {
      unsigned char * cur_conf;
      int tmp_pos, conf_len;

      while (pos < size && buffer[pos] != '(')
	pos++;

      if (pos == size)
	break;

      tmp_pos = parse_parens (buffer, pos, &cur_conf);
      if (tmp_pos == -1)
	return -1;

      if (tmp_pos < 0)
	return -2;

      conf_len = strlen (cur_conf);

      int cur_pos = 1;

      while (!isspace (cur_conf[cur_pos]))
	cur_pos++;

      unsigned char * cur_conf_key;

      cur_conf_key = (unsigned char *) calloc (cur_pos, sizeof (char));
      CHECK_ALLOC (cur_conf_key, -1);

      strncpy (cur_conf_key, cur_conf + 1, cur_pos - 1);
      cur_conf_key[cur_pos - 1] = '\0';

      if (!strcmp (cur_conf_key, "key-cmd"))
	{
	  free (cur_conf_key);

	  char * cmd, * key, tmp, * path;
	  int mod;

	  cmd = (char *) calloc (conf_len, sizeof (char));
	  CHECK_ALLOC (cmd, -1);

	  key = (char *) calloc (conf_len, sizeof (char));
	  CHECK_ALLOC (key, -1);

	  sscanf (cur_conf, "(key-cmd \'%[^\']\' \'%[^\']\')",
		  cmd, key);

	  // Parse key (c+s+s = Ctrl + Shift + s)
	  mod = 0;

	  ret_chk = 0;
	  char tmp_chk, * key_pos = key;

	  while (1)
	    {
	      ret_chk = sscanf (key, "%c%c", &tmp, &tmp_chk);

	      if (ret_chk != 2)
		break;

	      switch (tmp)
		{
		case 'c':
		  mod |= GDK_CONTROL_MASK;
		  break;
		case 's':
		  mod |= GDK_SHIFT_MASK;
		  break;
		}

	      key += 2;
	    }

	  path = (char *) calloc (strlen (cmd) + strlen (WIN_PATH) + 4,
				  sizeof (char));
	  CHECK_ALLOC (path, -1);

	  sprintf (path, "<%s>/%s", WIN_PATH, cmd);
	  free (cmd);

	  gtk_accel_map_change_entry (path, (int) (*key), mod, TRUE);
	  pos = tmp_pos + 2;

	  free (key_pos);
	  free (cur_conf);
	  continue;
	}

      if (!strcmp (cur_conf_key, "grade"))
	{
	  free (cur_conf_key);
	  char * cmd, * key;

	  cmd = (char *) calloc (conf_len, sizeof (char));
	  CHECK_ALLOC (cmd, -1);

	  key = (char *) calloc (conf_len, sizeof (char));
	  CHECK_ALLOC (key, -1);

	  sscanf (cur_conf, "(grade \'%[^\']\' \'%[^\']\')",
		  cmd, key);

	  READ_GRADE_ENT (cmd, "ip", key, app->ip_addr);
	  READ_GRADE_ENT (cmd, "pass", key, app->grade_pass);
	  READ_GRADE_ENT (cmd, "dir", key, app->grade_dir);

	  free (cmd);
	  free (key);

	  pos = tmp_pos + 2;
	  free (cur_conf);
	  continue;
	}

      if (!strcmp (cur_conf_key, "font-size"))
	{
	  free (cur_conf_key);

	  // Should be in the form (font-size type size)

	  int def_font;
	  char * type;

	  type = (char *) calloc (strlen (cur_conf), sizeof (char));
	  CHECK_ALLOC (type, -1);

	  ret_chk = sscanf (cur_conf, "(font-size \'%[^\']\' %i)",
			    type, &def_font);

	  if (ret_chk != 2)
	    {
	      free (cur_conf);
	      return -2;
	    }

	  int font_type;

	  font_type = the_app_get_font_by_name (app, type);

	  if (app->fonts[font_type])
	    pango_font_description_free (app->fonts[font_type]);

	  INIT_FONT (app->fonts[font_type], def_font);

	  pos = tmp_pos + 2;
	  free (cur_conf);
	  continue;
	}

      if (!strcmp (cur_conf_key, "color-pref"))
	{
	  free (cur_conf_key);

	  char * cur_color;
	  int r,g,b, c_hex;

	  cur_color = (char *) calloc (conf_len, sizeof (char));
	  CHECK_ALLOC (cur_color, -1);

	  ret_chk = sscanf (cur_conf, "(color-pref \'%[^\']\' %x)",
			    cur_color, &c_hex);
	  free (cur_conf);

	  if (ret_chk != 2)
	    {
	      free (cur_color);
	      return -2;
	    }

	  int cur;
	  cur = the_app_get_color_by_type (app, cur_color);
	  free (cur_color);

	  b = c_hex & 0xff;
	  g = (c_hex >> 8) & 0xff;
	  r = (c_hex >> 16) & 0xff;

	  if (app->bg_colors[cur])
	    free (app->bg_colors[cur]);

	  app_set_color (app, cur, r, g, b);
	  // Also need to update the colors of each sentence in each proof.

	  pos = tmp_pos + 2;
	  continue;
	}

      return -2;
    }

  return 0;
}

/* Gets or sets a value for a menu configuration object.
 *  input:
 *    obj - the configuration object to get or set.
 *    get - 1 if getting, 0 otherwise.
 *  output:
 *    The GtkWidget that is associated with this object if get = 1,
 *    or the string to print to the configuration file otherwise.
 */
void *
conf_menu_value (conf_obj * obj, int get)
{

  if (get)
    {
      GtkEntryBuffer * buffer;
      GtkAccelKey accel_key;
      char * accel_path;

      accel_path = (char *) calloc (strlen (obj->label)
				    + 16, sizeof (char));
      CHECK_ALLOC (accel_path, NULL);

      sprintf (accel_path, "<ARIS-WINDOW>/%s", obj->label);

      gtk_accel_map_lookup_entry (accel_path, &accel_key);
      free (accel_path);

      int key_pos = 0;
      char * key;

      key = (char *) calloc (8, sizeof (char));
      CHECK_ALLOC (key, NULL);

      if (accel_key.accel_mods & GDK_CONTROL_MASK)
	key_pos += sprintf (key + key_pos, "c+");
      if (accel_key.accel_mods & GDK_SHIFT_MASK)
	key_pos += sprintf (key + key_pos, "s+");
      key_pos += sprintf (key + key_pos, "%c", (char) accel_key.accel_key);

      buffer = gtk_entry_buffer_new (key, key_pos);
      obj->widget = gtk_entry_new_with_buffer (buffer);
      return obj->widget;
    }
  else
    {
      char * ret;
      const char * key;

      key = gtk_entry_get_text (GTK_ENTRY (obj->widget));

      int alloc_size;
      alloc_size = strlen (key) + strlen (obj->label) + 17;
      ret = (char *) calloc (alloc_size + 1, sizeof (char));
      CHECK_ALLOC (ret, NULL);

      if (key[0] != '\0')
	{
	  sprintf (ret, "(key-cmd \'%s\' \'%s\')\n",
		   obj->label, key);
	}
      else
	{
	  ret = NULL;
	}

      return ret;
    }

  return NULL;
}

/* Gets or sets a value for an ip configuration object.
 *  input:
 *    obj - the configuration object to get or set.
 *    get - 1 if getting, 0 otherwise.
 *  output:
 *    The GtkWidget that is associated with this object if get = 1,
 *    or the string to print to the configuration file otherwise.
 */
void *
conf_grade_value (conf_obj * obj, int get)
{
  if (get)
    {
      GtkEntryBuffer * buffer;
      int len = 0;

      switch (obj->id)
	{
	case CONF_GRADE_IP:
          len = the_app->ip_addr ? strlen (the_app->ip_addr) : -1;
	  buffer = gtk_entry_buffer_new (the_app->ip_addr, len);
	  break;

	case CONF_GRADE_PASS:
          len = the_app->grade_pass ? strlen (the_app->grade_pass) : -1;
	  buffer = gtk_entry_buffer_new (the_app->grade_pass, len);
	  break;

	case CONF_GRADE_DIR:
          len = the_app->grade_dir ? strlen (the_app->grade_dir) : -1;
	  buffer = gtk_entry_buffer_new (the_app->grade_dir, len);
	  break;
	}

      obj->widget = gtk_entry_new_with_buffer (buffer);
      return obj->widget;
    }
  else
    {
      char * ret;
      const char * text;

      const char * key;

      switch (obj->id)
	{
	case 0:
	  key = "ip";
	  break;
	case 1:
	  key = "pass";
	  break;
	case 2:
	  key = "dir";
	  break;
	default:
	  return NULL;
	}

      text = gtk_entry_get_text (GTK_ENTRY (obj->widget));

      int alloc_size;
      alloc_size = strlen (key) + strlen (text) + 14;

      ret = (char *) calloc (alloc_size + 1, sizeof (char));
      CHECK_ALLOC (ret, NULL);

      sprintf (ret, "(grade \'%s\' \'%s\')\n", key, text);
      return ret;
    }
}

/* Gets or sets a value for a font configuration object.
 *  input:
 *    obj - the configuration object to get or set.
 *    get - 1 if getting, 0 otherwise.
 *  output:
 *    The GtkWidget that is associated with this object if get = 1,
 *    or the string to print to the configuration file otherwise.
 */
void *
conf_font_value (conf_obj * obj, int get)
{
  if (get)
    {
      FONT_TYPE cur_val;
      int val;

      cur_val = the_app->fonts[obj->id];
      obj->widget = gtk_spin_button_new_with_range (8.0, 32.0, 1.0);

      FONT_GET_SIZE (cur_val, val);

      gtk_spin_button_set_value (GTK_SPIN_BUTTON (obj->widget), val);

      return obj->widget;
    }
  else
    {
      char * ret, * size;
      int val, ret_chk, lbl_len, alloc_size;

      lbl_len = strlen (obj->label);

      val = (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (obj->widget));

      size = (char *) calloc (lbl_len, sizeof (char));
      CHECK_ALLOC (size, NULL);

      ret_chk = sscanf (obj->label, "Font %s Preset", size);
      if (ret_chk != 1)
	strcpy (size, _("Default"));

      alloc_size = strlen (size) + 16 + (int) log10 ((double) val) + 1;
      ret = (char *) calloc (alloc_size + 1 , sizeof (char));
      CHECK_ALLOC (ret, NULL);

      sprintf (ret, "(font-size \'%s\' %i)\n", size, val);
      free (size);
      return ret;
    }
}

/* Gets or sets a value for a color configuration object.
 *  input:
 *    obj - the configuration object to get or set.
 *    get - 1 if getting, 0 otherwise.
 *  output:
 *    The GtkWidget that is associated with this object if get = 1,
 *    or the string to print to the configuration file otherwise.
 */
void *
conf_color_value (conf_obj * obj, int get)
{
  if (get)
    {
      obj->widget =
	gtk_color_button_new_with_rgba (the_app->bg_colors[obj->id]);

      return obj->widget;
    }
  else
    {
      GdkRGBA color;

      gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (obj->widget),
				  &color);

      char * cur_type;
      int r, g, b;

      r = color.red * 255;
      g = color.green * 255;
      b = color.blue * 255;

      cur_type = (char *) calloc (strlen (obj->label), sizeof (char));
      CHECK_ALLOC (cur_type, NULL);

      sscanf (obj->label, "%s", cur_type);

      char * ret;
      int alloc_size;

      alloc_size = strlen (cur_type) + 24;

      ret = (char *) calloc (alloc_size + 1, sizeof (char));
      CHECK_ALLOC (ret, NULL);

      sprintf (ret, "(color-pref \'%s\' %02x%02x%02x)\n",
	       cur_type, r, g, b);
      free (cur_type);
      return ret;
    }
}

unsigned char *
config_default ()
{
  unsigned char * ret;
  int i, j, pos = 0, new_size;
  ret = (unsigned char *) calloc (1, sizeof (char));
  CHECK_ALLOC (ret, NULL);

  for (j = 0; j < 4; j++)
    {
      for (i = 0; i < conf_sizes[j]; i++)
	{
	  conf_obj cur = conf_arrays[j][i];
	  if (!cur.default_value)
	    continue;
	  const char * cmd = conf_cmds[cur.type];
	  new_size = 9 + strlen (cmd) + strlen (cur.label)
	    + strlen (cur.default_value) + pos;
	  ret = (unsigned char *) realloc (ret, (new_size + 1)
					   * sizeof (char));
	  CHECK_ALLOC (ret, NULL);

	  pos += sprintf (ret + pos, "(%s \'%s\' \'%s\')\n",
			  cmd, cur.label, cur.default_value);
	}
    }

  return ret;
}

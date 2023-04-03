/* Functions for handling the application structure.

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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "config.h"

#include "process.h"
#include "app.h"
#include "rules-table.h"
#include "list.h"
#include "aris-proof.h"
#include "callbacks.h"
#include "icon.xpm"
#include "goal.h"
#include "sen-parent.h"
#include "menu.h"
#include "conf-file.h"

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

aris_app * the_app = 0;

// Macro to get the length of a file.

#ifndef WIN32
#define GET_LEN(f,l){				\
  fseeko (f, 0, SEEK_END);			\
  l = ftello (f);				\
  fseeko (f, 0, SEEK_SET);			\
  }
#else
#define GET_LEN(f,l){				\
  fseek (f, 0, SEEK_END);			\
  l = ftell (f);				\
  fseek (f, 0, SEEK_SET);			\
  }
#endif

#define FTP_PORT 21
#define FTP_QUIT(s) { ftp_send_cmd (s, "QUIT"); g_socket_close (s,NULL); }

/* Initializes an app structure.
 *  input:
 *    boolean - the boolean flag from the cmd line args.
 *  output:
 *    the new app structure.
 */
aris_app *
init_app (int boolean, int verbose)
{
  aris_app * app = (aris_app *) calloc (1, sizeof (aris_app));
  if (!app)
    {
      PERROR (NULL);
      return NULL;
    }

  int ret_chk;

  // Initialize the fonts and background colors.

  int i;
  for (i = 0; i < NUM_FONT_TYPES; i++)
    app->fonts[i] = NULL;
  for (i = 0; i < NUM_BG_COLORS; i++)
    app->bg_colors[i] = NULL;

  /* First, read the default configuration.
   * This way, if there are any problems reading the user's
   *   configuration, then there will already be one in place.
   */

  ret_chk = the_app_read_default_config (app);
  if (ret_chk == -1)
    return NULL;

  ret_chk = the_app_read_config_file (app);
  if (ret_chk == -1)
    return NULL;

  app->icon = gdk_pixbuf_new_from_xpm_data (aris_icon_xpm);

  the_app_init_conn_pixbufs (app);

  app->rt = rules_table_init (boolean);
  gtk_window_set_icon (GTK_WINDOW (app->rt->window), app->icon);

  app->rt->boolean = boolean;
  app->boolean = boolean;
  app->verbose = verbose;

  app->working_dir = (char *) calloc (1024, sizeof (char));
  app->help_file = (char *) calloc (1024, sizeof (char));

  app->guis = init_list ();

  return app;
}

// Initializes a connective pixbuf.

#define INIT_CONN_PIXBUF(i,d,c) {		\
    app->conn_pixbufs[i] = gdk_pixbuf_new_from_xpm_data ((const char **) d); \
    g_object_set_data (G_OBJECT (app->conn_pixbufs[i]), _("conn"), c);	\
  }

/* Initializes the connective pixbufs for the app.
 *  input:
 *    app - the main application structure.
 *  output:
 *    0 on success.
 */
int
the_app_init_conn_pixbufs (aris_app * app)
{
  INIT_CONN_PIXBUF (0, and_conn_xpm, AND);
  INIT_CONN_PIXBUF (1, or_conn_xpm, OR);
  INIT_CONN_PIXBUF (2, not_conn_xpm, NOT);
  INIT_CONN_PIXBUF (3, con_conn_xpm, CON);
  INIT_CONN_PIXBUF (4, bic_conn_xpm, BIC);
  INIT_CONN_PIXBUF (5, unv_conn_xpm, UNV);
  INIT_CONN_PIXBUF (6, exl_conn_xpm, EXL);
  INIT_CONN_PIXBUF (7, tau_conn_xpm, TAU);
  INIT_CONN_PIXBUF (8, ctr_conn_xpm, CTR);
  INIT_CONN_PIXBUF (9, elm_conn_xpm, ELM);
  INIT_CONN_PIXBUF (10, nil_conn_xpm, NIL);

  return 0;
}

/* Gets a connective from the application by the connective type.
 *  input:
 *    type - the type of the connective to get.
 *  output:
 *    The connective pixbuf.
 */
GdkPixbuf *
the_app_get_conn_by_type (char * type)
{
  GdkPixbuf * ret = NULL;

  if (!strcmp (type, AND))
    ret = the_app->conn_pixbufs[AND_CONN];

  if (!strcmp (type, OR))
    ret = the_app->conn_pixbufs[OR_CONN];

  if (!strcmp (type, NOT))
    ret = the_app->conn_pixbufs[NOT_CONN];

  if (!strcmp (type, CON))
    ret = the_app->conn_pixbufs[CON_CONN];

  if (!strcmp (type, BIC))
    ret = the_app->conn_pixbufs[BIC_CONN];

  if (!strcmp (type, UNV))
    ret = the_app->conn_pixbufs[UNV_CONN];

  if (!strcmp (type, EXL))
    ret = the_app->conn_pixbufs[EXL_CONN];

  if (!strcmp (type, TAU))
    ret = the_app->conn_pixbufs[TAU_CONN];

  if (!strcmp (type, CTR))
    ret = the_app->conn_pixbufs[CTR_CONN];

  if (!strcmp (type, ELM))
    ret = the_app->conn_pixbufs[ELM_CONN];

  if (!strcmp (type, NIL))
    ret = the_app->conn_pixbufs[NIL_CONN];

  return ret;
}

/* Gets an application color by its type.
 *  input:
 *    app - the main application structure.
 *    type - the type of the color to obtain.
 *  output:
 *    The offset within the application structures color list
 *     of the color specified by type.
 */
int
the_app_get_color_by_type (aris_app * app, char * type)
{
  int i;
  int ret = 0;
  for (i = 4; i < NUM_DISPLAY_CONFS; i++)
    {
      if (!strcmp (type, display_conf[i].label))
        {
          ret = display_conf[i].id;
          break;
        }
    }

  return ret;
}

/* Gets a color designation by its index.
 *  input:
 *    app - the main application structure.
 *    index - the offset within the application structure's list
 *              of colors.
 *  output:
 *    The name of the color type.
 */
char *
the_app_get_color_by_index (aris_app * app, int index)
{
  char * ret = NULL;
  int i;

  for (i = 4; i < NUM_DISPLAY_CONFS; i++)
    {
      if (display_conf[i].id == index)
        {
          ret = display_conf[i].label;
          break;
        }
    }

  return ret;
}

/* Gets a font type by its name.
 *  input:
 *    app - the main application structure.
 *    name - the name of the font type.
 *  output:
 *    The offset with app->fonts of the font.
 */
int
the_app_get_font_by_name (aris_app * app, char * name)
{
  int ret = -1;
  int i;

  for (i = 0; i < 4; i++)
    {
      if (!strcmp (name, display_conf[i].label))
        {
          ret = display_conf[i].id;
          break;
        }
    }

  return ret;
}

/* Read the default configuration.
 *  input:
 *    app - the main application structure.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
the_app_read_default_config (aris_app * app)
{
  int ret_chk;

  unsigned char * conf_def = config_default ();

  ret_chk = conf_file_read (conf_def, app);
  if (ret_chk == AEC_MEM)
    return AEC_MEM;

  return 0;
}

/* Constructs the config file path, then reads from it.
 *  input:
 *   none.
 *  output:
 *   0 on success, -1 on memory error, -2 on read error.
 */
int
the_app_read_config_file (aris_app * app)
{
  FILE * conf_file;
  char * path, * home_dir;
  int alloc_size, exists, ret_chk, path_pos = 0;

  home_dir = getenv ("HOME");
  if (!home_dir)
    home_dir = getenv ("HOMEPATH");

  alloc_size = strlen (home_dir) + strlen (CONF_FILE) + 1;
#ifdef WIN32
  alloc_size += strlen (getenv ("HOMEDRIVE")) + 1;
#endif
  path = (char *) calloc (alloc_size + 1, sizeof (char));
  CHECK_ALLOC (path, AEC_MEM);

#ifdef WIN32
  path_pos += sprintf (path, "%s\\", getenv ("HOMEDRIVE"));
#endif
 
  sprintf (path + path_pos, "%s/%s", home_dir, CONF_FILE);
  exists = access (path, F_OK);
  if (exists == -1)
    {
      ret_chk = the_app_make_default_config_file (path);
      if (ret_chk == -2)
	return -2;
    }

  conf_file = fopen (path, "r+");
  if (!conf_file)
    {
      PERROR ("load_config_file");
      return -2;
    }
  free (path);

  unsigned char * buffer;
  size_t size;

  GET_LEN (conf_file, size);
  buffer = (unsigned char *) calloc (size + 1, sizeof (char));
  CHECK_ALLOC (buffer, -1);

  // Because Losedows hates it when we try to read the entire file at once.
  int f_pos;
  for (f_pos = 0; f_pos < size; f_pos+=512)
    fread (buffer + f_pos, 1, 512, conf_file);
  buffer[size] = '\0';

  fclose (conf_file);

  ret_chk = conf_file_read (buffer, app);
  if (ret_chk == -1)
    return -1;

  free (buffer);

  if (ret_chk == -2)
    {
      // Maybe notify the user that there is a problem.
      // Otherwise, it shouldn't matter.
    }

  return 0;
}

/* Creates the default configuration file.
 *  input:
 *   path - the path to the configuration file.
 *  output:
 *   0 on success, -2 on read error, -1 on memory error.
 */
int
the_app_make_default_config_file (char * path)
{
  FILE * config_file;

  config_file = fopen (path, "w");
  if (!config_file)
    {
      PERROR (NULL);
      return -2;
    }

  unsigned char * conf_def;
  conf_def = config_default ();
  if (!conf_def)
    {
      fclose (config_file);
      return AEC_MEM;
    }

  int len, rc;
  len = strlen (conf_def);

  rc = fwrite (conf_def, 1, len, config_file);
  if (rc != len)
    {
      PERROR ("default_config_file_fwrite");
      fclose (config_file);
      return -2;
    }

  fclose (config_file);

  return 0;
}

/* Sets focus to a gui.
 *  input:
 *    ap - the proof of the gui to set focus to.
 *  output:
 *    0 on success, -1 on error.
 */
int
the_app_set_focus (aris_proof * ap)
{
  if (!ap)
    return AEC_MEM;

  the_app->focused = ap;

  item_t * itm;
  itm = ls_find (the_app->guis, ap);
  if (!itm)
    return AEC_MEM;

  // This is already the first element,
  // so no need to re-order the elements.
  if (!itm->prev)
    return 0;

  itm->prev->next = itm->next;
  if (itm->next)
    itm->next->prev = itm->prev;
  else
    the_app->guis->tail = itm->prev;

  itm->next = the_app->guis->head;
  itm->prev = NULL;
  the_app->guis->head->prev = itm;
  the_app->guis->head = itm;

  if (the_app->rt)
    {
      rules_table_align (the_app->rt, ap);
      rules_table_set_boolean_mode (the_app->rt, ap->boolean);
    }

  return 0;
}

/* Adds a gui to the app's list.
 *  input:
 *    ap - the proof of the gui to add.
 *  output:
 *    0 on success, -1 on error.
 */
int
the_app_add_gui (aris_proof * ap)
{
  if (!ap)
    return -1;

  item_t * itm;
  itm = ls_ins_obj (the_app->guis, ap, the_app->guis->tail);
  if (!itm)
    return AEC_MEM;

  gtk_window_set_icon (GTK_WINDOW (SEN_PARENT (ap)->window), the_app->icon);
  gtk_window_set_icon (GTK_WINDOW (SEN_PARENT (ap->goal)->window), the_app->icon);

  int ret;
  ret = the_app_set_focus (ap);
  if (ret < 0)
    return AEC_MEM;

  return 0;
}

/* Removes a gui from the app's list.
 *  input:
 *    ap - the proof of the gui to remove.
 *  output:
 *    none.
 */
void
the_app_rem_gui (aris_proof * ap)
{
  item_t * itm, * n_itm;
  itm = ls_find (the_app->guis, ap);

  if (the_app->guis->num_stuff == 1)
    n_itm = NULL;
  else if (itm == the_app->guis->head)
    n_itm = the_app->guis->head->next;
  else if (itm == the_app->guis->tail)
    n_itm = the_app->guis->tail->prev;
  else
    n_itm = itm->prev;

  ls_rem_obj (the_app->guis, itm);
  if (n_itm)
    the_app_set_focus ((aris_proof *) n_itm->value);
  else
    {
      gtk_widget_show_all (the_app->rt->window);
      the_app->focused = NULL;
      if (the_app->rt->toggled != -1)
	{
	  TOGGLE_BUTTON (the_app->rt->rules [the_app->rt->toggled]);
	}
    }
}

/* Quits out of the application.
 *   input:
 *     None.
 *   output:
 *     0 on success, -1 if canceled.
 */
int
app_quit ()
{
  item_t * g_itr, * ng_itr;
  int rc;

  for (g_itr = the_app->guis->head; g_itr; g_itr = ng_itr)
    {
      ng_itr = g_itr->next;

      rc = gui_destroy ((aris_proof *) g_itr->value);
      if (rc < 0)
        return -1;

      g_itr->next = g_itr->prev = g_itr->value = NULL;
      free (g_itr);
    }

  return 0;
}

/* Sets the color with index INDEX to the specified RGB values.
 *  input:
 *    app - the main application object.
 *    index - the index of the color to setup.
 *    red, green, blue - the RGB values of the new color ([0,255]).
 *  output:
 *    0 on success.
 */
int
app_set_color (aris_app * app, int index, int red, int green, int blue)
{
  INIT_COLOR (app->bg_colors[index], red, green, blue);

  if (app->guis)
    {
      item_t * gui_iter;
      for (gui_iter = app->guis->head; gui_iter;
	   gui_iter = gui_iter->next)
	{
	  aris_proof * ap = gui_iter->value;
	  sen_parent_children_set_bg_color (SEN_PARENT (ap));
	}
    }

  return 0;
}

/* Thanks to the developers of the inetutils package for this next part.
 * The ftp utility's source code was very helpful with this.
 */

/* Send a command to an ftp server.
 *  input:
 *    ftp_socket - the socket that connects to the ftp server.
 *    cmd - the command to send to the ftp server.
 *  output:
      none
 */
void
ftp_send_cmd (GSocket * ftp_sock, const char * cmd)
{
  char * buf;
  int len;

  buf = (char *) calloc (strlen (cmd) + 3, sizeof (char));
  len = sprintf (buf, "%s\r\n", cmd);
  g_socket_send (ftp_sock, buf, len, NULL, NULL);
  free (buf);
}

/* Get a response from the ftp server.
 *  input:
 *    ftp_sock - the file handle of the server.
 *    port - a pointer to an integer if the port needs to be determined.
 *  output:
 *    the response code.
 */
int
ftp_get_response (GSocket * ftp_sock, int * port)
{
  char response_buffer[2048], * buf_str;
  int ret_code, rc;

  buf_str = response_buffer;

  while (1)
    {
      g_socket_receive (ftp_sock, buf_str, 1, NULL, NULL);
      if (*buf_str == EOF || *buf_str == '\n')
	break;
      buf_str++;
    }

  *buf_str = '\0';
  rc = sscanf (response_buffer, "%i", &ret_code);
  if (rc != 1)
    return AEC_IO;

  if (port)
    {
      if (ret_code == 227)
	{
	  int a0,a1,a2,a3,p0,p1;
	  sscanf (response_buffer + 26, "(%i,%i,%i,%i,%i,%i)\n",
		  &a0,&a1,&a2,&a3,&p0,&p1);

	  *port = ((p0 & 0xff) << 8) | (p1 & 0xff);
	}
    }

  return ret_code;
}

/* Sends a file to the ftp server.
 *  input:
 *    ftp_sock - a handle to the ftp server.
 *    file_name - the name of the file to transfer.
 *    buffer - the contents of the file to transfer.
 *  output:
 *    0 on success, -1 on memory error, -2 on file error.
 */
int
ftp_send (GSocket * ftp_sock, char * file_name, char * buffer)
{
  int port;
  char * buf;
  int ret_chk;

  buf = buffer;

  GSocket * sock;
  sock = g_socket_new (G_SOCKET_FAMILY_IPV4,
		       G_SOCKET_TYPE_STREAM,
		       G_SOCKET_PROTOCOL_TCP,
		       NULL);
  if (!sock)
    {
      return AEC_IO;
    }

  ftp_send_cmd (ftp_sock, "PASV");
  ret_chk = ftp_get_response (ftp_sock, &port);
  if (ret_chk != 227)
    {
      FTP_QUIT (ftp_sock);
      return AEC_IO;
    }

  GSocketAddress * in_addr;
  GInetAddress * addr;
  addr = g_inet_address_new_from_string (the_app->ip_addr);
  in_addr = g_inet_socket_address_new (addr, port);

  gboolean ret_val;
  ret_val = g_socket_connect (sock, in_addr, NULL, NULL);
  if (!ret_val)
    {
      return AEC_IO;
    }

  char * abuf;
  size_t alen;

  alen = strlen (file_name) + 5;
  abuf = (char *) calloc (alen + 1, sizeof (char));
  CHECK_ALLOC (abuf, -1);

  sprintf (abuf, "STOR %s", file_name);
  ftp_send_cmd (ftp_sock, abuf);
  free (abuf);
  ret_chk = ftp_get_response (ftp_sock, NULL);

  if (ret_chk != 150)
    {
      FTP_QUIT (ftp_sock);
      return AEC_IO;
    }

  while (*buf)
    {
      if (*buf == '\n')
	g_socket_send (sock, "\r", 1, NULL, NULL);
      g_socket_send (sock, buf, 1, NULL, NULL);
      buf++;
    }

  g_socket_close (sock, NULL);

  ret_chk = ftp_get_response (ftp_sock, NULL);
  if (ret_chk != 226)
    {
      FTP_QUIT (ftp_sock);
      return AEC_IO;
    }

  return 0;
}

/* Initiates the ftp connection.
 *  input:
 *    ip_addr - the IP Address of the ftp server.
 *  output:
 *    A handle to the ftp server on success, NULL on error.
 */
GSocket *
ftp_connect (char * ip_addr)
{
  GSocket * ret;
  ret = g_socket_new (G_SOCKET_FAMILY_IPV4,
		      G_SOCKET_TYPE_STREAM,
		      G_SOCKET_PROTOCOL_TCP,
		      NULL);
  if (!ret)
    {
      return NULL;
    }

  GSocketAddress * in_addr;
  GInetAddress * addr;
  addr = g_inet_address_new_from_string (ip_addr);
  in_addr = g_inet_socket_address_new (addr, FTP_PORT);

  // Set a timeout to prevent the socket from taking too long.
  g_socket_set_timeout (ret, 5);
  
  gboolean ret_val;
  ret_val = g_socket_connect (ret, in_addr, NULL, NULL);

  if (!ret_val)
    return NULL;

  // I'm not sure what the response code is supposed to be here.
  // I *think* 200
  int rc, ret_chk;
  rc = ftp_get_response (ret, NULL);
  if (rc < 0)
    {
      FTP_QUIT (ret);
      return NULL;
    }

  ftp_send_cmd (ret, "USER anonymous");
  ret_chk = ftp_get_response (ret, NULL);

  if (ret_chk != 331)
    {
      FTP_QUIT (ret);
      return NULL;
    }

  char * pass_buf;

  pass_buf = (char *) calloc (strlen (the_app->grade_pass) + 6,
			      sizeof (char));
  CHECK_ALLOC (pass_buf, NULL);

  sprintf (pass_buf, "PASS %s", the_app->grade_pass);

  ftp_send_cmd (ret, pass_buf);
  free (pass_buf);
  ret_chk = ftp_get_response (ret, NULL);

  if (ret_chk != 230)
    {
      FTP_QUIT (ret);
      return NULL;
    }

  char * dir_buf;
  dir_buf = (char * ) calloc (strlen (the_app->grade_dir) + 6,
			      sizeof (char));
  CHECK_ALLOC (dir_buf, NULL);

  sprintf (dir_buf, "CWD %s", the_app->grade_dir);

  ftp_send_cmd (ret, dir_buf);
  free (dir_buf);
  ret_chk = ftp_get_response (ret, NULL);

  if (ret_chk != 250)
    {
      FTP_QUIT (ret);
      return NULL;
    }

  return ret;
}

/* Submits all open proofs for grading.
 *  input:
 *    user_email - the email address of the user.
 *    instr_email - the email address of the instructor.
 *    entries - the array of submission entries.
 *  output:
 *    0 on success, -1 on memory error, -2 on file error, -3 on other errors.
 */
int
the_app_submit (const char * user_email, const char * instr_email,
		struct submit_ent * entries)
{
  int ret_chk;
  GSocket * ftp_file;

  ftp_file = ftp_connect (the_app->ip_addr);
  if (!ftp_file)
    return -3;

  char * email_base;

  email_base = (char *) calloc (strlen (user_email), sizeof (char));
  CHECK_ALLOC (email_base, AEC_MEM);

  ret_chk = sscanf (user_email, "%[^@]@", email_base);
  if (ret_chk != 1)
    {
      printf ("Submission Error - please specify a valid email address.\n");
      free (email_base);
      return -3;
    }

  char * dir_file_name, * dir_buffer;

  dir_file_name = (char *) calloc (strlen (email_base) + 11, sizeof (char));
  CHECK_ALLOC (dir_file_name, AEC_MEM);
  sprintf (dir_file_name, "%s.directive", email_base);

  int dir_pos = 0;

  int alloc_size = strlen (user_email) + strlen (instr_email) + 16;

  dir_buffer = (char *) calloc (alloc_size + 3, sizeof (char));
  CHECK_ALLOC (dir_buffer, AEC_MEM);

  dir_pos += sprintf (dir_buffer, "user: %s\ninstr: %s\n",
		      user_email, instr_email);
  int i;

  for (i = 0; entries[i].hw; i++)
    {
      char * hw, * file_name;

      hw = entries[i].hw;
      file_name = entries[i].file_name;

      char * base, * base_name, * ap_file_name;

      GFile * file = g_file_new_for_path (file_name);
      base_name = g_file_get_basename (file);

      base = (char *) calloc (strlen (base_name), sizeof (char));
      CHECK_ALLOC (base, AEC_MEM);

      sscanf (base_name, "%[^.].tle", base);
      free (base_name);

      ap_file_name = (char *) calloc (strlen (email_base)
				      + strlen (base) + 6,
				      sizeof (char));
      CHECK_ALLOC (ap_file_name, AEC_MEM);
      sprintf (ap_file_name, "%s-%s.tle", base, email_base);

      free (base);

      FILE * ap_file;
      char * ap_buffer;
      off_t len;

      ap_file = fopen (file_name, "r");
      if (!ap_file)
	{
	  free (ap_file_name);
	  PERROR (NULL);
	  return AEC_IO;
	}

      GET_LEN (ap_file, len);

      ap_buffer = (char *) calloc (len + 1, sizeof (char));
      CHECK_ALLOC (ap_buffer, AEC_MEM);

      ret_chk = fread (ap_buffer, 1, len, ap_file);
      if (ret_chk != len)
        {
          free (ap_buffer);
          free (dir_buffer);
          free (email_base);
          free (dir_file_name);
          free (ap_file_name);
          fclose (ap_file);
          FTP_QUIT (ftp_file);
          return AEC_IO;
        }
      ap_buffer[len] = '\0';
      fclose (ap_file);

      ret_chk = ftp_send (ftp_file, ap_file_name, ap_buffer);
      if (ret_chk == AEC_MEM)
	return AEC_MEM;

      if (ret_chk == AEC_IO)
        {
          free (ap_buffer);
          free (dir_buffer);
          free (email_base);
          free (dir_file_name);
          free (ap_file_name);
          FTP_QUIT (ftp_file);
          return AEC_IO;
        }

      free (ap_buffer);

      int alloc_size = strlen (ap_file_name) + strlen (hw)
	+ 3 + dir_pos;

      dir_buffer = (char *) realloc (dir_buffer, alloc_size
				     * sizeof (char));
      CHECK_ALLOC (dir_buffer, -1);
      dir_pos += sprintf (dir_buffer + dir_pos, "%s|%s\n",
			  hw, ap_file_name);

      free (ap_file_name);
    }

  free (email_base);

  ret_chk = ftp_send (ftp_file, dir_file_name, dir_buffer);
  if (ret_chk == AEC_MEM)
    return AEC_MEM;
  if (ret_chk == AEC_IO)
    {
      free (dir_file_name);
      free (dir_buffer);
      FTP_QUIT (ftp_file);
      return AEC_IO;
    }

  free (dir_file_name);
  free (dir_buffer);

  FTP_QUIT (ftp_file);

  return 0;
}

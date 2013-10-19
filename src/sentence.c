/* Functions for handling the sentence structure.

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

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "sentence.h"
#include "app.h"
#include "sen-parent.h"
#include "aris-proof.h"
#include "list.h"
#include "sen-data.h"
#include "callbacks.h"
#include "rules-table.h"
#include "process.h"
#include "goal.h"
#include "vec.h"

//#define LETTERS

#ifdef LETTERS
static char * sen_values[6] = {" ", "T", "F", "*", "?", "#"};
#else
static char * sen_values[6] = {GTK_STOCK_MEDIA_STOP,
			       GTK_STOCK_APPLY,
			       GTK_STOCK_CANCEL,
			       GTK_STOCK_STOP,
			       GTK_STOCK_SPELL_CHECK,
			       GTK_STOCK_SELECT_COLOR};
#endif

// GTextCharPredicate for determining the location of the comment.
static gboolean
comment_predicate (gunichar ch, gpointer user_data)
{
  if (ch == SEN_COMMENT_CHAR)
    return TRUE;
  else
    return FALSE;
}


/* Initializes a sentence
 *  input:
 *    sd  - sentence data to initialize from.
 *    sp  - the new parent of this sentence.
 *    fcs - the item that this sentence will come after.
 *  output:
 *    The newly initialized sentence.
 */
sentence *
sentence_init (sen_data * sd, sen_parent * sp, item_t * fcs)
{
  // Initialize the new sentence.
  sentence * sen = NULL;
  sen = (sentence *) calloc (1, sizeof (sentence));
  CHECK_ALLOC (sen, NULL);

  int ln = 0, ret, depth;

  // Only run this part if this is a proof sentence.
  if (sp->type == SEN_PARENT_TYPE_PROOF)
    {
      // Is this the first sentence?
      if (!fcs)
	ln = 1;
      else
	ln = SENTENCE (fcs->value)->line_num + 1;
    }

  // Initialize the GUI components.
  sen->premise = sd->premise;
  sen->depth = sd->depth;
  sentence_gui_init (sen);
  sen->parent = sp;
  sen->value_type = VALUE_TYPE_BLANK;
  sen->selected = 0;
  sen->sexpr = NULL;
  sen->font_resizing = 0;

  sen->indices = (int *) calloc (sen->depth + 1, sizeof (int));
  CHECK_ALLOC (sen->indices, NULL);

  int i = 0;

  if (!sen->premise)
    {
      sentence * fcs_sen;

      int index_copy_end;

      fcs_sen = fcs->value;

      index_copy_end = (fcs_sen->depth < sen->depth)
	? fcs_sen->depth : sen->depth;

      for (i = 0; i < index_copy_end; i++)
	sen->indices[i] = fcs_sen->indices[i];

      if (sd->subproof)
	sen->indices[i++] = ln;
    }

  sen->indices[i] = -1;

  // Set the data components.
  ret = sentence_set_line_no (sen, ln);
  if (ret == -1)
    return NULL;

  sen->rule = sd->rule;

  if (sen->rule != -1)
    {
      gtk_label_set_text (GTK_LABEL (sen->rule_box),
			  rules_list[sen->rule]);
    }

  if (sd->text)
    {
      sen->text = (unsigned char *) calloc (strlen (sd->text) + 1,
					    sizeof (char));
      CHECK_ALLOC (sen->text, NULL);
      strcpy (sen->text, sd->text);
      ret = sentence_paste_text (sen);
      if (ret == -1)
	return NULL;

      if (ret == -2)
	return NULL;
    }
  else
    {
      sen->text = (unsigned char *) calloc (1, sizeof (char));
      CHECK_ALLOC (sen->text, NULL);
      sen->text[0] = '\0';
    }

  sen->refs = init_list ();
  if (!sen->refs)
    return NULL;

  if (sd->refs)
    {
      for (i = 0; sd->refs[i] != -1; i++)
	{
	  int cur_line;
	  item_t * ev_itr;

	  cur_line = sd->refs[i];
	  ev_itr = sen->parent->everything->head;

	  if (cur_line > ln)
	    continue;

	  for (; ev_itr != NULL; ev_itr = ev_itr->next)
	    {
	      sentence * ref_sen;
	      ref_sen = ev_itr->value;

	      if (ref_sen->line_num == cur_line)
		{
		  item_t * itm;
		  itm = ls_push_obj (sen->refs, ref_sen);
		  if (!itm)
		    return NULL;

		  break;
		}
	    }
	}
    }

  if (sd->file)
    {
      unsigned int file_len = strlen ((const char *) sd->file);
      sen->file = (unsigned char *) calloc (file_len + 1,
					    sizeof (char));
      CHECK_ALLOC (sen->file, NULL);
      strcpy (sen->file, sd->file);
    }
  else
    {
      sen->file = NULL;
    }

  sen->vars = NULL;
  sen->reference = 0;
  sen->subproof = sd->subproof;

  sentence_set_font (sen, sen->parent->font);

  if (!sd->premise)
    sentence_set_bg (sen, BG_COLOR_CONC);
  else
    sentence_set_bg (sen, BG_COLOR_DEFAULT);

  sentence_connect_signals (sen);

  return sen;
}

/* Initializes the gui elements of a sentence.
 * input:
 *   sen - the sentence to initialize the gui elements of.
 * output:
 *   none.
 */
void
sentence_gui_init (sentence * sen)
{
  // Initialize the GUI components.
  sen->panel = gtk_hbox_new (FALSE, 0);

  sen->line_no = gtk_label_new (NULL);
  gtk_label_set_justify (GTK_LABEL (sen->line_no), GTK_JUSTIFY_FILL);
  gtk_label_set_width_chars (GTK_LABEL (sen->line_no), 3);

  sen->eventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (sen->eventbox), sen->line_no);
  gtk_event_box_set_above_child (GTK_EVENT_BOX (sen->eventbox), TRUE);

  GtkWidget * widget;
  if (sen->depth > 0)
    {
      widget = gtk_label_new (NULL);
      g_object_set (G_OBJECT (widget), "width-chars", 4 * sen->depth,
		    NULL);
    }

  sen->entry = gtk_text_view_new ();


#ifdef LETTERS
  sen->value = gtk_label_new (NULL);
  gtk_label_set_justify (GTK_LABEL (sen->value), GTK_JUSTIFY_FILL);
  gtk_label_set_width_chars (GTK_LABEL (sen->value), 2);
#else
  sen->value = gtk_image_new_from_stock (sen_values[0], GTK_ICON_SIZE_MENU);
  //sen->value = gtk_image_new ();
#endif

  gtk_box_pack_start (GTK_BOX (sen->panel), sen->eventbox, FALSE,
		      FALSE, 0);
  
  if (sen->depth > 0)
    {
      gtk_box_pack_start (GTK_BOX (sen->panel), widget, FALSE, FALSE,
			  0);
    }

  gtk_box_pack_start (GTK_BOX (sen->panel), sen->entry, TRUE, TRUE,
		      0);

  gtk_box_pack_start (GTK_BOX (sen->panel), sen->value, FALSE, FALSE,
		      0);

  sen->rule_box = gtk_label_new (NULL);
  gtk_label_set_justify (GTK_LABEL (sen->rule_box), GTK_JUSTIFY_FILL);
  gtk_label_set_width_chars (GTK_LABEL (sen->rule_box), 2);

  gtk_box_pack_start (GTK_BOX (sen->panel), sen->rule_box, FALSE,
		      FALSE, 0);

  sen->mark = NULL;

  GtkTextTag * tag;
  GtkTextTagTable * table;

  table =
    gtk_text_buffer_get_tag_table (gtk_text_view_get_buffer
				   (GTK_TEXT_VIEW (sen->entry)));

  tag = gtk_text_tag_new ("hilight");
  g_object_set (G_OBJECT (tag),
		"background-gdk", the_app->bg_colors[BG_COLOR_GOOD],
		NULL);

  gtk_text_tag_table_add (table, tag);

  tag = gtk_text_tag_new ("negative");
  g_object_set (G_OBJECT (tag),
		"background-gdk", the_app->bg_colors[BG_COLOR_BAD],
		NULL);

  gtk_text_tag_table_add (table, tag);
}

/* Destroys a sentence.
 *  input:
 *    sen - the sentence to destroy.
 *  output:
 *    none.
 */
void
sentence_destroy (sentence * sen)
{
  if (sen->refs)
    destroy_list (sen->refs);
  sen->refs = NULL;

  if (sen->text)
    free (sen->text);
  sen->text = NULL;

  if (sen->vars)
    {
      int i;
      for (i = 0; sen->vars[i]; i++)
	free (sen->vars[i]);
      free (sen->vars);
    }

  sen->vars = NULL;

  sen->parent = NULL;

  if (sen->indices)
    free (sen->indices);
  sen->indices = NULL;

  if (sen->sexpr)
    free (sen->sexpr);
  sen->sexpr = NULL;

  gtk_widget_destroy (sen->panel);
  free (sen);
}

/* Copies the elements of a sentence into a sen_data structure.
 *  input:
 *    sen - the sentence to copy.
 *  output:
 *    the sen_data structure.
 */
sen_data *
sentence_copy_to_data (sentence * sen)
{
  sen_data * sd;
  int i;
  item_t * ref_itr, * var_itr;
  short * refs;

  i = 0;

  if (sen->refs && sen->refs->num_stuff > 0)
    {
      refs = (short *) calloc (sen->refs->num_stuff + 1, sizeof (int));
      CHECK_ALLOC (refs, NULL);

      ref_itr = sen->refs->head;
      for (; ref_itr != NULL; ref_itr = ref_itr->next)
	{
	  sentence * sen_itr;
	  int line_num;

	  sen_itr = ref_itr->value;
	  line_num = sen_itr->line_num;
	  refs[i++] = (short) line_num;
	}
    }
  else
    {
      refs = (short *) calloc (1, sizeof (int));
      CHECK_ALLOC (refs, NULL);
    }

  if (sen->text)
    free (sen->text);

  sen->text = sentence_copy_text (sen);
  if (!sen->text)
    return NULL;

  refs[i] = -1;

  sd = sen_data_init (sen->line_num, sen->rule,
		      sen->text, refs, sen->premise, sen->file,
		      sen->subproof, sen->depth, sen->sexpr);

  if (!sd)
    return NULL;

  sd->indices = (int *) calloc (sen->depth + 1, sizeof (int));
  CHECK_ALLOC (sd->indices, NULL);

  for (i = 0; i < sen->depth; i++)
    sd->indices[i] = sen->indices[i];
  sd->indices[i] = -1;

  return sd;
}

/* Sets the line number of a sentence.
 *  input:
 *    sen - The sentence to set the line number.
 *    new_line_no - The new line number.
 *  output:
 *    -1 on error, -2 if the line isn't meant to be set, 0 on success.
 */
int
sentence_set_line_no (sentence * sen, int new_line_no)
{
  if (new_line_no < 1)
    {
      if (sen->line_num == -1)
	return -2;

      sen->line_num = -1;
      gtk_label_set_text (GTK_LABEL (sen->line_no), NULL);
      return 0;
    }

  char * new_label;
  double label_len = log10 ((double) new_line_no);
  int sp_chk = 0;
  sen->line_num = new_line_no;

  //The length of any number in base 10 will be log10(n) + 1
  new_label = (char *) calloc ((int)label_len + 2, sizeof (char));
  CHECK_ALLOC (new_label, -1);

  sp_chk = sprintf (new_label, "%i", new_line_no);
  if (sp_chk != (int) label_len + 1)
    {
      //There is an error, so exit the program.
      fprintf (stderr, "Print Error - \
Unable to print the correct characters to a string.\n");
      return -1;
    }

  //Set the line_no label from sen.
  gtk_label_set_text (GTK_LABEL (sen->line_no), (const char *) new_label);

  //Free the memory from new_label.
  free (new_label);
}

/* Sets the font of a sentence.
 *  input:
 *    sen - The sentence to set the font of.
 *    font - The new font.
 *  output:
 *    none.
 */
void
sentence_set_font (sentence * sen, int font)
{
  int font_size;

  sen->font_resizing = 1;
  font_size = pango_font_description_get_size (the_app->fonts[font]);
  font_size /= PANGO_SCALE;

  sentence_resize_text (sen, font_size);
  LABEL_SET_FONT (sen->line_no, the_app->fonts[font]);
  ENTRY_SET_FONT (sen->entry, the_app->fonts[font]);
#ifdef LETTERS
  LABEL_SET_FONT (sen->value, the_app->fonts[font]);
#else
  //gtk_image_set_from_stock (GTK_WIDGET (sen->value), sen_values[sen->value_type], sen_value_sizes[font]);
#endif
  LABEL_SET_FONT (sen->rule_box, the_app->fonts[font]);

  sen->font_resizing = 0;
}

/* Resizes the pixbufs of a sentence.
 *  input:
 *    sen - the sentence to be resized.
 *    new_size - the font size to which to set the sentence.
 *  output:
 *    0 on success.
 */
int
sentence_resize_text (sentence * sen, int new_size)
{
  GtkTextIter iter;
  GtkTextBuffer * buffer;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sen->entry));
  gtk_text_buffer_get_start_iter (buffer, &iter);

  while (!gtk_text_iter_is_end (&iter))
    {
      GdkPixbuf * pix;
      pix = gtk_text_iter_get_pixbuf (&iter);
      if (pix)
	{
	  GdkPixbuf * new_pix, * old_pix;
	  char * val;
	  val = (char *) g_object_get_data (G_OBJECT (pix), _("conn"));

	  new_pix = sen_parent_get_conn_by_type (sen->parent,
						 val);

	  GtkTextIter next;
	  next = iter;
	  gtk_text_iter_forward_char (&next);
	  gtk_text_buffer_delete (buffer, &iter, &next);
	  gtk_text_buffer_insert_pixbuf (buffer, &iter,
					 new_pix);
	}
      else
	{
	  gtk_text_iter_forward_char (&iter);
	}
    }

  return 0;
}

/* Sets the background color of a sentence.
 *  input:
 *    sen - the sentence to change the background color of.
 *    bg_color - the index in the_app->bg_colors.
 *  output:
 *    none
 */
void
sentence_set_bg (sentence * sen, int bg_color)
{
  COLOR_TYPE inv;
  INVERT (the_app->bg_colors[bg_color], inv);
  gtk_widget_modify_bg (sen->entry, GTK_STATE_NORMAL,
			the_app->bg_colors[bg_color]);
  gtk_widget_modify_base (sen->entry, GTK_STATE_NORMAL,
			  the_app->bg_colors[bg_color]);
  gtk_widget_modify_bg (sen->entry, GTK_STATE_SELECTED, inv);
  sen->bg_color = bg_color;
  free (inv);
}

/* Sets the evaluation value of a sentence.
 *  input:
 *    sen - the sentence to change the value of.
 *    value_type - the value type to change it to.
 *  output:
 *    none.
 */
void
sentence_set_value (sentence * sen, int value_type)
{
  sen->value_type = value_type;
#ifdef LETTERS
  gtk_label_set_text (GTK_LABEL (sen->value), sen_values[value_type]);
#else
  gtk_image_set_from_stock (GTK_IMAGE (sen->value), sen_values [value_type], GTK_ICON_SIZE_MENU);
#endif
}

/* Connects the callback signals to a sentence.
 *  input:
 *    sen - the sentence to connect signals to.
 *  output:
 *    none.
 */
void
sentence_connect_signals (sentence * sen)
{
  g_signal_connect (G_OBJECT (sen->entry), "focus-in-event",
		    G_CALLBACK (sentence_focus_in), (gpointer) sen);
  g_signal_connect (G_OBJECT (sen->entry), "focus-out-event",
		    G_CALLBACK (sentence_focus_out), (gpointer) sen);
  g_signal_connect (G_OBJECT (sen->entry), "button-press-event",
		    G_CALLBACK (sentence_btn_press), (gpointer) sen);
  g_signal_connect (G_OBJECT (sen->entry), "button-release-event",
		    G_CALLBACK (sentence_btn_release), (gpointer) sen);
  g_signal_connect (G_OBJECT (sen->entry), "key-press-event",
		    G_CALLBACK (sentence_key_press), (gpointer) sen);
  g_signal_connect (G_OBJECT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (sen->entry))),
		    "changed",
		    G_CALLBACK (sentence_changed), (gpointer) sen);
  sen->sig_id = g_signal_connect (G_OBJECT (sen->entry), "size-allocate",
				  G_CALLBACK (sentence_mapped), (gpointer) sen);
}

/* Selects the references and rule of a sentence when it is selected.
 *  input:
 *    sen - the sentence that has just been selected.
 *  output:
 *    0 on success.
 */
int
sentence_in (sentence * sen)
{
  sen_parent * sp = sen->parent;
  item_t * e_itr = ls_find (sp->everything, sen);
  // Find the item in everything that corresponds to this sentence.

  sp->focused = e_itr;

  // Set the background color to cyan.
  sentence_set_bg (sen, BG_COLOR_CONC);

  if (!sen->premise)
    {
      // Toggle the rule, if one exists.
      if (sen->rule != -1)
	{
	  // Find which toggle button corresponds to this rule.
	  if (the_app->rt->toggled != sen->rule)
	    {
	      the_app->rt->user = 0;
	      TOGGLE_BUTTON (the_app->rt->rules[sen->rule]);
	      the_app->rt->user = 1;
	    }
	}
      else if (the_app->rt->toggled != -1)
	{
	  the_app->rt->user = 0;
	  TOGGLE_BUTTON (the_app->rt->rules[the_app->rt->toggled]);
	  the_app->rt->user = 1;
	}

      // Set the background color of the references.
      item_t * r_itr = sen->refs->head;
      for (; r_itr; r_itr = r_itr->next)
	{
	  int entire = 1;

	  entire = sentence_check_entire (sen, r_itr->value);
	  sentence_set_reference (SENTENCE (r_itr->value), 1, entire);
	}
    }
  else if (sp->type == SEN_PARENT_TYPE_PROOF
	   && the_app->rt->toggled != -1)
    {
      the_app->rt->user = 0;
      TOGGLE_BUTTON (the_app->rt->rules[the_app->rt->toggled]);
      the_app->rt->user = 1;
    }

  return 0;
}

/* Deselects the references and rule of a sentence when it is deselected.
 *  input:
 *    sen - the newly deselected sentence.
 *  output:
 *    0 on success, -1 on memory error.
*/
int
sentence_out (sentence * sen)
{
  sen_parent * sp = sen->parent;
  if (!sp)
    return -1;

  GtkTextBuffer * buffer;
  GtkTextIter start, end;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sen->entry));

  gtk_text_buffer_get_bounds (buffer, &start, &end);
  gtk_text_buffer_remove_tag_by_name (buffer, "hilight", &start, &end);
  gtk_text_buffer_remove_tag_by_name (buffer, "negative", &start, &end);

  // Reset the background color.
  // Reset the background color of the references.
  sentence_set_bg (sen, BG_COLOR_DEFAULT);

  if (!sen->premise)
    {

      item_t * ref_itr = sen->refs->head;
      for (; ref_itr; ref_itr = ref_itr->next)
	{
	  int entire;

	  entire = sentence_check_entire (sen, ref_itr->value);
	  sentence_set_reference (SENTENCE (ref_itr->value), 0, entire);
	}
    }

  return 0;
}

/* Selects a sentence as a reference.
 *  input:
 *    sen - the sentence that has just been selected.
 *  output:
 *    0 on success, -1 on error, -2 on memory error.
 */
int
select_reference (sentence * sen)
{
  // Confirm that focused exists.
  // Determine whether or not this sentence is already a reference.
  // Add/remove sen to/from focused's references.
  // Set the background color of sen.

  sen_parent * sp = sen->parent;

  if (the_app->verbose)
    printf ("Selecting reference.\n");

  if (!sp->focused || SENTENCE (sp->focused->value)->premise)
    return -1;

  sentence * fcs_sen;
  fcs_sen = sp->focused->value;

  if (sen->line_num >= fcs_sen->line_num)
    {
      if (the_app->verbose)
	printf ("Must select reference that comes before focused.\n");
      return -1;
    }

  sentence * ref_sen = sen;
  int entire, ret;

  // Get indices of each.
  // This isn't necessary if the sentence is a premise, or if it has depth == zero.
  ret = sentence_can_select_as_ref (fcs_sen, sen);
  if (ret < 0)
    {
      entire = 1;
      ret *= -1;
    }
  else
    {
      entire = 0;
    }

  item_t * ref_itr;
  ref_itr = ls_nth (sp->everything, ret - 1);
  ref_sen = ref_itr->value;

  if (ref_sen->reference)
    {
      // Remove sen from focused's references.
      if (the_app->verbose)
	printf ("Removing reference.\n");

      ls_rem_obj_value (fcs_sen->refs, ref_sen);
      sentence_set_reference (ref_sen, 0, entire);
    }
  else
    {
      // Add sen to focused's references.
      if (the_app->verbose)
	printf ("Adding reference.\n");

      item_t * itm;
      itm = ls_push_obj (fcs_sen->refs, ref_sen);
      if (!itm)
	return -2;

      sentence_set_reference (ref_sen, 1, entire);
    }

  if (sp->type == SEN_PARENT_TYPE_PROOF)
    {
      undo_info ui;
      ui.type = -1;
      ret = aris_proof_set_changed ((aris_proof *) sp, 1, ui);
      if (ret < 0)
	return -2;
    }

  return 0;
}

/* Selects a sentence.
 *  input:
 *    sen - the sentence being selected.
 *  output:
 *    0 on success, -1 on error (memory).
 */
int
select_sentence (sentence * sen)
{
  sen_parent * sp;

  sp = sen->parent;

  if (sp->type == SEN_PARENT_TYPE_GOAL)
    return 0;

  if (!ARIS_PROOF (sp)->selected)
    return 0;

  if (sen->selected)
    {
      if (the_app->verbose)
	printf ("Deselecting sentence.\n");

      ls_rem_obj_value (ARIS_PROOF (sp)->selected, sen);
      if (sen->subproof)
	{
	  // Remove the entire subproof.
	  int sen_depth;
	  sen_depth = sen->depth;

	  item_t * ev_itr;
	  ev_itr = ls_find (sp->everything, sen);
	  for (; ev_itr; ev_itr = ev_itr->next)
	    {
	      sentence * ev_sen;
	      ev_sen = ev_itr->value;
	      if (ev_sen->depth < sen_depth)
		break;

	      ls_rem_obj_value (ARIS_PROOF (sp)->selected, ev_sen);
	    }
	}
      sentence_set_selected (sen, 0);
    }
  else
    {
      if (the_app->verbose)
	printf ("Selecting sentence.\n");

      item_t * itm;
      itm = ls_push_obj (ARIS_PROOF (sp)->selected, sen);
      if (!itm)
	return -1;

      if (sen->subproof)
	{
	  // Add entire subproof.
	  int sen_depth;
	  sen_depth = sen->depth;

	  item_t * ev_itr;
	  ev_itr = ls_find (sp->everything, sen);
	  for (ev_itr = ev_itr->next; ev_itr; ev_itr = ev_itr->next)
	    {
	      sentence * ev_sen;
	      ev_sen = ev_itr->value;
	      if (ev_sen->depth < sen_depth)
		break;

	      itm = ls_push_obj (ARIS_PROOF (sp)->selected, ev_sen);
	      if (!itm)
		return -1;
	    }
	}

      sentence_set_selected (sen, 1);
    }

  return 0;
}

/* Gets the index of a text iter overall.
 *  input:
 *    iter - the text iter of which to get the index.
 *  output:
 *    The index in the buffer of the text iter.
 */
int
get_index (GtkTextIter * iter)
{
  GtkTextBuffer * buffer;
  int ret, line, i;

  buffer = gtk_text_iter_get_buffer (iter);
  line = gtk_text_iter_get_line (iter);
  ret = 0;

  for (i = 0; i < line; i++)
    {
      GtkTextIter tmp_itr;
      gtk_text_buffer_get_iter_at_line (buffer, &tmp_itr, i);
      ret += gtk_text_iter_get_chars_in_line (&tmp_itr);
    }

  ret += gtk_text_iter_get_line_offset (iter);
  return ret;
}

/* Gets the iterator of the text buffer at the given index.
 *  input:
 *    buffer - the text buffer.
 *    iter - a pointer to the text iterator to get the iterator.
 *    index - the index of which to get the iterator.
 *  output:
 *    none.
 */
void
get_iter_at_index (GtkTextBuffer * buffer, GtkTextIter * iter, int index)
{
  int i, offset;
  offset = i = 0;

  while (1)
    {
      GtkTextIter tmp_itr;
      int tmp;

      gtk_text_buffer_get_iter_at_line (buffer, &tmp_itr, i);
      tmp = gtk_text_iter_get_chars_in_line (&tmp_itr);
      if (tmp + offset >= index)
	break;
      offset += tmp;
      i++;
    }

  gtk_text_buffer_get_iter_at_line_offset (buffer, iter, i,
					   index - offset);
}


/* Processes a key press for a sentence.
 *  input:
 *    sen - the sentence being processed.
 *    key - the keycode for the key being pressed.
 *    ctrl - whether or not ctrl is being held.
 *  output:
 *    1 if the event should be propogated, 0 otherwise.
 */
int
sentence_key (sentence * sen, int key, int ctrl)
{
  sen_parent * sp = sen->parent;
  int ret = 1;
  GtkTextBuffer * buffer;
  GtkTextIter start, end;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sen->entry));

  gtk_text_buffer_get_bounds (buffer, &start, &end);
  gtk_text_buffer_remove_tag_by_name (buffer, "hilight", &start,
				      &end);
  gtk_text_buffer_remove_tag_by_name (buffer, "negative", &start,
				      &end);

  int tmp_pos, offset;

  if (ctrl)
    {
      char * insert_char = NULL;
      GdkPixbuf * pixbuf = NULL;
      int font_size;

      font_size =
	pango_font_description_get_size (the_app->fonts[sen->parent->font]);
      font_size /= PANGO_SCALE;

      switch (key)
	{
	case GDK_KEY_7:
	  pixbuf = sen_parent_get_conn_by_type (sen->parent, AND);
	  break;
	case GDK_KEY_backslash:
	  pixbuf = sen_parent_get_conn_by_type (sen->parent, OR);
	  break;
	case GDK_KEY_grave:
	  pixbuf = sen_parent_get_conn_by_type (sen->parent, NOT);
	  break;
	case GDK_KEY_4:
	  pixbuf = sen_parent_get_conn_by_type (sen->parent, CON);
	  break;
	case GDK_KEY_5:
	  pixbuf = sen_parent_get_conn_by_type (sen->parent, BIC);
	  break;
	case GDK_KEY_2:
	  pixbuf = sen_parent_get_conn_by_type (sen->parent, UNV);
	  break;
	case GDK_KEY_3:
	  pixbuf = sen_parent_get_conn_by_type (sen->parent, EXL);
	  break;
	case GDK_KEY_1:
	  pixbuf = sen_parent_get_conn_by_type (sen->parent, TAU);
	  break;
	case GDK_KEY_6:
	  pixbuf = sen_parent_get_conn_by_type (sen->parent, CTR);
	  break;
	case GDK_KEY_semicolon:
	  pixbuf = sen_parent_get_conn_by_type (sen->parent, ELM);
	  break;
	case GDK_KEY_period:
	  pixbuf = sen_parent_get_conn_by_type (sen->parent, NIL);
	  break;
	case GDK_KEY_space:
	  insert_char = " ";
	  break;
 	default:
	  break;
	}

      if (pixbuf)
	{
	  GtkTextIter iter;
	  GtkTextMark * mark;

	  mark = gtk_text_buffer_get_insert (buffer);
	  gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
	  gtk_text_buffer_insert_pixbuf (buffer, &iter, pixbuf);
	}

      if (insert_char)
	gtk_text_buffer_insert_at_cursor (buffer, insert_char, -1);
    }
  else
    {
      item_t * dst;
      GtkTextIter pos, chk_pos, c_pos;
      gunichar o_char, c_char;
      int got_par = 0, mod = 0, chk;

      gtk_text_buffer_get_iter_at_mark (buffer, &pos,
					gtk_text_buffer_get_insert (buffer));
      chk_pos = pos;

      switch (key)
	{
	case GDK_KEY_Up:
	  dst = (sp->focused == sp->everything->head)
	    ? sp->everything->tail : sp->focused->prev;
	  if (the_app->verbose)
	    printf ("Got Key Up\n");
	  gtk_widget_grab_focus (SENTENCE (dst->value)->entry);
	  ret = 0;
	  break;
	case GDK_KEY_Down:
	  dst = (sp->focused == sp->everything->tail)
	    ? sp->everything->head : sp->focused->next;
	  if (the_app->verbose)
	    printf ("Got Key Down\n");
	  gtk_widget_grab_focus (SENTENCE (dst->value)->entry);
	  ret = 0;
	  break;
	case GDK_KEY_Left:
	case GDK_KEY_Right:
	  if (the_app->verbose)
	    {
	      if (key == GDK_KEY_Left)
		printf ("Got Key Left\n");
	      else
		printf ("Got Key Right\n");
	    }

	  chk = (key == GDK_KEY_Left)
	    ? gtk_text_iter_backward_char (&chk_pos)
	    : gtk_text_iter_forward_char (&chk_pos);

	  break;
	}

      if (key != GDK_KEY_Up && key != GDK_KEY_Down)
	{
	  c_pos = chk_pos;
	  gtk_text_iter_backward_char (&c_pos);
	  o_char = gtk_text_iter_get_char (&chk_pos);
	  c_char = gtk_text_iter_get_char (&c_pos);
	  if (o_char == '(')
	    {
	      got_par = 1;
	      pos = chk_pos;
	      mod = 0;
	    }

	  if (c_char == ')')
	    {
	      got_par = 2;
	      pos = c_pos;
	      mod = -1;
	    }
	}
	  
      if (got_par)
	{
	  unsigned char * tmp_str = NULL;
	  GtkTextIter oth_pos;

	  offset = get_index (&pos);

	  if (got_par == 1)
	    {
	      tmp_pos = parse_parens (sen->text, offset, NULL);
	    }
	  else
	    {
	      tmp_pos = reverse_parse_parens (sen->text, offset,
					      NULL);
	      if (tmp_pos == -2)
		return -1;
	    }

	  GtkTextIter semi;
	  semi = pos;
	  gtk_text_iter_forward_char (&semi);

	  if (tmp_pos < 0)
	    {
	      gtk_text_buffer_apply_tag_by_name (buffer, "negative",
						 &pos, &semi);
	    }
	  else
	    {
	      gtk_text_buffer_apply_tag_by_name (buffer, "hilight",
						 &pos, &semi);
	      get_iter_at_index (buffer, &oth_pos, tmp_pos);
	      semi = oth_pos;
	      gtk_text_iter_forward_char (&semi);
	      gtk_text_buffer_apply_tag_by_name (buffer, "hilight",
						 &oth_pos, &semi);
	    }
	}
    }
  return ret;
}


/* Sets the reference state of a sentence.
 *  input:
 *    sen - the sentence to set the reference state of.
 *    reference - whether this sentence is being added or removed as a reference.
 *    entire_subproof - whether or not the entire subproof should be selected.
 *  output:
 *    none.
 */
void
sentence_set_reference (sentence * sen, int reference, int entire_subproof)
{
  if (reference)
    sentence_set_bg (sen, BG_COLOR_REF);
  else
    sentence_set_bg (sen, BG_COLOR_DEFAULT);

  if (sen->subproof && entire_subproof)
    {
      item_t * sub_itr;

      sub_itr = ls_find (sen->parent->everything, sen);

      for (sub_itr = sub_itr->next; sub_itr; sub_itr = sub_itr->next)
	{
	  sentence * sub_sen;
	  sub_sen = sub_itr->value;

	  if (sub_sen->depth < sen->depth)
	    break;

	  if (reference)
	    sentence_set_bg (sub_sen, BG_COLOR_REF);
	  else
	    sentence_set_bg (sub_sen, BG_COLOR_DEFAULT);
	}
    }

  sen->reference = reference;
}

/* Sets the selected state of a sentence.
 *  input:
 *   sen - The sentence that is being selected.
 *   selected - Whether or not the sentence is being selected.
 *  output:
 *   none.
 */
void
sentence_set_selected (sentence * sen, int selected)
{
  if (selected)
    sentence_set_bg (sen, BG_COLOR_SEL);
  else
    sentence_set_bg (sen, BG_COLOR_DEFAULT);

  if (sen->subproof)
    {
      item_t * sub_itr;

      sub_itr = ls_find (sen->parent->everything, sen);

      for (sub_itr = sub_itr->next; sub_itr; sub_itr = sub_itr->next)
	{
	  sentence * sub_sen;
	  sub_sen = sub_itr->value;

	  if (sub_sen->depth < sen->depth)
	    break;

	  if (selected)
	    sentence_set_bg (sub_sen, BG_COLOR_SEL);
	  else
	    sentence_set_bg (sub_sen, BG_COLOR_DEFAULT);
	}
    }

  sen->selected = selected;
}

/* Gets the correct text from a sentence text view.
 *  input:
 *    sen - the sentence from which to get the text.
 *  output:
 *    the text of the sentence, or NULL on error.
 */
char *
sentence_copy_text (sentence * sen)
{
  char * ret_str;

  GtkTextBuffer * buffer;
  GtkTextIter start, end;
  int text_len;
  int i;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sen->entry));
  gtk_text_buffer_get_bounds (buffer, &start, &end);

  i = 0;
  ret_str = (char *) calloc (1, sizeof (char));
  CHECK_ALLOC (ret_str, NULL);

  while (!gtk_text_iter_is_end (&start))
    {
      GdkPixbuf * pixbuf;
      pixbuf = gtk_text_iter_get_pixbuf (&start);
      if (pixbuf)
	{
	  // Determine what this is, and add the
	  // corresponding character to ret_str;
	  int alloc_size;
	  unsigned char * new_char;
	  const char * val;

	  val = (char *) g_object_get_data (G_OBJECT (pixbuf),
					    _("conn"));
	  ret_str = (char *) realloc (ret_str,
				      (i + CL + 1) * sizeof (char));
	  CHECK_ALLOC (ret_str, NULL);
	  strcpy (ret_str + i, val);
	  i += CL;
	}
      else
	{
	  char * c;
	  end = start;
	  gtk_text_iter_forward_char (&end);

	  c = gtk_text_iter_get_text (&start, &end);

	  ret_str = (char *) realloc (ret_str, (i + 2)
				      * sizeof (char));
	  CHECK_ALLOC (ret_str, NULL);
	  
	  ret_str[i++] = *c;
	}

      gtk_text_iter_forward_char (&start);
    }

  ret_str[i] = '\0';

  // Handle comments.
  // Will need to fix this up, since this means that sentences aren't saved.

  unsigned char * semi_str, * fin_str;
  semi_str = strchr (ret_str, ';');
  if (semi_str)
    {
      int ret_len;
      semi_str[0] = '\0';
      ret_len = strlen (ret_str);
      fin_str = (unsigned char *) calloc (ret_len + 1, sizeof (char));
      CHECK_ALLOC (fin_str, NULL);
      memcpy (fin_str, ret_str, ret_len);
      free (ret_str);
      ret_str = fin_str;
    }

  return ret_str;
}

/* Sets the text of the text view of a sentence from the text.
 *  input:
 *    sen - the sentence of which to set the text.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
sentence_paste_text (sentence * sen)
{
  int i;

  GtkTextBuffer * buffer;
  GtkTextIter end;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sen->entry));
  gtk_text_buffer_get_start_iter (buffer, &end);

  for (i = 0; sen->text[i]; i++)
    {
      if (IS_TYPE_CONN (sen->text + i, gui_conns)
	  || IS_TYPE_CONN (sen->text + i, cli_conns))
	{
	  GdkPixbuf * pix, * new_pix;

	  unsigned char * conn;
	  int len;

	  if (IS_TYPE_CONN (sen->text + i, gui_conns))
	    {
	      if (!strncmp (sen->text + i, gui_conns.not, gui_conns.nl))
		len = gui_conns.nl;
	      else
		len = gui_conns.cl;
	    }
	  else
	    {
	      if (!strncmp (sen->text + i, NOT, NL))
		len = NL;
	      else
		len = CL;
	    }

	  conn = (unsigned char *) calloc (len + 1, sizeof (char));
	  CHECK_ALLOC (conn, -1);
	  
	  strncpy (conn, sen->text + i, len);
	  conn[len] = '\0';

	  pix = sen_parent_get_conn_by_type (sen->parent,
					     conn);
	  free (conn);

	  if (!pix)
	    return -2;

	  // No need to resize this, since sentence_set_font
	  // will be called after this, and it handles it instead.

	  gtk_text_buffer_insert_pixbuf (buffer, &end, pix);
	  i += len - 1;
	}
      else
	{
	  gtk_text_buffer_insert (buffer, &end, sen->text + i, 1);
	}
    }

  return 0;
}

/* Processes a text change in a sentence.
 *  input:
 *    sen - the sentence to process a change in.
 *  ouput:
 *    0 on success, -1 on memory error.
 */
int
sentence_text_changed (sentence * sen)
{
  if (sen->font_resizing || sen->parent->undo)
    return 0;

  sen_parent * sp = sen->parent;
  sentence_set_value (sen, VALUE_TYPE_BLANK);

  item_t * e_itr = ls_find (sp->everything, sen);

  for (e_itr = sp->everything->head; e_itr; e_itr = e_itr->next)
    {
      list_t * e_refs;
      e_refs = SENTENCE (e_itr->value)->refs;
      if (e_refs)
	{
	  item_t * r_itr;
	  r_itr = ls_find (e_refs, sen);

	  if (r_itr)
	    sentence_set_value (SENTENCE (e_itr->value), VALUE_TYPE_BLANK);
	}
    }

  if (sen->sexpr)
    {
      free (sen->sexpr);
      sen->sexpr = NULL;
    }

  char * text;
  GtkTextBuffer * buffer;
  GtkTextIter start, end, semi;
  int text_len, old_len;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sen->entry));
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  text = sentence_copy_text (sen);
  text_len = strlen (text);

  int diff_pos;

  old_len = strlen (sen->text);
  diff_pos = find_difference (sen->text, (unsigned char *) text);

  if (0)
    {
      //TODO: Check for semi-colon and set mark.
      if (text_len > old_len)
	{
	  if (sen->text[diff_pos] == ';')
	    {
	      GtkTextIter cur_pos;
	      gtk_text_buffer_get_iter_at_mark (buffer, &cur_pos,
						gtk_text_buffer_get_insert (buffer));
	  
	      if (sen->mark)
		{
		  gtk_text_buffer_get_iter_at_mark (buffer, &semi, sen->mark);

		  int mark_off, cur_off;
		  mark_off = gtk_text_iter_get_offset (&semi);
		  cur_off = gtk_text_iter_get_offset (&cur_pos);

		  if (cur_off < mark_off)
		    {
		      gtk_text_buffer_move_mark (buffer, sen->mark, &cur_pos);
		    }
		}
	      else
		{
		  sen->mark = gtk_text_mark_new (SEMI_NAME, TRUE);
		  gtk_text_buffer_add_mark (buffer, sen->mark, &cur_pos);
		}
	    }
	}
      else
	{
	  if (text[diff_pos] == ';')
	    {
	      GtkTextIter cur_pos;
	      gtk_text_buffer_get_iter_at_mark (buffer, &cur_pos,
						gtk_text_buffer_get_insert (buffer));
	    }
	}
    }

  undo_info ui;
  ui = undo_info_init_one (NULL, sen, UIT_MOD_TEXT);
  if (ui.type == -1)
    return -1;

  if (sp->type == SEN_PARENT_TYPE_PROOF)
    {
      int ret;

      ret = aris_proof_set_changed (ARIS_PROOF (sp), 1, ui);
      if (ret < 0)
	return -1;

      gtk_widget_modify_bg (sen->eventbox, GTK_STATE_NORMAL, NULL);

      if (ARIS_PROOF (sp)->goal->goals->num_stuff > 0)
	{
	  item_t * mod_itm;
	  mod_itm = ARIS_PROOF (sp)->goal->goals->head;
	  for (; mod_itm; mod_itm = mod_itm->next)
	    {
	      if (SENTENCE (mod_itm->value)->line_num == sen->line_num)
		{
		  sentence_set_line_no (SENTENCE (mod_itm->value), -1);
		  sentence_set_value (SENTENCE (mod_itm->value), VALUE_TYPE_BLANK);
		  break;
		}
	    }
	}
    }
  else
    {
      // Otherwise, it's a goal.
      int ret;

      ret = aris_proof_set_changed (GOAL (sp)->parent, 1, ui);
      if (ret < 0)
	return -1;

      item_t * mod_itm;
      sentence * mod_sen;
      mod_itm = ls_nth ((SEN_PARENT (GOAL (sp)->parent)->everything), sen->line_num);
      if (mod_itm)
	{

	  mod_sen = mod_itm->value;
	  gtk_widget_modify_bg (mod_sen->eventbox, GTK_STATE_NORMAL, NULL);
	  gtk_label_set_text (GTK_LABEL (sen->line_no), "");
	  sen->line_num = 0;
	}
    }

  if (sen->text)
    free (sen->text);

  sen->text = (unsigned char *) calloc (text_len + 1, sizeof (char));
  CHECK_ALLOC (sen->text, -1);

  strcpy (sen->text, text);
  free (text);
  return 0;
}

/* Checks if only the subproof premise or the entire subproof should be selected.
 *  input:
 *    sen - the sentence that is adding a reference.
 *    ref - the sentence being added as a reference.
 *  output:
 *    0 if only the premise should be selected, 1 otherwise.
 */
int
sentence_check_entire (sentence * sen, sentence * ref)
{
  if (!ref->subproof)
    return 0;

  if (ref->depth > sen->depth)
    return 1;

  int i;
  for (i = 0; i < ref->depth; i++)
    {
      if (ref->indices[i] != sen->indices[i])
	break;
    }

  int ret;
  if (ref->indices[i] == -1)
    ret = 0;
  else
    ret = 1;

  return ret;
}

/* Checks a sentence's rule against the boolean rules.
 *  input:
 *    sen - the sentenc being checked.
 *    boolean - whether or not boolean mode is being activated.
 *  output:
 *    0 if the rule does not check out, 1 if it does.
 */
int
sentence_check_boolean_rule (sentence * sen, int boolean)
{
  if (!boolean)
    return 1;

  int bool_okay = 1;

  if (sen->rule < RULE_DM || sen->rule == RULE_EQ || sen->rule == RULE_EP
      || (sen->rule >= RULE_EG && sen->rule <= RULE_SP))
    bool_okay = 0;

  return bool_okay;
}

/* Determines whether or not one sentence can select the other as a reference.
 *  input:
 *    sen - The sentence looking to select a reference.
 *    ref - The sentence that is attempting to be selected.
 *  output:
 *    0 if ref cannot be selected by sen
 *    The common line between them,
 *    The negation of the common line if an entire subproof is being selected.
 */
int
sentence_can_select_as_ref (sentence * sen, sentence * ref)
{
  if (ref->line_num >= sen->line_num)
    {
      if (the_app->verbose)
	printf ("Must select reference that comes before focused.\n");
      return 0;
    }

  int entire;

  // Get indices of each.
  // This isn't necessary if the sentence is a premise, or if it has depth == zero.
  if (ref->premise || ref->indices[0] == -1)
    return ref->line_num;

  int common_line;
  int i;

  common_line = 0;
  for (i = 0; sen->indices[i] != -1 && ref->indices[i] != -1; i++)
    {
      if (sen->indices[i] != ref->indices[i])
	break;
    }

  // If the set of the reference sentence's indices is a subset of
  // the set of the focused sentence's indices, then the sentence can
  // be selected as a reference.

  if (ref->indices[i] == -1)
    {
      common_line = ref->line_num;
      entire = 0;
    }
  else
    {
      common_line = ref->indices[i];
      entire = 1;
    }

  if (ref->indices[i] != -1 && sen->indices[i] == -1)
    entire = 1;

  if (entire == 1)
    common_line *= -1;

  return common_line;
}

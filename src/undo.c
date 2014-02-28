/* Functions for handling undo and redo.

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
#include <stdio.h>
#include "aris-proof.h"
#include "undo.h"
#include "list.h"
#include "vec.h"
#include "sen-data.h"
#include "sentence.h"

/* Initialize and undo information object.
 *  input:
 *    ap - the aris proof object to which this undo information belongs.
 *    sens - a list of sentences that have been modified.
 *    type - the type of action by which this was generated.
 *  output:
 *    A newly initialized undo information object.
 */
undo_info
undo_info_init (aris_proof * ap, list_t * sens, int type)
{
  undo_info ret;
  ret.type = -1;

  item_t * it, * nit;
  ret.ls = init_list ();
  if (!ret.ls)
    return ret;

  for (it = sens->head; it;)
    {
      nit = it->next;
      sen_data * sd = it->value;
      ls_push_obj (ret.ls, sd);
      free (it);
      it = nit;
    }
  free (sens);

  ret.type = type;
  ret.stamp = time (NULL);

  return ret;
}

/* Initialize an undo information object from a single sentence.
 *  input:
 *    ap - the aris proof to which this object belongs.
 *    sen - the sentence that was modified.
 *    type - the type of action that generated this.
 *  output:
 *    A newly initialized undo information object.
 */
undo_info
undo_info_init_one (aris_proof * ap, sentence * sen, int type)
{
  undo_info ret;
  ret.type = -1;

  list_t * sens;

  sens = init_list ();
  if (!sens)
    return ret;

  sen_data * sd;
  unsigned char * text;
  text = strdup (sentence_get_text (sen));
  if (!text)
    return ret;

  sd = sentence_copy_to_data (sen);
  if (!sd)
    return ret;

  if (sd->text)
    free (sd->text);
  sd->text = text;

  //fprintf (stderr, "ui_init: sd->line_num == %i\n", sd->line_num);

  ls_push_obj (sens, sd);

  ret = undo_info_init (ap, sens, type);

  return ret;
}

/* Destroy an undo information object.
 *  input:
 *    ui - the undo information object to destroy.
 *  output:
 *    None.
 */
void
undo_info_destroy (undo_info ui)
{
  item_t * it;
  sen_data * sd;
  if (ui.ls)
    {
      for (it = ui.ls->head; it;)
        {
          item_t * next = it->next;
          sd = (sen_data *) it->value;
          sen_data_destroy (sd);
          free (it);
          it = next;
        }

      free (ui.ls);
    }
  ui.ls = NULL;
}


undo_op
undo_determine_op (int undo, int type)
{
  undo_op op;

  switch (type)
    {
    case UIT_MOD_TEXT:
      op = &undo_op_mod;
      break;

    case UIT_ADD_SEN:
      op = undo ? &undo_op_remove : &undo_op_add;
      break;

    case UIT_REM_SEN:
      op = undo ? &undo_op_add : &undo_op_remove;
      break;
    }

  return op;
}

void
undo_sen (int undo, aris_proof * ap, item_t * itm, sentence * sen, sen_data * sd)
{
  if (undo)
    {
      SEN_PARENT (ap)->focused = itm;
      aris_proof_remove_sentence (ap, sen);
    }
  else
    {
      if (itm)
        itm = itm->prev;
      else
        itm = SEN_PARENT (ap)->everything->tail;
      SEN_PARENT (ap)->focused = itm;
      aris_proof_create_sentence (ap, sd, 0);
    }
}

int
undo_op_remove (aris_proof * ap, undo_info * ui)
{
  list_t * ls;
  ls = init_list ();
  if (!ls)
    return -1;

  item_t * itm, * ui_itr;
  int ln;

  for (ui_itr = ui->ls->head; ui_itr; ui_itr = ui_itr->next)
    {
      sen_data * sd = ui_itr->value;
      for (itm = SEN_PARENT (ap)->everything->head; itm; itm = itm->next)
        {
          sentence * sen = (sentence*) itm->value;
          ln = sentence_get_line_no (sen);
          if (ln == sd->line_num)
            {
              ls_push_obj (ls, sen);
              continue;
            }
        }
    }

  for (itm = ls->head; itm; itm = itm->next)
    {
      sentence * sen = itm->value;
      aris_proof_remove_sentence (ap, sen);
    }

  return 0;
}

int
undo_op_add (aris_proof * ap, undo_info * ui)
{
  item_t * ui_itr, * itm;

  for (ui_itr = ui->ls->head; ui_itr; ui_itr = ui_itr->next)
    {
      sen_data * sd;
      int ln;
      sentence * sen;

      sd = (sen_data *) ui_itr->value;

      for (itm = SEN_PARENT (ap)->everything->head; itm; itm = itm->next)
        {
          sen = (sentence *) itm->value;
          ln = sentence_get_line_no (sen);
          if (ln >= sd->line_num)
            break;
        }

      if (itm)
        itm = itm->prev;
      else
        itm = SEN_PARENT (ap)->everything->tail;
      SEN_PARENT (ap)->focused = itm;
      aris_proof_create_sentence (ap, sd, 0);
    }

  return 0;
}

int
undo_op_mod (aris_proof * ap, undo_info * ui)
{
  item_t * ui_itr, * itm;

  for (ui_itr = ui->ls->head; ui_itr; ui_itr = ui_itr->next)
    {
      sen_data * sd;
      GtkTextBuffer * buffer;
      int ln;
      sentence * sen;

      sd = (sen_data *) ui_itr->value;

      for (itm = SEN_PARENT (ap)->everything->head; itm; itm = itm->next)
        {
          sen = (sentence *) itm->value;
          ln = sentence_get_line_no (sen);
          if (ln >= sd->line_num)
            break;
        }


      unsigned char * old_text = strdup (sentence_get_text (sen));
      SEN_PARENT(ap)->undo = 1;
      int ret = sentence_set_text (sen, sd->text);
      if (ret == -1)
        return -1;

      free (sd->text);
      sd->text = strdup (old_text);
      free (old_text);

      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sen->entry));
      gtk_text_buffer_set_text (buffer, "", -1);
      sentence_paste_text (sen);
      SEN_PARENT(ap)->undo = 0;
    }

  return 0;
}


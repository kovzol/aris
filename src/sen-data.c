/* Functions for handling sentence data.

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
#include "sen-data.h"
#include "process.h"
#include "list.h"
#include "vec.h"
#include "rules.h"
#include "var.h"
#include "aio.h"
#include "proof.h"
#include "interop-isar.h"

/* Initializes the sentence data.
 *  input:
 *    line_num - the line number to set to this sentence data.
 *    rule - the rule to set.
 *    text - the text to set.
 *    refs - the references for this sentence data.
 *    premise - whether or not this is a premise.
 *    file - the file name to set, in the event that rule == RULE_LM.
 *    subproof - whether or not this starts a subproof.
 *    depth - the depth of this sentence.
 *    sexpr - the sexpr text of this sentence.
 *  output:
 *    the newly initialized sentence data, or NULL on error.
 */
sen_data *
sen_data_init (int line_num, int rule, unsigned char * text,
               short * refs, int premise, unsigned char * file,
               int subproof, int depth, unsigned char * sexpr)
{
  sen_data * sd;
  sd = (sen_data *) calloc (1, sizeof (sen_data));
  CHECK_ALLOC (sd, NULL);

  sd->line_num = line_num;
  sd->rule = rule;
  sd->text = sd->file = sd->sexpr = NULL;

  if (text)
    {
      sd->text = strdup (text);
      CHECK_ALLOC (sd->text, NULL);
    }

  if (file)
    {
      sd->file = strdup (file);
      CHECK_ALLOC (sd->file, NULL);
    }

  if (sexpr)
    {
      sd->sexpr = strdup (sexpr);
      CHECK_ALLOC (sd->sexpr, NULL);
    }

  if (refs)
    {
      int len = 0;
      while (refs[len] != REF_END)
        len++;

      sd->refs = (short*) calloc (len + 1, sizeof (short));
      CHECK_ALLOC (sd->refs, NULL);
      memcpy (sd->refs, refs, sizeof(short)*(len+1));
    }
  else
    {
      sd->refs = (short*) calloc (1, sizeof(short));
      CHECK_ALLOC (sd->refs, NULL);
      sd->refs[0] = REF_END;
    }

  sd->premise = premise;
  sd->depth = depth;
  sd->subproof = subproof;
  sd->indices = NULL;

  return sd;
}

/* Destroys sentence data.
 *  input:
 *    sd - the sentence data to destroy.
 *  output:
 *    none.
 */
void
sen_data_destroy (sen_data * sd)
{
  sd->line_num = sd->rule = -1;

  if (sd->text)
    free (sd->text);
  sd->text = NULL;

  if (sd->refs)
    free (sd->refs);
  sd->refs = NULL;

  if (sd->sexpr)
    free (sd->sexpr);
  sd->sexpr = NULL;

  if (sd->indices)
    free (sd->indices);
  sd->indices = NULL;

  free (sd);
}

/* Copies the data from one sentence data object to another.
 *  input:
 *    old_sd - the sentence data object from which to copy.
 *    new_sd - the sentence data object to which to copy.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
sen_data_copy (sen_data * old_sd, sen_data * new_sd)
{
  new_sd->line_num = old_sd->line_num;
  new_sd->rule = old_sd->rule;
  new_sd->text = new_sd->file = new_sd->sexpr = NULL;
  new_sd->premise = old_sd->premise;
  new_sd->depth = old_sd->depth;
  new_sd->subproof = old_sd->subproof;

  if (old_sd->text)
    {
      new_sd->text = strdup (old_sd->text);
      CHECK_ALLOC (new_sd->text, AEC_MEM);
    }

  if (old_sd->file)
    {
      new_sd->file = strdup (old_sd->file);
      CHECK_ALLOC (new_sd->file, AEC_MEM);
    }

  if (old_sd->sexpr)
    {
      new_sd->sexpr = strdup (old_sd->sexpr);
      CHECK_ALLOC (new_sd->sexpr, AEC_MEM);
    }

  new_sd->refs = NULL;
  new_sd->indices = NULL;

  int len;

  // Determine length of old_sd->refs.
  // Allocate the memory, and memcpy.
  // This does need to be len+1, to catch the -1.
  if (old_sd->refs)
    {
      len = 0;
      while (old_sd->refs[len] != REF_END)
        len++;

      new_sd->refs = (short*) calloc (len + 1, sizeof (short));
      CHECK_ALLOC (new_sd->refs, AEC_MEM);
      memcpy (new_sd->refs, old_sd->refs, sizeof(short)*(len+1));
    }
  else
    {
      new_sd->refs = (short*) calloc (1, sizeof(short));
      CHECK_ALLOC (new_sd->refs, AEC_MEM);
      new_sd->refs[0] = REF_END;
    }

  if (old_sd->indices)
    {
      len = old_sd->depth;
      new_sd->indices = (int*) calloc (len + 1, sizeof (int));
      CHECK_ALLOC (new_sd->indices, AEC_MEM);
      memcpy (new_sd->indices, old_sd->indices, sizeof(int)*(len+1));
    }
  else
    {
      new_sd->indices = (int*) calloc (1, sizeof (int));
      CHECK_ALLOC (new_sd->indices, AEC_MEM);
      new_sd->indices[0] = -1;
    }

  return 0;
}

/* Convert text to sexpr form.
 *  input:
 *    text - the text to convert to sexpr form.
 *    sexpr - a pointer to a string that receives the sexpr form.
 *  output:
 *    0 on succes, -1 on memory error, -2 on other error.
 */
int
sen_convert_sexpr (unsigned char * text, unsigned char ** sexpr)
{
  int ret;

  if (text[0] == '\0')
    return -2;

  if (!(*sexpr))
    {
      unsigned char * tmp_str, * sexpr_sen;
      tmp_str = format_string (text);
      if (!tmp_str)
        return AEC_MEM;

      ret = check_text (tmp_str);
      if (ret == AEC_MEM)
        return AEC_MEM;

      if (ret != 0)
        return -2;

      sexpr_sen = convert_sexpr (tmp_str);
      if (!sexpr_sen)
        return AEC_MEM;
      free (tmp_str);
      *sexpr = sexpr_sen;
    }

  return 0;
}

/* Converts a sentence data object to sexpr form.
 *  input:
 *    sd - the sentence data object to convert.
 *  output:
 *    The same as sen_convert_sexpr.
 */
int
sd_convert_sexpr (sen_data * sd)
{
  return sen_convert_sexpr (sd->text, &(sd->sexpr));
}

/* Evaluates a sentence given its data.
 *  input:
 *   sd - The sentence data to evaluate.
 *   ret_val - The return value.
 *   pf_vars - The variables from the proof.
 *   lines - The lines from the proof.
 *  output:
 *   The status message to output.
 */
char *
sen_data_evaluate (sen_data * sd, int * ret_val, list_t * pf_vars, list_t * lines)
{
  if (sd->text[0] == '\0')
    {
      if (sd->line_num == 1)
        {
          *ret_val = VALUE_TYPE_BLANK;
          return CORRECT;
        }
      else
        {
          *ret_val = VALUE_TYPE_ERROR;
          return _("Only the first sentence can be blank.");
        }
    }

  unsigned char * text;
  text = format_string (sd->text);

  int ret;
  ret = check_text (text);
  if (ret == AEC_MEM)
    return NULL;

  *ret_val = VALUE_TYPE_ERROR;

  switch (ret)
    {
    case 0:
      break;
    case -2:
      return _("The sentence has mismatched parenthesis.");
    case -3:
      return _("The sentence has invalid connectives.");
    case -4:
      return _("The sentence has invalid quantifiers.");
    case -5:
      return _("The sentence has syntactical errors.");
    }

  *ret_val = VALUE_TYPE_BLANK;

  if (sd->premise || sd->subproof)
    {
      *ret_val = VALUE_TYPE_TRUE;
      return CORRECT;
    }

  if (sd->rule == -1 || sd->rule >= NUM_RULES)
    {
      *ret_val = VALUE_TYPE_RULE;
      return _("The sentence is missing a rule.");
    }

  *ret_val = VALUE_TYPE_ERROR;
  const char * rule = rules_list[sd->rule];

  vec_t * refs;

  refs = init_vec (sizeof (char *));
  if (!refs)
    return NULL;

  int i;

  for (i = 0; sd->refs[i] != REF_END; i++)
    {
      item_t * cur_ref;
      sen_data * ref_data;

      if (sd->refs[i] > lines->num_stuff)
        return NULL;

      cur_ref = ls_nth (lines, sd->refs[i] - 1);
      ref_data = cur_ref->value;

      unsigned char * tmp_ref_str = format_string (ref_data->text);

      ret = check_text (tmp_ref_str);
      if (ret == AEC_MEM)
        return NULL;
      free (tmp_ref_str);

      if (ret < 0)
        {
          destroy_str_vec (refs);
          *ret_val = VALUE_TYPE_REF;

          return _("One of the sentence's references has a text error.");
        }

      unsigned char * ref_text;
      ref_text = ref_data->sexpr;

      ret = vec_str_add_obj (refs, ref_text);
      if (ret == AEC_MEM)
        return NULL;

      if (ref_data->subproof)
        {
          ret = sen_data_can_select_as_ref (sd, ref_data);
          if (ret < 0)
            {
              sen_data * sen_0;
              item_t * ev_itr;

              ev_itr = ls_nth (lines, ref_data->line_num);
              while (ev_itr->next &&
                     SD(ev_itr->next->value)->depth >= ref_data->depth)
                {
                  if (!ev_itr->next)
                    break;
                  ev_itr = ev_itr->next;
                }

              sen_0 = ev_itr->value;
              tmp_ref_str = format_string (sen_0->text);
              if (!tmp_ref_str)
                return NULL;

              ret = check_text (tmp_ref_str);
              if (ret == AEC_MEM)
                return NULL;
              free (tmp_ref_str);

              if (ret < 0)
                {
                  *ret_val = VALUE_TYPE_REF;
                  destroy_str_vec (refs);
                  return _("One of the sentence's references has a text error.");
                }

              unsigned char * ref_text;
              ref_text = sen_0->sexpr;

              ret = vec_str_add_obj (refs, ref_text);
              if (ret == AEC_MEM)
                return NULL;
            }
          else if (sd->rule == RULE_SP)
            {
              return _("\'sp\' can only be used with a subproof as a reference.");
            }
        }
    }

  // Next, get the variables.

  vec_t * vars = init_vec (sizeof (variable));
  item_t * var_itr;
  proof_t * proof = NULL;

  for (var_itr = pf_vars->head; var_itr; var_itr = var_itr->next)
    {
      variable * var = var_itr->value;
      ret = vec_add_obj (vars, var);
      if (ret == AEC_MEM)
        return NULL;
    }

  char * fin_text;
  ret = sen_convert_sexpr (sd->text, &(sd->sexpr));
  if (ret == AEC_MEM)
    return NULL;

  fin_text = sd->sexpr;

  // Check for a file.

  if (sd->file)
    {
      // Obtain the extension.
      int f_len;
      f_len = strlen (sd->file);

      if (!strncmp (sd->file + f_len - 4, ".thy", 4))
        {
          proof = proof_init ();
          if (!proof)
            return NULL;

          ret = parse_thy (sd->file, proof);
          if (ret == AEC_MEM)
            return NULL;
        }
      else
        {
          struct connectives_list current_conns;
          current_conns = main_conns;

          proof = aio_open (sd->file);
          if (!proof)
            {
              *ret_val = VALUE_TYPE_ERROR;
              destroy_str_vec (refs);
              destroy_vec (vars);
              return _("Unable to open lemma file.");
            }

          main_conns = current_conns;
        }
    }

  char * proc_ret = process (fin_text, refs, rule, vars, proof);
  if (!proc_ret)
    return NULL;

  destroy_str_vec (refs);
  destroy_vec (vars);
  if (proof)
    proof_destroy (proof);

  if (!strcmp (proc_ret, CORRECT))
    *ret_val = VALUE_TYPE_TRUE;
  else
    *ret_val = VALUE_TYPE_FALSE;

  return proc_ret;
}

/* Determines whether a sentence can another as a reference.
 *  input:
 *    sen_line, ref_line - the line numbers of the two lines.
 *    sen_indices, ref_indices - the indices of the two lines.
 *    ref_prem - whether or not the reference is a premise.
 *  output:
 *    The line number of the line to be selected.
 *    The negation of the line number if the entire subproof is to be selected.
 *    0 if ref can not be selected by sen.
 */
int
sen_data_can_sel_as_ref (int sen_line, int * sen_indices,
                         int ref_line, int * ref_indices,
                         int ref_prem)
{
  if (ref_line >= sen_line)
    return 0;

  int entire;

  // Get indices of each.
  // This isn't necessary if the sentence is a premise, or if it has depth == zero.
  if (ref_prem || ref_indices[0] == -1)
    return ref_line;

  int common_line;
  int i;

  common_line = 0;
  for (i = 0; sen_indices[i] != -1 && ref_indices[i] != -1; i++)
    {
      if (sen_indices[i] != ref_indices[i])
        break;
    }

  // If the set of the reference sentence's indices is a subset of
  // the set of the focused sentence's indices, then the sentence can
  // be selected as a reference.

  if (ref_indices[i] == -1)
    {
      common_line = ref_line;
      entire = 0;
    }
  else
    {
      common_line = ref_indices[i];
      entire = 1;
    }

  if (ref_indices[i] != -1 && sen_indices[i] == -1)
    entire = 1;

  if (entire == 1)
    common_line *= -1;

  return common_line;
}

/* Determines whether a sentence can select another one - wrapper function.
 *  input:
 *    sen - The sentence selecting.
 *    ref - The sentence being selected.
 *  output:
 *    The line number of the line to be selected.
 *    The negation of the line number if the entire subproof is to be selected.
 *    0 if ref can not be selected by sen.
 */
int
sen_data_can_select_as_ref (sen_data * sen, sen_data * ref)
{
  if (ref->line_num >= sen->line_num)
    return 0;

  if (sen->premise || sen->subproof)
    return 0;

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

/* Converts sentence data to a LaTeX line.
 *  input:
 *   sd - the sentence data to convert.
 *  output:
 *   The converted text, or NULL on error.
 */
char *
convert_sd_latex (sen_data * sd)
{
  char * out_str, * text;
  int out_pos, i;

  // I feel like that isn't necessary either.

  text = sd->text;

  out_str = (char *) calloc (sd->depth * 6 + 1, sizeof (char));
  CHECK_ALLOC (out_str, NULL);

  out_pos = 0;

  for (i = 0; i < sd->depth; i++)
    out_pos += sprintf (out_str + out_pos, "\\pquad ");

  for (i = 0; text[i]; i++)
    {
      if (!strncmp (text + i, AND, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\land ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, OR, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\lor ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, NOT, NL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\neg ");
	  i += NL - 1;
	  continue;
	}

      if (!strncmp (text + i, CON, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 16);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\rightarrow ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, BIC, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 18);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\leftrightarrow ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, UNV, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 16);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\forall ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, EXL, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 16);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\exists ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, TAU, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\top ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, CTR, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\bot ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, ELM, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\in ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, NIL, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, " \\obslash ");
	  i += CL - 1;
	  continue;
	}

      out_str = (char *) realloc (out_str, out_pos + 2);
      CHECK_ALLOC (out_str, NULL);
      out_str[out_pos++] = text[i];

    }

  out_str = (char *) realloc (out_str, out_pos + 1);
  CHECK_ALLOC (out_str, NULL);
  out_str[out_pos] = '\0';

  return out_str;
}

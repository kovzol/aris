/* Functions for handling sentence data.

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
 *    indices - the indices of the sentence.
 *    sexpr - the sexpr text of this sentence.
 *  output:
 *    the newly initialized sentence data, or NULL on error.
 */
sen_data *
sen_data_init (int line_num, int rule, unsigned char * text,
	       int * refs, int premise, unsigned char * file,
	       int subproof, int depth, unsigned char * sexpr)
{
  sen_data * sd;
  sd = (sen_data *) calloc (1, sizeof (sen_data));
  CHECK_ALLOC (sd, NULL);

  sd->vars = NULL;

  sd->line_num = line_num;
  sd->rule = rule;
  sd->text = sd->file = sd->sexpr = NULL;

  if (text)
    {
      int text_len;
      text_len = strlen (text);
      sd->text = (unsigned char *) calloc (text_len + 1,
					   sizeof (char));
      CHECK_ALLOC (sd->text, NULL);
      strcpy (sd->text, text);
    }

  if (file)
    {
      int file_len;
      file_len = strlen (file);

      sd->file = (unsigned char *) calloc (file_len + 1,
					   sizeof (char));
      CHECK_ALLOC (sd->file, NULL);
      strcpy (sd->file, file);
    }

  if (sexpr)
    {
      sd->sexpr = (unsigned char *) calloc (strlen (sexpr) + 1, sizeof (char));
      CHECK_ALLOC (sd->sexpr, NULL);
      strcpy (sd->sexpr, sexpr);
    }

  sd->refs = refs;

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

  if (sd->vars)
    {
      int i = 0;
      unsigned char * var = sd->vars[i];
      for (i = 0; sd->vars[i] != NULL; i++)
	free (sd->vars[i]);

      free (sd->vars);
    }

  if (sd->indices)
    free (sd->indices);
  sd->indices = NULL;

  free (sd);
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

  int ret;
  ret = check_text (sd->text);
  if (ret == -1)
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

  if (sd->rule == -1)
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

  for (i = 0; sd->refs[i] != -1; i++)
    {
      item_t * cur_ref;
      sen_data * ref_data;

      if (sd->refs[i] > lines->num_stuff)
	return NULL;

      cur_ref = ls_nth (lines, sd->refs[i] - 1);
      ref_data = cur_ref->value;

      ret = check_text (ref_data->text);
      if (ret == -1)
	return NULL;

      if (ret < 0)
	{
	  destroy_str_vec (refs);
	  *ret_val = VALUE_TYPE_REF;

	  return _("One of the sentence's references has a text error.");
	}

      unsigned char * ref_text;
      ref_text = ref_data->sexpr;

      ret = vec_str_add_obj (refs, ref_text);
      if (ret == -1)
	return NULL;

      if (ref_data->subproof)
	{
	  ret = sen_data_can_select_as_ref (sd, ref_data);
	  if (ret < 0)
	    {
	      sen_data * sen_0;
	      item_t * ev_itr;

	      ev_itr = ls_nth (lines, ref_data->line_num);
	      while (((sen_data *) ev_itr->next->value)->depth >= ref_data->depth)
		ev_itr = ev_itr->next;

	      sen_0 = ev_itr->value;
	      ret = check_text (sen_0->text);
	      if (ret == -1)
		return NULL;

	      if (ret < 0)
		{
		  *ret_val = VALUE_TYPE_REF;
		  destroy_str_vec (refs);
		  return _("One of the sentence's references has a text error.");
		}

	      unsigned char * ref_text;
	      ref_text = sen_0->sexpr;

	      ret = vec_str_add_obj (refs, ref_text);
	      if (ret == -1)
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
      if (ret == -1)
	return NULL;
    }

  char * eval_text, * fin_text;
  if (!sd->sexpr)
    {
      eval_text = die_spaces_die (sd->text);
      if (!eval_text)
	return NULL;

      fin_text = convert_sexpr (eval_text);
      if (!fin_text)
	return NULL;
      free (eval_text);
      sd->sexpr = fin_text;
    }
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
	  if (ret == -1)
	    return NULL;
	}
      else
	{
	  struct connectives_list current_conns;
	  current_conns = main_conns;
	  //main_conns = gui_conns;

	  proof = aio_open (sd->file);
	  if (!proof)
	    return NULL;
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

/* Determines whether a sentence can select another one.
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

  int entire;

  // Get indices of each.
  // This isn't necessary if the sentence is a premise, or if it has depth == zero.
  if (ref->premise || ref->depth == 0)
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

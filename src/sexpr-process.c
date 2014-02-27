/* Processing functions used by all sexpr processing functions.

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

#include "process.h"
#include "sexpr-process.h"
#include "vec.h"
#include "var.h"
#include "list.h"
#include <stdarg.h>

/* Gets a sentence part from a sexpr.
 *  input:
 *   in_str - the input string.
 *   init_pos - the initial position within in_str.
 *   out_str - the string that receives the new sentence part, if any.
 *  output:
 *   the position in in_str of the end of the part, or -1 on memory error.
 */
int
sexpr_get_part (unsigned char * in_str, unsigned int init_pos, unsigned char ** out_str)
{
  int fin_pos = init_pos;
  int tmp_pos;

  *out_str = NULL;

  switch (in_str[init_pos])
    {
    case '(':
      tmp_pos = parse_parens (in_str, init_pos, out_str);
      if (tmp_pos == ERROR_CODE_MEMORY)
	return ERROR_CODE_MEMORY;

      fin_pos = tmp_pos + 1;
      break;

    case ' ':
      fin_pos++;
      break;

    default:
      tmp_pos = init_pos;
      while (in_str[tmp_pos] != ' '
	     && in_str[tmp_pos] != ')'
	     && in_str[tmp_pos] != '\0')
	tmp_pos++;

      (*out_str) = (unsigned char *) calloc (tmp_pos - init_pos + 1, sizeof (char));
      CHECK_ALLOC ((*out_str), -1);
      strncpy ((*out_str), in_str + init_pos, tmp_pos - init_pos);
      (*out_str)[tmp_pos - init_pos] = '\0';

      fin_pos = tmp_pos;
      break;
    }

  return fin_pos;
}

unsigned char *
sexpr_car (unsigned char * in_str)
{
  unsigned char * out_str;
  int pos;
  pos = sexpr_get_part (in_str, 1, &out_str);
  if (pos == -1)
    return NULL;

  return out_str;
}

unsigned char *
sexpr_cdr (unsigned char * in_str)
{
  unsigned char * out_str, * car;
  int pos, len;
  pos = sexpr_get_part (in_str, 1, &car);
  if (pos == -1)
    return NULL;

  len = strlen (in_str);
  out_str = (unsigned char *) calloc (len - pos + 1, sizeof (char));
  CHECK_ALLOC (out_str, NULL);

  sprintf (out_str, "(%s", in_str + pos + 1);

  return out_str;
}

int
sexpr_car_cdr (unsigned char * in_str,
	       unsigned char ** car,
	       vec_t * cdr)
{
  int init_pos = (in_str[0] == '(') ? 1 : 0;
  int pos, ret_chk;
  pos = sexpr_get_part (in_str, init_pos, car);
  if (pos == -1)
    return -1;

  pos++;

  while (in_str[pos] != '\0')
    {
      int tmp_pos;
      unsigned char * tmp_str = NULL;

      tmp_pos = sexpr_get_part (in_str, pos, &tmp_str);
      if (tmp_pos == -1)
	return -1;

      if (tmp_str)
	{
	  ret_chk = vec_str_add_obj (cdr, tmp_str);
	  if (ret_chk == -1)
	    return -1;
	}
      pos = tmp_pos + 1;
    }

  return cdr->num_stuff;
}

int
sexpr_str_car_cdr (unsigned char * in_str,
		   unsigned char ** car,
		   unsigned char ** cdr)
{
  int tmp_pos;

  *car = *cdr = NULL;

  tmp_pos = sexpr_get_part (in_str, 1, car);
  if (tmp_pos == -1)
    return -1;

  tmp_pos += 1;

  tmp_pos = parse_parens (in_str, tmp_pos, cdr);
  if (tmp_pos == ERROR_CODE_MEMORY)
    return ERROR_CODE_MEMORY;

  if (tmp_pos < 0)
    return -2;

  return 0;
}

/* Places sentences based on their lengths.
 *  input:
 *    in0 - the first sentence to place.
 *    in1 - the second sentence to place.
 *    sh_sen - a pointer to the shorter sentence.
 *    ln_sen - a pointer to the longer sentence.
 *  output:
 *    none
 */
void
sen_put_len (unsigned char * in0,
	     unsigned char * in1,
	     unsigned char ** sh_sen,
	     unsigned char ** ln_sen)
{
  int len0, len1;
  len0 = strlen (in0);
  len1 = strlen (in1);

  if (len0 > len1)
    {
      *sh_sen = in1;
      *ln_sen = in0;
    }
  else
    {
      *sh_sen = in0;
      *ln_sen = in1;
    }
}

unsigned char *
construct_other (unsigned char * main_str,
		 int init_pos,
		 int fin_pos,
		 int alloc_size,
		 char * template,
		 ...)
{
  unsigned char * oth_sen;
  int oth_pos = init_pos;
  va_list args;

  va_start (args, template);

  oth_sen = (unsigned char *) calloc (alloc_size + 1, sizeof (char));
  CHECK_ALLOC (oth_sen, NULL);
  strncpy (oth_sen, main_str, oth_pos);

  oth_pos += vsprintf (oth_sen + oth_pos, template, args);

  strcpy (oth_sen + oth_pos, main_str + fin_pos);

  va_end (args);

  return oth_sen;
}

/* Checks for a negation on a sexpr string.
 *  input:
 *    in_str - the sexpr text on which to check for a negation.
 *  output:
 *    1 if there is a negation, 0 otherwise.
 */
int
sexpr_not_check (unsigned char * in_str)
{
  int tmp_pos;

  tmp_pos = parse_parens (in_str, 0, NULL);

  if (tmp_pos < 0 || in_str[tmp_pos + 1] != '\0'
      || strncmp (in_str + 1, S_NOT, S_NL))
    return 0;

  return 1;
}

/* Adds a negation to a sexpr string.
 *  input:
 *    in_str - the sexpr text to which to add a negation.
 *  output:
 *    The modified string, or NULL on memory error.
 */
unsigned char *
sexpr_add_not (unsigned char * in_str)
{
  unsigned char * not_in_str;

  not_in_str = (unsigned char *) calloc (strlen (in_str) + S_NL + 3, sizeof (char));
  CHECK_ALLOC (not_in_str, NULL);
  sprintf (not_in_str, "(%s %s)\0", S_NOT, in_str);

  return not_in_str;
}

/* Eliminates a negation from a sexpr string.
 *  input:
 *    in_str - the sexpr text from which to remove a negation.
 *  output:
 *    The modified string, or NULL on memory error.
 */
unsigned char *
sexpr_elim_not (unsigned char * in_str)
{
  unsigned char * out_str;
  int in_len;

  in_len = strlen (in_str);
  out_str = (unsigned char *) calloc (in_len - S_NL - 2, sizeof (char));
  CHECK_ALLOC (out_str, NULL);
  strncpy (out_str, in_str + 2 + S_NL, in_len - (3 + S_NL));
  out_str[in_len - (3 + S_NL)] = '\0';

  return out_str;
}

/* Gets the generalities from a sexpr string.
 *  input:
 *    in_str - the sexpr text from which to obtain the generalities.
 *    conn - the connective to check for, or an empty string to check for any.
 *    vec - the string vector to hold the generalities.
 *  output:
 *    The size of vec, or -1 on memory error.
 */
int
sexpr_get_generalities (unsigned char * in_str, unsigned char * conn, vec_t * vec)
{
  int ret_chk;
  unsigned char * tmp_conn;

  ret_chk = sexpr_car_cdr (in_str, &tmp_conn, vec);
  if (ret_chk == -1)
    return -1;

  if (ret_chk == 0)
    {
      vec_str_add_obj (vec, tmp_conn);
      return 1;
    }

  if (tmp_conn[0] == '(')
    {
      vec_pop_obj (vec);
      vec_str_add_obj (vec, in_str);
      return 1;
    }

  if (conn[0] == '\0')
    {
      strncpy (conn, tmp_conn, S_CL);
      conn[S_CL] = '\0';
    }
  else
    {
      if (strncmp (conn, tmp_conn, S_CL))
	{
	  free (tmp_conn);
	  return 1;
	}
    }

  free (tmp_conn);
  return vec->num_stuff;
}

/* Gets the top connectives from a sexpr string.
 *  input:
 *    in_str - the sexpr text from which to get the top connective.
 *    conn - the connective to check for, or an empty string to check for any.
 *    lsen - a pointer to a string to hold the left sentence.
 *    rsen - a pointer to a string to hold the right sentence.
 *  output:
 *    0 on success, -1 on memory error, -2 if there aren't two generalities.
 */
int
sexpr_find_top_connective (unsigned char * in_str, unsigned char * conn,
			   unsigned char ** lsen, unsigned char ** rsen)
{
  int gg;
  vec_t * vec;

  *lsen = *rsen = NULL;

  vec = init_vec (sizeof (char *));
  if (!vec)
    return -1;

  gg = sexpr_get_generalities (in_str, conn, vec);
  if (gg == -1)
    return -1;

  if (gg == 1 || gg > 2)
    {
      destroy_str_vec (vec);
      return -2;
    }

  unsigned char * gen_0, * gen_1;
  int g0_len, g1_len;

  gen_0 = vec_str_nth (vec, 0);
  gen_1 = vec_str_nth (vec, 1);

  g0_len = strlen (gen_0);
  g1_len = strlen (gen_1);

  *lsen = (unsigned char *) calloc (g0_len + 1, sizeof (char));
  CHECK_ALLOC (*lsen, -1);
  strcpy (*lsen, gen_0);

  *rsen = (unsigned char *) calloc (g1_len + 1, sizeof (char));
  CHECK_ALLOC (*rsen, -1);
  strcpy (*rsen, gen_1);

  destroy_str_vec (vec);

  return 0;
}

/* Finds an unmatched opening parenthesis.
 *  input:
 *    in_str - the string on which to check for an unmatched opening parenthesis.
 *    in_pos - the initial position.
 *  output:
 *    The position of the unmatched opening parenthesis, or -1 on memory error.
 */
int
find_unmatched_o_paren (unsigned char * in_str, int in_pos)
{
  int pos = in_pos;

  while (pos >= 0)
    {
      if (in_str[pos] == ')')
	{
	  unsigned char * tmp_str;
	  pos = reverse_parse_parens (in_str, pos, &tmp_str);
	  if (pos == ERROR_CODE_MEMORY)
	    return ERROR_CODE_MEMORY;
	  free (tmp_str);
	}
      else if (in_str[pos] == '(')
	break;

      pos--;
    }

  return pos;
}

/* Find unmatched sentence parts from an opening parenthesis.
 *  input:
 *    sen_a, sen_b - the sentences to check.
 *    ai, bi - integer pointers that receive the positions.
 *      These both must start on an opening parenthesis.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
sexpr_find_unmatched (unsigned char * sen_a, unsigned char * sen_b,
		      int * ai, int * bi)
{
  int a, b, tmp_a, tmp_b;
  unsigned char * a_str, * b_str;

  a = *ai;  b = *bi;

  tmp_a = parse_parens (sen_a, a, &a_str);
  if (tmp_a == ERROR_CODE_MEMORY)
    return -1;

  tmp_b = parse_parens (sen_b, b, &b_str);
  if (tmp_b == ERROR_CODE_MEMORY)
    return -1;

  while (!strcmp (a_str, b_str))
    {
      free (a_str);
      free (b_str);

      a = find_unmatched_o_paren (sen_a, a - 1);
      b = find_unmatched_o_paren (sen_b, b - 1);

      if (a < 0 || b < 0)
	break;

      tmp_a = parse_parens (sen_a, a, &a_str);
      if (tmp_a == ERROR_CODE_MEMORY)
	return -1;

      tmp_b = parse_parens (sen_b, b, &b_str);
      if (tmp_b == ERROR_CODE_MEMORY)
	return -1;
    }

  *ai = a;
  *bi = b;

  return 0;
}

/* Get the predicate arguments from a sexpr string.
 *  input:
 *    in_str - the sexpr text from which to get the predicate arguments.
 *    pred - a string pointer that receives the predicate symbol.
 *    vec - a string vector that receives the arguments.
 *  output:
 *    The number of arguments on success,
 *    0 on error,
 *    -1 on memory error.
 */
int
sexpr_get_pred_args (unsigned char * in_str, unsigned char ** pred, vec_t * vec)
{
  int ret_chk;
  unsigned char * tmp_pred;

  ret_chk = sexpr_car_cdr (in_str, &tmp_pred, vec);
  if (ret_chk == -1)
    return -1;

  if (tmp_pred[0] == '(' || ret_chk == 0)
    return 0;

  if (pred)
    {
      *pred = strdup (tmp_pred);
      if (!(*pred))
	return -1;
      free (tmp_pred);
    }

  return vec->num_stuff;
}

/* Eliminates a quantifier from a sexpr string.
 *  input:
 *    in_str - the quantifier from which to eliminate the quantifier.
 *    quant - receives the quantifier.
 *    var - a string pointer that receives the variable.
 *  output:
 *    the scope of the quantifier, or NULL on error.
 */
unsigned char *
sexpr_elim_quant (unsigned char * in_str, unsigned char * quant,
		  unsigned char ** var)
{

  int tmp_pos, ret_chk;
  unsigned char * tmp_str, * car;

  tmp_pos = sexpr_str_car_cdr (in_str, &car, &tmp_str);
  if (tmp_pos == -1)
    return NULL;

  if (tmp_pos == -2)
    {
      if (car) free (car);
      if (tmp_str) free (tmp_str);
      return "\0";
    }

  *var = NULL;

  int alloc_size = strlen (car) - 3 - S_CL;
  *var = (unsigned char *) calloc (alloc_size + 1, sizeof (char));
  CHECK_ALLOC (var, NULL);

  ret_chk = sscanf (car, "(%s %[^)])", quant, *var);
  if (ret_chk != 2
      || (strcmp (quant, S_UNV) && strcmp (quant, S_EXL)))
    return "\0";

  free (car);

  return tmp_str;
}

/* Get the offsets from the start of a quantifier's scope of its variable.
 *  input:
 *    in_str - the sexpr string from which to obtain the offsets.
 *    var - a vector of integers that receives the offsets.
 *  output:
 *    The size of vars on success, -1 on memory error, -2 on general error.
 */
int
sexpr_get_quant_vars (unsigned char * in_str, vec_t * vars)
{
  unsigned char * scope, quant[S_CL + 1], * var;

  scope = sexpr_elim_quant (in_str, quant, &var);
  if (!scope)
    return -1;

  if (scope[0] == '\0')
    return -2;

  int i, v_len, ret_chk;

  v_len = strlen (var);

  for (i = 0; scope[i] != '\0'; i++)
    {
      if (strncmp (scope + i, var, v_len))
	continue;

      if (scope[i - 1] == '(')
	continue;

      if (scope[i + 1] != ')' && scope[i + 1] != ' ')
	continue;

      ret_chk = vec_add_obj (vars, &i);
      if (ret_chk < 0)
	return -1;
    }

  return vars->num_stuff;
}

/* Replaces a variable in a sexpr string.
 *  input:
 *    in_str - the sexpr text to replace the variables of.
 *    new_var - the new variable.
 *    old_var - the old variable.
 *    off_var - the variable offsets in in_str.
 *    out_str - the modified string.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
sexpr_replace_var (unsigned char * in_str, unsigned char * new_var,
		   unsigned char * old_var, vec_t * off_var,
		   unsigned char ** out_str)
{
  int out_pos, i, * cur_off, old_len;

  old_len = strlen (old_var);
  *out_str = (unsigned char *) calloc (strlen (in_str) + strlen (new_var) * off_var->num_stuff, sizeof (char));
  CHECK_ALLOC (*out_str, -1);

  i = 0;
  cur_off = vec_nth (off_var, i);
  strncpy (*out_str, in_str, *cur_off);
  out_pos = *cur_off;
  out_pos += sprintf (*out_str + out_pos, "%s", new_var);

  for (i = 1; i < off_var->num_stuff; i++)
    {
      int * last_off;
      cur_off = vec_nth (off_var, i);
      last_off = vec_nth (off_var, i - 1);
      strncpy (*out_str + out_pos, in_str + *last_off + old_len,
	       *cur_off - *last_off - old_len);
      out_pos += *cur_off - *last_off - old_len;
      out_pos += sprintf (*out_str + out_pos, "%s", new_var);
    }

  strcpy (*out_str + out_pos, in_str + *cur_off + old_len);

  return 0;
}

/* Processes the standard quantifier inference rules.
 *  input:
 *    quant_sen - the quantifier sentence.
 *    elim_sen - the other sentence.
 *    quant - the quantifier.
 *    cons - the constraints - 0 normally, 1 for ug, 2 for ei.
 *    cur_vars - the current variables, or NULL if they're not neccessary.
 *  output:
 *    0 - success
 *    1 - the strings are the same
 *    -1 - memory error
 *    -2 - general error
 *    -3 - variable error
 */
int
sexpr_quant_infer (unsigned char * quant_sen, unsigned char * elim_sen,
		   unsigned char * quant, int cons, vec_t * cur_vars)
{
  if (!strcmp (quant_sen, elim_sen))
    return 1;

  if (quant_sen[0] != '(' || quant_sen[1] != '(')
    return -2;

  unsigned char * var, * elm_sen, qs_quant[S_CL + 1];

  elm_sen = sexpr_elim_quant (quant_sen, qs_quant, &var);
  if (!elm_sen)
    return -1;

  if (elm_sen[0] == '\0' || strcmp (qs_quant, quant))
    return -2;

  int q_pos, e_pos, cmp, tmp_0;

  cmp = 0;
  tmp_0 = -2;

  while (!cmp)
    {
      int tmp_1;
      unsigned char * str_0, * str_1;

      q_pos = tmp_0 + 2;

      if (elm_sen[q_pos + 1] != '(')
	break;

      tmp_0 = parse_parens (elm_sen, q_pos + 1, &str_0);
      if (tmp_0 == ERROR_CODE_MEMORY)
	return ERROR_CODE_MEMORY;

      if (elim_sen[1] != '(')
	{
	  free (str_0);
	  q_pos = tmp_0 + 2;
	  continue;
	}

      tmp_1 = parse_parens (elim_sen, 1, &str_1);
      if (tmp_1 == ERROR_CODE_MEMORY)
	return ERROR_CODE_MEMORY;

      cmp = !strcmp (str_0, str_1);
      free (str_0);
      free (str_1);
    }

  // Determine the offset, and get the quantifier's variable positions.

  int offset, ret_chk;
  vec_t * var_offs;

  offset = q_pos;
  var_offs = init_vec (sizeof (int));
  if (!var_offs)
    return -1;

  ret_chk = sexpr_get_quant_vars (quant_sen, var_offs);
  if (ret_chk == -1)
    return -1;

  // Use the offset to determine the position of the first variable in elim_sen.
  // Get the variable from elim_sen.

  int * off_0;
  unsigned char * oth_sen, * new_var = NULL;

  off_0 = vec_nth (var_offs, 0);
  if (!off_0)
    {
      destroy_vec (var_offs);
      free (elm_sen);
      return -2;
    }
  q_pos = e_pos = *off_0 - offset;

  if (elim_sen[e_pos] == '(')
    {
      q_pos = parse_parens (elim_sen, e_pos, &new_var);
      if (q_pos == ERROR_CODE_MEMORY)
	return ERROR_CODE_MEMORY;
    }
  else
    {
      while (elim_sen[q_pos] != ' ' && elim_sen[q_pos] != ')')
	q_pos++;

      new_var = (unsigned char *) calloc (q_pos - e_pos + 1, sizeof (char));
      CHECK_ALLOC (new_var, -1);

      strncpy (new_var, elim_sen + e_pos, q_pos - e_pos);
      new_var[q_pos - e_pos] = '\0';
    }

  // If there are constraints, then check them.

  if (cons)
    {
      int i;

      for (i = 0; new_var[i] != '\0'; i++)
	if (new_var[i] == ' ')
	  break;

      if (new_var[i] != '\0')
	{
	  free (new_var);
	  destroy_vec (var_offs);
	  free (elm_sen);
	  return -3;
	}

      for (i = 0; i < cur_vars->num_stuff; i++)
	{
	  variable * cur_var;
	  cur_var = vec_nth (cur_vars, i);
	  if (!strcmp (cur_var->text, new_var))
	    break;
	}

      if (i != cur_vars->num_stuff)
	{
	  variable * cur_var;
	  cur_var = vec_nth (cur_vars, i);

	  if (cons == 2 || (cons == 1 && !cur_var->arbitrary))
	    {
	      free (new_var);
	      destroy_vec (var_offs);
	      free (elm_sen);
	      return -3;
	    }
	}
    }

  ret_chk = sexpr_replace_var (elm_sen, new_var, var, var_offs, &oth_sen);
  if (ret_chk == -1)
    return -1;

  free (var);
  free (elm_sen);
  destroy_vec (var_offs);
  free (new_var);

  if (cons != 2)
    {
      ret_chk = sexpr_quant_infer (oth_sen, elim_sen, quant, cons, cur_vars);
      if (ret_chk == -1)
	return -1;

      ret_chk = (ret_chk == 1) ? 0 : ret_chk;
    }
  else
    {
      ret_chk = strcmp (oth_sen, elim_sen);
    }

  free (oth_sen);
  if (ret_chk)
    return -2;

  return 0;
}

/* Determines the positions in a string of a variable.
 *  input:
 *    in_str - the sexpr text of which to determine the positions.
 *    var - the variable.
 *    offsets - an integer vector that stores the offsets.
 *  output:
 *    The size of offsets on success, -1 on memory error.
 */
int
sexpr_find_vars (unsigned char * in_str, unsigned char * var, vec_t * offsets)
{
  int i, var_len, ret_chk;

  var_len = strlen (var);

  for (i = 0; in_str[i] != '\0'; i++)
    {
      if (strncmp (in_str + i, var, var_len))
	continue;

      if (in_str[i - 1] == '(')
	continue;

      if (in_str[i + var_len] != ')' && in_str[i + var_len] != ' ')
	continue;

      ret_chk = vec_add_obj (offsets, &i);
      if (ret_chk < 0)
	return -1;
    }

  return offsets->num_stuff;
}

/* Collects variables from a sexpr string.
 *  input:
 *    in_str - the sexpr text from which to collect variables.
 *    vars - a string vector that holds the variables.
 *    quant - whether or not quantifier variables are being looked for.
 *  output:
 *    The size of vars on success, -1 on memory error.
 */
int
sexpr_parse_vars (unsigned char * in_str, vec_t * vars, int quant)
{
  // There are no variables in this string.
  if (in_str[0] != '(')
    return 0;

  int i, j;
  for (i = 0; in_str[i] != '\0'; i++)
    {
      if (!islower (in_str[i]))
	continue;

      if (in_str[i - 1] == '(' || in_str[i - 1] != ' ')
	continue;

      if (i > (S_CL + 1) &&
	  (!strncmp (in_str + i - (1 + S_CL), S_UNV, S_CL)
	   || !strncmp (in_str + i - (1 + S_CL), S_EXL, S_CL)))
	{
	  if (!quant)
	    continue;
	}
      else if (quant)
	{
	  continue;
	}

      int pos = i;
      unsigned char * new_var;

      while (in_str[pos] != ' ' && in_str[pos] != ')')
	pos++;

      new_var = (unsigned char *) calloc (pos - i + 1, sizeof (char));
      CHECK_ALLOC (new_var, -1);
      strncpy (new_var, in_str + i, pos - i);
      new_var[pos - i] = '\0';
      i = pos;

      for (j = 0; j < vars->num_stuff; j++)
	if (!strcmp (new_var, vec_str_nth (vars, j)))
	  break;

      if (j == vars->num_stuff)
	{
	  pos = vec_str_add_obj (vars, new_var);
	  if (pos < 0)
	    return -1;
	}

      free (new_var);
    }

  return vars->num_stuff;
}

/* Collect variables from a sentence to a list.
 *  input:
 *    vars - the list of variables from a proof.
 *    text - the text of the sentence.
 *    arb - whether or not the variables are arbitrary.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
sexpr_collect_vars_to_proof (list_t * vars, unsigned char * text, int arb)
{
  int ret, i, is_arbitrary;
  vec_t * sen_vars;

  sen_vars = init_vec (sizeof (char *));
  if (!sen_vars)
    return -1;

  ret = sexpr_parse_vars (text, sen_vars, 0);
  if (ret == -1)
    return -1;

  is_arbitrary = arb;

  if (sen_vars->num_stuff == 0)
    {
      destroy_str_vec (sen_vars);
      return 0;
    }

  for (i = 0; i < sen_vars->num_stuff; i++)
    {
      unsigned char * cur_var;
      item_t * ap_var_itr;

      cur_var = vec_str_nth (sen_vars, i);
      ap_var_itr = vars->head;

      for (; ap_var_itr; ap_var_itr = ap_var_itr->next)
	{
	  variable * var;

	  var = ap_var_itr->value;
	  if (!strcmp (var->text, cur_var))
	    break;
	}

      if (!ap_var_itr)
	{
	  variable * var;
	  item_t * itm;

	  var = variable_init (cur_var, is_arbitrary);
	  if (!var)
	    return -1;

	  itm = ls_push_obj (vars, var);
	  if (!itm)
	    return -1;
	}
    }

  destroy_str_vec (sen_vars);
  return 0;
}

/* Collect the object ids from a sexpr sentence.
 *  input:
 *    sen - the sentence text.
 *    ids - a ponter to an array that receives the ids.
 *    sen_ids - ids for sentence parts from previous sentences.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
sexpr_get_ids (unsigned char * sen, int ** ids, vec_t * sen_ids)
{
  int i, j;
  int sen_start_id;
  int sen_len;

  sen_len = strlen (sen);

  sen_start_id = SEN_ID_START;

  if (sen_ids)
    {
      if (sen_ids->num_stuff != 0)
	{
	  sen_id * sid;
	  sid = vec_nth (sen_ids, sen_ids->num_stuff - 1);
	  sen_start_id = sid->id + 1;
	}
    }

  *ids = (int *) calloc (sen_len, sizeof (int));
  CHECK_ALLOC (*ids, -1);

  j = 0;
  for (i = 0; i < sen_len; i++)
    {
      if (sen[i] == '(')
	{
	  (*ids)[j++] = SEN_ID_OPAREN;
	  continue;
	}

      if (sen[i] == ')')
	{
	  (*ids)[j++] = SEN_ID_CPAREN;
	  continue;
	}

      if (sen[i] == ' ')
	{
	  (*ids)[j++] = SEN_ID_SPACE;
	  continue;
	}

      if (!strncmp (sen + i, S_NOT, S_NL))
	{
	  (*ids)[j++] = SEN_ID_NOT;
	  i += (S_NL - 1);
	  continue;
	}

      if (!strncmp (sen + i, S_AND, S_CL))
	{
	  (*ids)[j++] = SEN_ID_AND;
	  i += (S_CL - 1);
	  continue;
	}

      if (!strncmp (sen + i, S_OR, S_CL))
	{
	  (*ids)[j++] = SEN_ID_OR;
	  i += (S_CL - 1);
	  continue;
	}

      if (!strncmp (sen + i, S_CON, S_CL))
	{
	  (*ids)[j++] = SEN_ID_CON;
	  i += (S_CL - 1);
	  continue;
	}

      if (!strncmp (sen + i, S_BIC, S_CL))
	{
	  (*ids)[j++] = SEN_ID_BIC;
	  i += (S_CL - 1);
	  continue;
	}

      if (!strncmp (sen + i, S_UNV, S_CL))
	{
	  (*ids)[j++] = SEN_ID_UNV;
	  i += (S_CL - 1);
	  continue;
	}

      if (!strncmp (sen + i, S_EXL, S_CL))
	{
	  (*ids)[j++] = SEN_ID_EXL;
	  i += (S_CL - 1);
	  continue;
	}

      if (sen[i] == '=')
	{
	  (*ids)[j++] = SEN_ID_EQ;
	  continue;
	}

      if (sen[i] == '<')
	{
	  (*ids)[j++] = SEN_ID_LT;
	  continue;
	}

      if (!strncmp (sen + i, S_ELM, S_CL))
	{
	  (*ids)[j++] = SEN_ID_ELM;
	  i += (S_CL - 1);
	  continue;
	}

      if (!strncmp (sen + i, S_NIL, S_CL))
	{
	  (*ids)[j++] = SEN_ID_NIL;
	  i += (S_CL - 1);
	  continue;
	}

      if (isalnum (sen[i]))
	{
	  int k;
	  int start, end;
	  int new_id;

	  start = end = i;
	  end++;
	  while (ISLEGIT (sen[end]))
	    end++;

	  if (sen_ids)
	    {
	      for (k = 0; k < sen_ids->num_stuff; k++)
		{
		  sen_id * cur_sen;
		  cur_sen = vec_nth (sen_ids, k);

		  if (!strncmp (cur_sen->sen, sen + start, end - start))
		    {
		      if (cur_sen->sen[end - start] == '\0')
			{
			  new_id = cur_sen->id;
			  break;
			}
		    }
		}
	    }

	  if (!sen_ids || k == sen_ids->num_stuff)
	    {
	      sen_id new_sen_id;

	      new_sen_id.sen = (unsigned char *) calloc (end - start + 1,
							 sizeof (char));
	      CHECK_ALLOC (new_sen_id.sen, -1);
	      strncpy (new_sen_id.sen, sen + start, end - start);
	      new_sen_id.sen[end - start] = '\0';

	      new_sen_id.id = sen_start_id++;
	      new_id = new_sen_id.id;

	      if (sen_ids)
		vec_add_obj (sen_ids, &new_sen_id);
	    }

	  (*ids)[j++] = new_id;
	  i = end - 1;
	  continue;
	}
    }

  (*ids)[j] = SEN_ID_END;

  return 0;
}

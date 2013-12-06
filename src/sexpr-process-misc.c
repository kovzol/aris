/* Functions for processing the miscellaneous rules.

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

#include "sexpr-process.h"
#include "proof.h"
#include "vec.h"
#include "list.h"
#include "sen-data.h"
#include "var.h"

int
sexpr_id_chk (unsigned char * cur_ref, int * pf_id, vec_t * cur_sen_ids)
{
  int i, k, l, m;
  int valid, ret_chk;
  unsigned char * new_sen;
  int cur_len = cur_sen_ids->num_stuff;

  i = k = l = m = 0;
  valid = 1;

  while (pf_id[k] != SEN_ID_END)
    {
      switch (pf_id[k])
	{
	case SEN_ID_OPAREN:
	  if (cur_ref[l] != '(')
	    valid = 0;
	  l++;
	  break;

	case SEN_ID_CPAREN:
	  if (cur_ref[l] != ')')
	    valid = 0;
	  l++;
	  break;

	case SEN_ID_SPACE:
	  if (cur_ref[l] != ' ')
	    valid = 0;
	  l++;
	  break;

	case SEN_ID_NOT:
	  if (strncmp (cur_ref + l, S_NOT, S_NL))
	    valid = 0;
	  l += S_NL;
	  break;

	case SEN_ID_AND:
	  if (strncmp (cur_ref + l, S_AND, S_CL))
	    valid = 0;
	  l += S_CL;
	  break;

	case SEN_ID_OR:
	  if (strncmp (cur_ref + l, S_OR, S_CL))
	    valid = 0;
	  l += S_CL;
	  break;

	case SEN_ID_CON:
	  if (strncmp (cur_ref + l, S_CON, S_CL))
	    valid = 0;
	  l += S_CL;
	  break;

	case SEN_ID_BIC:
	  if (strncmp (cur_ref + l, S_BIC, S_CL))
	    valid = 0;
	  l += S_CL;
	  break;

	case SEN_ID_UNV:
	  if (strncmp (cur_ref + l, S_UNV, S_CL))
	    valid = 0;
	  l += S_CL;
	  break;

	case SEN_ID_EXL:
	  if (strncmp (cur_ref + l, S_EXL, S_CL))
	    valid = 0;
	  l += S_CL;
	  break;

	case SEN_ID_EQ:
	  if (cur_ref[l] != '=')
	    valid = 0;
	  l++;
	  break;

	case SEN_ID_LT:
	  if (cur_ref[l] != '<')
	    valid = 0;
	  l++;
	  break;

	case SEN_ID_ELM:
	  if (strncmp (cur_ref + l, S_ELM, S_CL))
	    valid = 0;
	  l += S_CL;
	  break;

	case SEN_ID_NIL:
	  if (strncmp (cur_ref + l, S_NIL, S_CL))
	    valid = 0;
	  l += S_CL;
	  break;

	default:
	  ret_chk = sexpr_get_part (cur_ref, l, &new_sen);
	  if (ret_chk == -1)
	    return -1;

	  l = ret_chk;
	  int got_it = 0;

	  for (m = 0;  m < cur_sen_ids->num_stuff; m++)
	    {
	      sen_id * cur_sen_id;
	      cur_sen_id = vec_nth (cur_sen_ids, m);

	      if (!strcmp (cur_sen_id->sen, new_sen))
		break;

	      if (cur_sen_id->id == pf_id[k])
		got_it = 1;
	    }

	  if (m != cur_sen_ids->num_stuff)
	    {
	      sen_id * m_sen_id;
	      m_sen_id = vec_nth (cur_sen_ids, m);

	      if (pf_id[k] != m_sen_id->id)
		valid = 0;
	    }
	  else if (!got_it)
	    {
	      // This sentence id was not found elsewhere.

	      sen_id new_sen_id;
	      new_sen_id.sen = strdup (new_sen);
	      CHECK_ALLOC (new_sen_id.sen, -1);
	      new_sen_id.id = pf_id[k];

	      ret_chk = vec_add_obj (cur_sen_ids, &new_sen_id);
	      if (ret_chk < 0)
		return -1;
	    }
	  else
	    {
	      valid = 0;
	    }

	  free (new_sen);
	  new_sen = NULL;

	  break;
	}

      if (!valid)
	{
	  // Remove the new sentence ids.

	  while (cur_sen_ids->num_stuff > cur_len)
	    {
	      sen_id * cur_sen_id;
	      cur_sen_id = vec_nth (cur_sen_ids, cur_sen_ids->num_stuff - 1);

	      free (cur_sen_id->sen);
	      vec_pop_obj (cur_sen_ids);
	    }

	  break;
	}
      k++;
    }

  l = (valid) ? l : 0;
  return l;
}

/* Determines whether or not an equivalence follows from the lemma.
 *  input:
 *    prem - the premise of the argument.
 *    conc - the conclusion of the argument.
 *    pf_ids - the sentence ids from the lemma.
 *  output:
 *    0 - success
 *    -1 - memory error.
 *    -2 - failure.
 */
int
equiv_lemma (unsigned char * prem,
	     unsigned char * conc,
	     int ** pf_ids)
{
  int i, rc, j;
  int init_pos, end_pos;
  vec_t * sen_ids;

  for (i = 0; prem[i]; i++)
    {
      sen_ids = init_vec (sizeof (sen_id));

      rc = sexpr_id_chk (prem + i, pf_ids[0], sen_ids);
      if (rc == -1)
	return -1;

      if (!rc)
	continue;

      init_pos = i;
      end_pos = rc;

      rc = sexpr_id_chk (conc + i, pf_ids[1], sen_ids);
      if (rc == -1)
	return -1;

      if (!rc)
	{
	  for (j = 0; j < sen_ids->num_stuff; j++)
	    {
	      sen_id * cur_sen_id;
	      cur_sen_id = vec_nth (sen_ids, j);
	      free (cur_sen_id->sen);
	    }
	  destroy_vec (sen_ids);

	  continue;
	}
      break;
    }

  rc = i;

  if (!prem[rc])
    return -2;

  return 0;
}

char *
process_misc (unsigned char * conc, vec_t * prems, const char * rule, vec_t * vars,
	      proof_t * proof)
{
  char * ret = NOT_MINE;

  if (!strcmp (rule, "lm"))
    {
      if (!proof)
	return _("A proof must be specified.");

      ret = proc_lm (prems, conc, proof);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "sp"))
    {
      if (prems->num_stuff < 2)
	return _("Subproof requires a subproof as a reference."); 

      unsigned char * prem_0, * prem_1;

      prem_0 = vec_str_nth (prems, 0);
      prem_1 = vec_str_nth (prems, 1);

      ret = proc_sp (prem_0, prem_1, conc);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "sq"))
    {
      if (prems->num_stuff != 0)
	return _("Sequence requires zero (0) references.");

      ret = proc_sq (conc, vars);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "in"))
    {
      if (prems->num_stuff != 2)
	return _("Induction requires two (2) references.");

      ret = proc_in (vec_str_nth (prems, 0),
		     vec_str_nth (prems, 1),
		     conc, vars);
      if (!ret)
	return NULL;
    }

  return ret;
}

char *
proc_lm (vec_t * prems, unsigned char * conc, proof_t * proof)
{
  // First, get the premises and goals from the proof.

  vec_t * pf_sens;

  pf_sens = init_vec (sizeof (char *));
  if (!pf_sens)
    return NULL;

  item_t * itr;
  int ret_chk, num_pf_refs;

  for (itr = proof->everything->head; itr; itr = itr->next)
    {
      sen_data * sd;
      unsigned char * sexpr = NULL;

      sd = itr->value;
      if (!sd->premise || sd->text[0] == '\0')
	break;

      ret_chk = sen_convert_sexpr (sd->text, &sexpr);
      if (ret_chk == -1)
	return NULL;

      ret_chk = vec_str_add_obj (pf_sens, sexpr);
      if (ret_chk < 0)
	return NULL;
      free (sexpr);
    }

  num_pf_refs = pf_sens->num_stuff;

  if (num_pf_refs != prems->num_stuff)
    {
      destroy_str_vec (pf_sens);
      return _("Lemma requires the same amount of references as the amount of premises in the proof.");
    }

  for (itr = proof->goals->head; itr; itr = itr->next)
    {
      unsigned char * sd, * sexpr = NULL;

      sd = itr->value;
      ret_chk = sen_convert_sexpr (sd, &sexpr);
      if (ret_chk == -1)
	return NULL;

      ret_chk = vec_str_add_obj (pf_sens, sexpr);
      if (ret_chk < 0)
	return NULL;
      free (sexpr);
    }

  int ** pf_ids;
  pf_ids = (int **) calloc (pf_sens->num_stuff, sizeof (int *));
  CHECK_ALLOC (pf_ids, NULL);

  vec_t * pf_sen_ids, * cur_sen_ids;
  pf_sen_ids = init_vec (sizeof (sen_id));
  if (!pf_sen_ids)
    return NULL;

  int i, j;
  for (i = 0; i < pf_sens->num_stuff; i++)
    {
      ret_chk = sexpr_get_ids (vec_str_nth (pf_sens, i), &pf_ids[i], pf_sen_ids);
      if (ret_chk == -1)
	return NULL;
    }

  if (proof->boolean)
    {
      ret_chk = equiv_lemma (vec_str_nth (prems, 0), conc, pf_ids);
      if (ret_chk == -1)
	return NULL;

      if (!ret_chk)
	return CORRECT;
      return _("Incorrect usage of lemma.");
    }

  int k, l, m, valid, cur_len, pf_len;
  short * check;

  check = (short *) calloc (prems->num_stuff + 1, sizeof (short));
  CHECK_ALLOC (check, NULL);

  cur_sen_ids = init_vec (sizeof (sen_id));
  if (!cur_sen_ids)
    return NULL;

  pf_len = pf_sens->num_stuff;
  destroy_str_vec (pf_sens);

  for (i = 0; i < pf_len; i++)
    {
      for (j = 0; j < prems->num_stuff + 1; j++)
	{
	  if (check[j])
	    continue;

	  if (j == prems->num_stuff && i < num_pf_refs)
	    {
	      destroy_vec (cur_sen_ids);
	      destroy_vec (pf_sen_ids);

	      return _("None of the references matched one of the proof premises.");
	    }

	  cur_len = cur_sen_ids->num_stuff;

	  unsigned char * cur_ref, * new_sen;
	  cur_ref = (j < prems->num_stuff) ? vec_str_nth (prems, j) : conc;

	  valid = sexpr_id_chk (cur_ref, pf_ids[i], cur_sen_ids);
	  if (valid)
	    {
	      cur_len = cur_sen_ids->num_stuff;
	      check[j] = 1;
	      break;
	    }
	}
    }

  // Clean Up

  for (i = 0; i < cur_sen_ids->num_stuff; i++)
    {
      sen_id * cur_sen_id;
      cur_sen_id = vec_nth (cur_sen_ids, i);
      free (cur_sen_id->sen);
    }
  destroy_vec (cur_sen_ids);

  for (i = 0; i < pf_sen_ids->num_stuff; i++)
    {
      sen_id * cur_sen_id;
      cur_sen_id = vec_nth (pf_sen_ids, i);
      free (cur_sen_id->sen);
    }
  destroy_vec (pf_sen_ids);

  for (i = 0; i < pf_len; i++)
    free (pf_ids[i]);
  free (pf_ids);

  for (i = 0; i < prems->num_stuff + 1; i++)
    if (!check[i])
      break;

  free (check);

  if (i == prems->num_stuff + 1)
    return CORRECT;
  return _("None of the goals from the proof matched up with the conclusion.");
}

char *
proc_sp (unsigned char * prem_0, unsigned char * prem_1, unsigned char * conc)
{
  int ftc;
  unsigned char * lsen, * rsen;

  lsen = rsen = NULL;
  ftc = sexpr_find_top_connective (conc, S_CON, &lsen, &rsen);
  if (ftc == -1)
    return NULL;

  if (ftc < 0)
    {
      if (lsen)  free (lsen);
      if (rsen)  free (rsen);

      return _("There must be a conditional in the conclusion.");
    }

  int cmp_0, cmp_1;

  cmp_0 = !strcmp (prem_0, lsen);
  cmp_1 = !strcmp (prem_1, rsen);

  free (lsen);
  free (rsen);

  if (!cmp_0 || !cmp_1)
    return _("The premise of the subproof must be the antecedent, and the conclusion must be the consequence.");

  return CORRECT;
}

char *
proc_sq (unsigned char * conc, vec_t * vars)
{
  unsigned char * scope, * var, quant[S_CL + 1];

  scope = sexpr_elim_quant (conc, quant, &var);
  if (!scope)
    return NULL;

  if (scope[0] == '\0' || strcmp (quant, S_UNV))
    {
      if (var)  free (var);
      return _("There must be a universal at the beginning of the conclusion.");
    }

  if (scope[1] != '=')
    {
      free (var);
      return _("There must be an identity predicate in the scope.");
    }

  unsigned char * pred;
  int gpa;
  vec_t * args, * offsets;

  offsets = init_vec (sizeof (int));
  if (!offsets)
    return NULL;

  gpa = sexpr_get_quant_vars (conc, offsets);
  if (gpa == -1)
    return NULL;
  destroy_vec (offsets);

  if (gpa != 2)
    {
      free (var);

      return _("The variable must be used only twice.");
    }


  args = init_vec (sizeof (char *));
  if (!args)
    return NULL;

  gpa = sexpr_get_pred_args (scope, &pred, args);
  if (gpa == -1)
    return NULL;
  free (pred);
  free (scope);

  unsigned char * arg_0, * arg_1;
  arg_0 = vec_str_nth (args, 0);
  arg_1 = vec_str_nth (args, 1);

  vec_t * args_0;
  args_0 = init_vec (sizeof (char *));
  if (!args_0)
    return NULL;
  gpa = sexpr_get_pred_args (arg_0, &pred, args_0);
  if (gpa == -1)
    return NULL;

  gpa = !strcmp (pred, "v");
  free (pred);
  if (!gpa)
    {
      destroy_str_vec (args);
      destroy_str_vec (args_0);

      return _("The first argument must be a value function.");
    }

  int i;
  for (i = 0; i < vars->num_stuff; i++)
    {
      variable * cur_var;
      cur_var = vec_nth (vars, i);
      if (!strcmp (cur_var->text, vec_str_nth (args_0, 0)))
	break;
    }

  if (i != vars->num_stuff)
    {
      destroy_str_vec (args);
      destroy_str_vec (args_0);
      free (var);

      return _("The sequence variable must not have been used before.");
    }

  if (strcmp (vec_str_nth (args_0, 1), var))
    {
      destroy_str_vec (args);
      destroy_str_vec (args_0);
      free (var);

      return _("The variable must be the second argument of the value function.");
    }

  vec_t * args_1;
  unsigned char * tmp_arg = arg_1;

  while (1)
    {
      args_1 = init_vec (sizeof (char *));
      if (!args_1)
	return NULL;

      gpa = sexpr_get_pred_args (tmp_arg, &pred, args_1);
      if (gpa == -1)
	return NULL;

      if (gpa == 0)
	break;

      for (i = 0; i < args_1->num_stuff; i++)
	{
	  if (!strcmp (vec_str_nth (args_1, i), vec_str_nth (args_0, 0)))
	    break;
	}

      if (i != args_1->num_stuff)
	{
	  if (tmp_arg != arg_1)
	    free (tmp_arg);
	  free (var);
	  destroy_str_vec (args);
	  destroy_str_vec (args_0);
	  destroy_str_vec (args_1);

	  return _("The sequence must only be used once.");
	}

      if (strcmp (vec_str_nth (args_1, i - 1), var))
	{
	  if (tmp_arg != arg_1)
	    free (tmp_arg);
	  tmp_arg = (unsigned char *) calloc (strlen (vec_str_nth (args_1, i - 1)) + 1, sizeof (char));
	  CHECK_ALLOC (tmp_arg, NULL);
	  strcpy (tmp_arg, vec_str_nth (args_1, i - 1));
	  destroy_str_vec (args_1);
	  args_1 = NULL;
	  continue;
	}

      break;
    }

  if (tmp_arg != arg_1)
    free (tmp_arg);
  free (var);
  destroy_str_vec (args);
  destroy_str_vec (args_0);
  destroy_str_vec (args_1);

  if (gpa == 0)
    return _("The final argument of the sequence's function must be the variable.");

  return CORRECT;
}

char *
proc_in (unsigned char * prem_0, unsigned char * prem_1, unsigned char * conc, vec_t * vars)
{
  unsigned char * c_scope, * c_var, c_quant[S_CL + 1];

  unsigned char * sh_sen, * ln_sen;
  int p0_len, p1_len;

  p0_len = strlen (prem_0);
  p1_len = strlen (prem_1);

  if (p0_len < p1_len)
    {
      sh_sen = prem_0;
      ln_sen = prem_1;
    }
  else
    {
      sh_sen = prem_1;
      ln_sen = prem_0;
    }

  c_scope = sexpr_elim_quant (conc, c_quant, &c_var);
  if (!c_scope)
    return NULL;

  if (c_scope[0] == '\0' || strcmp (c_quant, S_UNV))
    {
      if (c_scope[0] != '\0')  free (c_scope);
      if (c_var)  free (c_var);

      return _("The conclusion must start with a universal.");
    }

  vec_t * var_offs;
  int chk, v_len;

  v_len = strlen (c_var);

  var_offs = init_vec (sizeof (int));
  if (!var_offs)
    return NULL;

  chk = sexpr_get_quant_vars (conc, var_offs);
  if (chk == -1)
    return NULL;

  if (chk == 0)
    {
      destroy_vec (var_offs);
      if (c_scope[0] != '\0')  free (c_scope);
      if (c_var)  free (c_var);

      return _("The new variable did not appear in the conclusion.");
    }

  unsigned char * z_scope, * z_var;

  z_var = (unsigned char *) calloc (v_len + 5, sizeof (char));
  CHECK_ALLOC (z_var, NULL);
  sprintf (z_var, "(z %s)", c_var);

  chk = sexpr_replace_var (c_scope, z_var, c_var, var_offs, &z_scope);
  if (chk == -1)
    return NULL;
  free (z_var);

  unsigned char * s_scope, * s_var;

  s_var = (unsigned char *) calloc (v_len + 5, sizeof (char));
  CHECK_ALLOC (s_var, NULL);
  sprintf (s_var, "(s %s)", c_var);

  chk = sexpr_replace_var (c_scope, s_var, c_var, var_offs, &s_scope);
  if (chk == -1)
    return NULL;
  free (s_var);

  destroy_vec (var_offs);

  unsigned char * in_str;
  int alloc_size;

  alloc_size = S_CL * 3 + v_len + strlen (z_scope)
    + strlen (c_scope) + strlen (s_scope) + 14;

  in_str = (unsigned char *) calloc (alloc_size + 1, sizeof (char));
  CHECK_ALLOC (in_str, NULL);
  sprintf (in_str, "((%s %s) (%s %s (%s %s %s)))",
	   S_UNV, c_var, S_AND, z_scope, S_CON, c_scope, s_scope);

  free (c_scope);
  free (s_scope);
  free (z_scope);
  free (c_var);

  unsigned char * oth_str;
  alloc_size = p0_len + p1_len + 4 + S_CL;
  oth_str = (unsigned char *) calloc (alloc_size + 1, sizeof (char));
  CHECK_ALLOC (oth_str, NULL);

  sprintf (oth_str, "(%s %s %s)", S_AND, sh_sen, ln_sen);

  char * ret_str = proc_ug (oth_str, in_str, vars);

  free (oth_str);
  free (in_str);

  chk = !strcmp (ret_str, CORRECT);

  if (chk)
    return CORRECT;
  return _("Induction constructed incorrectly.");
}

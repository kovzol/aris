/* Functions for processing quantifier rules.

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

#include "sexpr-process.h"
#include "vec.h"
#include "var.h"

int
help_fv (unsigned char * eq_sen, unsigned char * oth_sen, unsigned char * conc)
{
  unsigned char * var_0, * var_1, * pred, * oth_var, * conc_var;
  vec_t * args;
  int cmp, i, v0_len, v1_len;

  i = find_difference (oth_sen, conc);
  if (i == -1)
    return 1;

  args = init_vec (sizeof (char *));
  if (!args)
    return AEC_MEM;

  cmp = sexpr_get_pred_args (eq_sen, &pred, args);
  if (cmp == AEC_MEM)
    return AEC_MEM;
  free (pred);

  if (cmp != 2)
    {
      destroy_str_vec (args);
      return -2;
    }

  var_0 = vec_str_nth (args, 0);
  var_1 = vec_str_nth (args, 1);

  v0_len = strlen (var_0);
  v1_len = strlen (var_1);

  cmp = find_difference (var_0, var_1);

  int o_cmp_0, o_cmp_1, c_cmp_0, c_cmp_1;

  o_cmp_0 = !strncmp (oth_sen + i - cmp, var_0, v0_len);
  o_cmp_1 = !strncmp (oth_sen + i - cmp, var_1, v1_len);
  c_cmp_0 = !strncmp (conc + i - cmp, var_0, v0_len);
  c_cmp_1 = !strncmp (conc + i - cmp, var_1, v1_len);

  if (!((o_cmp_0 && c_cmp_1) || (o_cmp_1 && c_cmp_0)))
    {
      destroy_str_vec (args);
      return -2;
    }

  if (!strncmp (oth_sen + i - cmp, var_0, v0_len))
    {
      oth_var = var_0;
      conc_var = var_1;
    }
  else
    {
      oth_var = var_1;
      conc_var = var_0;
    }

  unsigned char * cons_sen;
  int cons_pos;

  cons_sen = (unsigned char *) calloc (strlen (oth_sen) + strlen (conc_var) + 1,
				       sizeof (char));
  CHECK_ALLOC (cons_sen, AEC_MEM);

  strncpy (cons_sen, oth_sen, i - cmp);
  cons_pos = i - cmp;

  cons_pos += sprintf (cons_sen + cons_pos, "%s", conc_var);
  strcpy (cons_sen + cons_pos, oth_sen + i - cmp + strlen (oth_var));
  destroy_str_vec (args);

  int ret_chk;
  ret_chk = help_fv (eq_sen, cons_sen, conc);
  if (ret_chk == AEC_MEM)
    return AEC_MEM;

  free (cons_sen);
  if (ret_chk == 0 || ret_chk == 1)
    return 0;

  return -2;
}

char *
process_quantifiers (unsigned char * conc, vec_t * prems, const char * rule, vec_t * vars)
{
  char * ret = NOT_MINE;
  unsigned char * prem;
  prem = vec_str_nth (prems, 0);
  
  if (!strcmp (rule, "ug"))
    {
      if (prems->num_stuff != 1)
	return _("Universal Generalization requires one (1) references.");

      ret = proc_ug (prem, conc, vars);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "ui"))
    {
      if (prems->num_stuff != 1)
	return _("Universal Instantiation requires one (1) reference.");

      ret = proc_ui (prem, conc);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "eg"))
    {
      if (prems->num_stuff != 1)
	return _("Existential Generalization requires one (1) reference.");

      ret = proc_eg (prem, conc);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "ei"))
    {
      if (prems->num_stuff != 1)
	return _("Existential Instantiation requires one (1) references.");

      ret = proc_ei (prem, conc, vars);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "bv"))
    {
      if (prems->num_stuff != 1)
	return _("Bound Variable requires one (1) reference.");

      ret = proc_bv (prem, conc);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "nq"))
    {
      if (prems->num_stuff != 1)
	return _("Null Quantification requires one (1) reference.");

      ret = proc_nq (prem, conc);
      if (!ret)
	return NULL;
    }  /*  End of null quantifier.  */

  if (!strcmp (rule, "pr"))
    {
      if (prems->num_stuff != 1)
	return _("Prenex requires one (1) reference.");

      ret = proc_pr (prem, conc);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "ii"))
    {
      if (prems->num_stuff != 0)
	return _("Identity requires zero (0) references.");

      ret = proc_ii (conc);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "fv"))
    {
      if (prems->num_stuff != 2)
	return _("Free Variable requires one two (2) references.");

      unsigned char * prem_0;
      prem_0 = vec_str_nth (prems, 1);

      ret = proc_fv (prem, prem_0, conc);
      if (!ret)
	return NULL;
    }

  return ret;
}

char *
proc_ug (unsigned char * prem, unsigned char * conc, vec_t * vars)
{
  int ret_chk;

  ret_chk = sexpr_quant_infer (conc, prem, S_UNV, 1, vars);
  if (ret_chk == AEC_MEM)
    return NULL;

  if (ret_chk == 0)
    return CORRECT;
  else if (ret_chk == -3)
    return _("The variable must be arbitrary.");
  else
    return _("Universal Generalization constructed incorrectly.");
}

char *
proc_ui (unsigned char * prem, unsigned char * conc)
{
  int ret_chk;
  ret_chk = sexpr_quant_infer (prem, conc, S_UNV, 0, NULL);
  if (ret_chk == AEC_MEM)
    return NULL;

  if (ret_chk == 0)
    return CORRECT;
  else
    return _("Universal Instantiation constructed incorrectly.");
}

char *
proc_eg (unsigned char * prem, unsigned char * conc)
{
  int ret_chk;
  ret_chk = sexpr_quant_infer (conc, prem, S_EXL, 0, NULL);
  if (ret_chk == AEC_MEM)
    return NULL;

  if (ret_chk == 0)
    return CORRECT;
  else
    return _("Existential Generalization constructed incorrectly.");
}

char *
proc_ei (unsigned char * prem, unsigned char * conc, vec_t * vars)
{
  int ret_chk;
  ret_chk = sexpr_quant_infer (prem, conc, S_EXL, 2, vars);
  if (ret_chk == AEC_MEM)
    return NULL;

  if (ret_chk == 0)
    return CORRECT;
  else if (ret_chk == -3)
    return _("The variable must not have been used.");
  else
    return _("Existential Instantiation constructed incorrectly.");
}

char *
proc_bv (unsigned char * prem, unsigned char * conc)
{
  int i, pi, ci;
  i = find_difference (prem, conc);
  if (i == AEC_MEM)
    return NO_DIFFERENCE;

  pi = ci = i;
  while (pi >= 0 && prem[pi] != ' ')
    pi--;
  while (ci >= 0 && conc[ci] != ' ')
    ci--;

  if (pi < 0 || ci < 0)
    return _("The difference must be a bound variable.");

  pi -= S_CL + 2;
  ci -= S_CL + 2;

  if (prem[pi] != '(' || conc[ci] != '(')
    return _("The difference must be a bound variable.");

  int tmp_p, tmp_c;
  unsigned char * p_str, * c_str;

  tmp_p = parse_parens (prem, pi, &p_str);
  if (tmp_p == AEC_MEM)
    return NULL;

  tmp_c = parse_parens (conc, ci, &c_str);
  if (tmp_c == AEC_MEM)
    return NULL;

  if (!c_str || !p_str)
    {
      if (c_str) free (c_str);
      if (p_str) free (p_str);
      return _("Bound Variable constructed incorrectly.");
    }

  if (strcmp (prem + tmp_p, conc + tmp_c))
    {
      free (p_str);
      free (c_str);
      return _("The rest of the sentences must be the same.");
    }

  unsigned char * p_var, * c_var, * p_scope, * c_scope;
  unsigned char p_quant[S_CL + 1], c_quant[S_CL + 1];

  p_scope = sexpr_elim_quant (p_str, p_quant, &p_var);
  if (!p_scope)
    return NULL;

  c_scope = sexpr_elim_quant (c_str, c_quant, &c_var);
  if (!c_scope)
    return NULL;
  free (c_str);

  if (p_scope[0] == '\0' || c_scope[0] == '\0')
    {
      if (c_var)  free (c_var);
      if (p_var)  free (p_var);

      free (c_scope);
      free (p_scope);
      free (p_str);

      return _("There must be quantifiers at the difference.");
    }

  if (strcmp (p_quant, c_quant))
    {
      free (c_var);
      free (p_var);
      free (c_scope);
      free (p_scope);
      free (p_str);

      return _("The quantifiers must be the same.");
    }

  vec_t * p_vars;

  p_vars = init_vec (sizeof (int));
  if (!p_vars)
    return NULL;

  tmp_p = sexpr_get_quant_vars (p_str, p_vars);
  if (tmp_p == AEC_MEM)
    return NULL;
  free (p_str);

  unsigned char * oth_sen;
  int cmp;

  tmp_p = sexpr_replace_var (p_scope, c_var, p_var, p_vars, &oth_sen);
  if (tmp_p == AEC_MEM)
    return NULL;

  destroy_vec (p_vars);
  free (c_var);
  free (p_var);
  free (p_scope);

  cmp = !strcmp (oth_sen, c_scope);

  free (oth_sen);
  free (c_scope);

  if (!cmp)
    return _("Bound Variable Substitution constructed incorrectly.");

  return CORRECT;
}

char *
proc_nq (unsigned char * prem, unsigned char * conc)
{
  unsigned char * ln_sen, * sh_sen;
  int p_len, c_len, l_len;

  p_len = strlen (prem);
  c_len = strlen (conc);

  if (p_len > c_len)
    {
      ln_sen = prem;  l_len = p_len;
      sh_sen = conc;
    }
  else
    {
      ln_sen = conc;  l_len = c_len;
      sh_sen = prem;
    }

  int i, li;
  i = find_difference (ln_sen, sh_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  if (ln_sen[i] != '(' && strncmp (ln_sen + i - 1, S_UNV, S_CL)
      && strncmp (ln_sen + i - 1, S_EXL, S_CL) && !islower (ln_sen[i]))
    {
      return _("Null Quantifier constructed incorrectly.");
    }

  if (ln_sen[i] == '(')
    {
      li = i - 1;
    }
  else if (!strncmp (ln_sen + i - 1, S_UNV, S_CL)
	   || !strncmp (ln_sen + i - 1, S_EXL, S_CL))
    {
      li = i - 3;
    }
  else
    {
      //backtrack to the quantifier.
      li = i;
      while (!isspace (ln_sen[li]))
	li--;

      li -= 2 + S_CL;
    }

  unsigned char * tmp_str;
  int tmp_pos;

  if (li >= 0)
    {
      tmp_pos = parse_parens (ln_sen, li, &tmp_str);
      if (tmp_pos == AEC_MEM)
        return NULL;

      if (!tmp_str)
        return _("Null Quantifier constructed incorrectly.");
    }
  else
    {
      tmp_pos = l_len - 1;
      tmp_str = strdup (ln_sen);
      CHECK_ALLOC (tmp_str, NULL);
    }

  unsigned char * scope, * var, quant[S_CL + 1];
  scope = sexpr_elim_quant (tmp_str, quant, &var);
  if (!scope)
    return NULL;

  free (var);

  int gqv;
  vec_t * offsets;

  offsets = init_vec (sizeof (int));
  if (!offsets)
    return NULL;

  gqv = sexpr_get_quant_vars (tmp_str, offsets);
  if (gqv == AEC_MEM)
    return NULL;
  free (tmp_str);
  destroy_vec (offsets);

  if (gqv != 0)
    {
      free (scope);
      return _("The variables must not appear in the scope.");
    }

  unsigned char * oth_sen;
  int oth_pos, alloc_size;

  alloc_size = l_len - strlen (var) - S_CL - 5;
  oth_sen = (unsigned char *) calloc (alloc_size + 1, sizeof (char));
  CHECK_ALLOC (oth_sen, NULL);

  li = (li < 0) ? 0 : li;
  strncpy (oth_sen, ln_sen, li);
  oth_pos = li;

  oth_pos += sprintf (oth_sen + oth_pos, "%s", scope);
  strcpy (oth_sen + oth_pos, ln_sen + tmp_pos + 1);

  free (scope);

  char * ret_str;
  if (ln_sen == conc)
    ret_str = proc_nq (sh_sen, oth_sen);
  else
    ret_str = proc_nq (oth_sen, sh_sen);
  if (!ret_str)
    return NULL;

  free (oth_sen);
  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Null Quantifier constructed incorrectly.");
}

char *
proc_pr (unsigned char * prem, unsigned char * conc)
{
  unsigned char * ln_sen, * sh_sen;
  int p_len, c_len, s_len;

  p_len = strlen (prem);
  c_len = strlen (conc);

  int i;
  i = find_difference (prem, conc);
  if (i == -1)
    return NO_DIFFERENCE;

  if (prem[i] == '(')
    {
      sh_sen = prem;  s_len = p_len;
      ln_sen = conc;
    }
  else if (conc[i] == '(')
    {
      sh_sen = conc;  s_len = c_len;
      ln_sen = prem;
    }
  else
    {
      return _("There must be a quantifier at the difference.");
    }

  unsigned char * tmp_str;
  int tmp_pos;

  if (i == 0)
    return _("Prenex constructed incorrectly.");

  tmp_pos = parse_parens (sh_sen, i - 1, &tmp_str);
  if (tmp_pos == AEC_MEM)
    return NULL;

  if (!tmp_str)
    return _("Prenex constructed incorrectly.");

  unsigned char * scope, * var, quant[S_CL + 1];
  int v_len;

  scope = sexpr_elim_quant (tmp_str, quant, &var);
  if (!scope)
    return NULL;
  if (scope[0] == '\0')
    return _("Prenex constructed incorrectly.");
  v_len = strlen (var);
  free (tmp_str);

  int gg;
  vec_t * gg_vec;
  unsigned char conn[S_CL + 1];

  conn[0] = '\0';
  gg_vec = init_vec (sizeof (char *));
  if (!gg_vec)
    return NULL;

  gg = sexpr_get_generalities (scope, conn, gg_vec);
  if (gg == AEC_MEM)
    return NULL;
  free (scope);

  if (gg == 1)
    {
      destroy_str_vec (gg_vec);
      return _("There must be generalities in the scope.");
    }

  if (strcmp (conn, S_AND) && strcmp (conn, S_OR))
    {
      destroy_str_vec (gg_vec);
      return _("The connective must be a conjunction or disjunction.");
    }

  // Determine which generalities are which.

  vec_t * var_gens, * nul_gens;
  int ret_chk, j;
  
  var_gens = init_vec (sizeof (char *));
  if (!var_gens)
    return NULL;

  nul_gens = init_vec (sizeof (char *));
  if (!nul_gens)
    return NULL;

  for (j = 0; j < gg_vec->num_stuff; j++)
    {
      unsigned char * cur_gen, * str;
      cur_gen = vec_str_nth (gg_vec, j);

      str = strstr (cur_gen, var);
      if (!str)
	break;

      while (*(str - 1) == '(' || (str[v_len] != ' ' && str[v_len] != ')'))
	{
	  str = strstr (cur_gen + 1, var);
	  if (str == NULL)
	    break;
	}
      if (!str)
	break;

      ret_chk = vec_str_add_obj (var_gens, cur_gen);
      if (ret_chk == AEC_MEM)
	return NULL;
    }

  for (; j < gg_vec->num_stuff; j++)
    {
      unsigned char * cur_gen, * str;
      cur_gen = vec_str_nth (gg_vec, j);
      str = strstr (cur_gen, var);
      while (str)
	{
	  if (*(str - 1) != '(' && (str[v_len] == ' ' || str[v_len] == ')'))
	    break;
	  str = strstr (cur_gen + 1, var);
	}

      ret_chk = vec_str_add_obj (nul_gens, cur_gen);
      if (ret_chk == AEC_MEM)
	return NULL;
    }

  destroy_str_vec (gg_vec);

  // Construct what should be the other sentence.

  unsigned char * oth_sen;
  int oth_pos, alloc_size;

  // 2 extra sets of parentheses + 3 extra spaces + 2 more connectives
  alloc_size = s_len + 7 + 2 * S_CL + 1;
  oth_sen = (unsigned char *) calloc (alloc_size, sizeof (char));
  CHECK_ALLOC (oth_sen, NULL);
  strncpy (oth_sen, sh_sen, i - 1);
  oth_pos = i - 1;

  oth_pos += sprintf (oth_sen + oth_pos, "(%s ", conn);
  oth_pos += sprintf (oth_sen + oth_pos, "((%s %s)", quant, var);

  if (var_gens->num_stuff > 1)
    oth_pos += sprintf (oth_sen + oth_pos, " (%s", conn);

  for (j = 0; j < var_gens->num_stuff; j++)
    {
      oth_pos += sprintf (oth_sen + oth_pos, " %s",
			  vec_str_nth (var_gens, j));
    }

  if (var_gens->num_stuff > 1)
    oth_pos += sprintf (oth_sen + oth_pos, ")");
  destroy_str_vec (var_gens);

  oth_pos += sprintf (oth_sen + oth_pos, ")");

  if (nul_gens->num_stuff > 1)
    oth_pos += sprintf (oth_sen + oth_pos, " (%s", conn);

  for (j = 0; j < nul_gens->num_stuff; j++)
    {
      oth_pos += sprintf (oth_sen + oth_pos, " %s",
			  vec_str_nth (nul_gens, j));
    }

  if (nul_gens->num_stuff > 1)
    oth_pos += sprintf (oth_sen + oth_pos, ")");
  destroy_str_vec (nul_gens);

  oth_pos += sprintf (oth_sen + oth_pos, ")");

  strcpy (oth_sen + oth_pos, sh_sen + tmp_pos + 1);

  free (var);

  char * ret_str;
  if (sh_sen == conc)
    ret_str = proc_pr (ln_sen, oth_sen);
  else
    ret_str = proc_pr (oth_sen, ln_sen);
  if (!ret_str)
    return NULL;

  free (oth_sen);

  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Prenex constructed incorrectly.");
}

char *
proc_ii (unsigned char * conc)
{
  int gpa;
  unsigned char * pred;
  vec_t * args;

  args = init_vec (sizeof (char *));
  if (!args)
    return NULL;

  gpa = sexpr_get_pred_args (conc, &pred, args);
  if (gpa == AEC_MEM)
    return NULL;

  if (gpa != 2 || strcmp (pred, "="))
    {
      if (gpa == 1)
	destroy_vec (args);
      else
	destroy_str_vec (args);

      return _("The conclusion must have an identity predicate.");
    }
  free (pred);

  unsigned char * arg_0, * arg_1;

  arg_0 = vec_str_nth (args, 0);
  arg_1 = vec_str_nth (args, 1);
  gpa = !strcmp (arg_0, arg_1);

  destroy_str_vec (args);

  if (gpa)
    return CORRECT;
  else
    return _("The two arguments to the identity predicate must be the same.");
}

char *
proc_fv (unsigned char * prem_0, unsigned char * prem_1, unsigned char * conc)
{
  int first_0, first_1;
  unsigned char * eq_sen, * oth_sen;

  first_0 = (prem_0[1] == '=') ? 1 : 0;
  first_1 = (prem_1[1] == '=') ? 1 : 0;

  if (first_0 && first_1)
    {
      int ret_0, ret_1;

      ret_0 = help_fv (prem_0, prem_1, conc);
      if (ret_0 == AEC_MEM)
	return NULL;

      ret_1 = help_fv (prem_1, prem_0, conc);
      if (ret_1 == AEC_MEM)
	return NULL;

      if (ret_0 == 0 || ret_1 == 0)
	return CORRECT;

      return _("Free Variable substitution constructed incorrectly.");
    }
  else if (first_0)
    {
      eq_sen = prem_0;
      oth_sen = prem_1;
    }
  else if (first_1)
    {
      eq_sen = prem_1;
      oth_sen = prem_0;
    }
  else
    {
      return _("One of the premises must contain an identity predicate.");
    }

  int ret;
  ret = help_fv (eq_sen, oth_sen, conc);
  if (ret == AEC_MEM)
    return NULL;

  if (ret == 0)
    return CORRECT;

  return _("Free Variable Substitution constructed incorrectly.");
}

/* Functions for processing equivalence rules.

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

/* All equivalence functions have the following format:
 *  input:
 *    the conclusion and premise, in that order.
 *  output:
 *    CORRECT if the evaluation checks out, or an error message otherwise.
 */

/* All equivalence functions (except proc_co and proc_id) have a similar structure:
 *  First, determine some values based on the premise and conclusion (usually strlen).
 *  Use these values to determine which sentence is which.
 *  Find difference, returning NO_DIFFERENCE if i == -1.
 *  Calculate the desired offset.
 *  Parse parens to get the working scope.
 *  From this, get the necessary parts.
 *  Using these, construct what should be the other sentence.
 *  run recursion, check for NO_DIFFERENCE or CORRECT.
 *  return generic error otherwise.
 */

#include "sexpr-process.h"
#include "vec.h"

int
recurse_co (unsigned char * sen_0, unsigned char * sen_1)
{
  if (!strcmp (sen_0, sen_1))
    return -3;

  int gg_0, gg_1;
  vec_t * gens_0, * gens_1;
  int con_0, con_1;

  gens_0 = init_vec (sizeof (char *));
  if (!gens_0)
    return -1;

  gens_1 = init_vec (sizeof (char *));
  if (!gens_1)
    return -1;

  unsigned char conn_0[S_CL + 1], conn_1[S_CL + 1];

  conn_0[0] = '\0';
  conn_1[0] = '\0';

  gg_0 = sexpr_get_generalities (sen_0, conn_0, gens_0);
  if (gg_0 == -1)
    return -1;

  gg_1 = sexpr_get_generalities (sen_1, conn_1, gens_1);
  if (gg_1 == -1)
    return -1;

  if (gg_0 != gg_1)
    {
      destroy_str_vec (gens_0);
      destroy_str_vec (gens_1);
      return -2;
    }

  if (gg_0 == 1)
    {
      destroy_str_vec (gens_0);
      destroy_str_vec (gens_1);

      // If only one generality, there is either a negation or quantifier.

      // Check for a negation.
      if (sexpr_not_check (sen_0) || sexpr_not_check (sen_1))
	{
	  if (!sexpr_not_check (sen_0) || !sexpr_not_check (sen_1))
	    return -2;

	  // Eliminate the negation, and continue.

	  int ret;
	  unsigned char * not_sen_0, * not_sen_1;

	  not_sen_0 = sexpr_elim_not (sen_0);
	  if (!not_sen_0)
	    return -1;

	  not_sen_1 = sexpr_elim_not (sen_1);
	  if (!not_sen_1)
	    return -1;

	  ret = recurse_co (not_sen_0, not_sen_1);
	  if (ret == -1)
	    return -1;

	  free (not_sen_0);
	  free (not_sen_1);
	  return ret;
	}

      // Check for a quantifier.

      if ((sen_0[0] == '(' && sen_1[0] == '(')
	  && ((!strncmp (sen_0 + 2, S_UNV, S_CL)
	       || !strncmp (sen_0 + 2, S_EXL, S_CL))
	      || (!strncmp (sen_1 + 2, S_UNV, S_CL)
		  || !strncmp (sen_1 + 2, S_EXL, S_CL))))
	{
	  if (!(strncmp (sen_0 + 2, S_UNV, S_CL)
		&& strncmp (sen_0 + 2, S_EXL, S_CL))
	      && !(strncmp (sen_1 + 2, S_UNV, S_CL)
		   && strncmp (sen_1 + 2, S_EXL, S_CL)))
	    {
	      return -2;
	    }

	  // Eliminate the quantifier and continue.

	  unsigned char * scope_0, * scope_1, * var_0, * var_1;
	  unsigned char quant_0[S_CL + 1], quant_1[S_CL + 1];

	  scope_0 = sexpr_elim_quant (sen_0, quant_0, &var_0);
	  if (!scope_0)
	    return -1;

	  scope_1 = sexpr_elim_quant (sen_1, quant_1, &var_1);
	  if (!scope_1)
	    return -1;

	  int cmp;
	  cmp = !strcmp (var_0, var_1);

	  free (var_0);
	  free (var_1);

	  if (!cmp || strcmp (quant_0, quant_1))
	    {
	      free (scope_0);
	      free (scope_1);
	      return -2;
	    }

	  cmp = recurse_co (scope_0, scope_1);
	  if (cmp == -1)
	    return -1;

	  free (scope_0);
	  free (scope_1);
	  return cmp;
	}

      return -2;
    }

  con_0 = 1;
  if (strcmp (conn_0, S_AND) && strcmp (conn_0, S_OR))
    con_0 = 0;

  con_1 = 1;
  if (strcmp (conn_1, S_AND) && strcmp (conn_1, S_OR))
    con_1 = 0;

  if ((con_0 && !con_1) || (!con_0 && con_1))
    {
      destroy_str_vec (gens_0);
      destroy_str_vec (gens_1);
      return -2;
    }

  if (con_0 && con_1)
    {
      int cmp;
      cmp = vec_str_cmp (gens_0, gens_1);
      if (cmp == -1)
	return -1;

      if (cmp == 0)
	{
	  destroy_str_vec (gens_0);
	  destroy_str_vec (gens_1);
	  return 0;
	}
    }

  int ret, i;

  for (i = 0; i < gens_0->num_stuff; i++)
    {
      unsigned char * cur_0, * cur_1;

      cur_0 = vec_str_nth (gens_0, i);
      cur_1 = vec_str_nth (gens_1, i);

      if (!strcmp (cur_0, cur_1))
	continue;

      ret = recurse_co (cur_0, cur_1);
      if (ret == -1)
	return -1;

      if (ret != 0)
	{
	  destroy_str_vec (gens_0);
	  destroy_str_vec (gens_1);
	  return ret;
	}
    }

  destroy_str_vec (gens_0);
  destroy_str_vec (gens_1);

  return 0;
}

int
recurse_id (unsigned char * sen_0, unsigned char * sen_1)
{
  // sen_0 will be the larger sentence.

  if (!strcmp (sen_0, sen_1))
    return -3;

  int gg_0, gg_1;
  vec_t * gens_0, * gens_1;
  int con_0, con_1;

  gens_0 = init_vec (sizeof (char *));
  if (!gens_0)
    return -1;

  gens_1 = init_vec (sizeof (char *));
  if (!gens_1)
    return -1;

  unsigned char conn_0[S_CL + 1], conn_1[S_CL + 1];

  conn_0[0] = '\0';
  conn_1[0] = '\0';

  gg_0 = sexpr_get_generalities (sen_0, conn_0, gens_0);
  if (gg_0 == -1)
    return -1;

  gg_1 = sexpr_get_generalities (sen_1, conn_1, gens_1);
  if (gg_1 == -1)
    return -1;

  if (gg_0 < gg_1)
    {
      destroy_str_vec (gens_0);
      destroy_str_vec (gens_1);
      return -2;
    }

  if (gg_0 == 1)
    {
      destroy_str_vec (gens_0);
      destroy_str_vec (gens_1);

      if (sexpr_not_check (sen_0) || sexpr_not_check (sen_1))
	{
	  if (!sexpr_not_check (sen_0) || !sexpr_not_check (sen_1))
	    return -2;

	  int ret;
	  unsigned char * not_sen_0, * not_sen_1;

	  not_sen_0 = sexpr_elim_not (sen_0);
	  if (!not_sen_0)
	    return -1;

	  not_sen_1 = sexpr_elim_not (sen_1);
	  if (!not_sen_1)
	    return -1;

	  ret = recurse_id (not_sen_0, not_sen_1);
	  if (ret == -1)
	    return -1;

	  free (not_sen_0);
	  free (not_sen_1);
	  return ret;
	}

      if ((sen_0[0] == '(' && sen_1[0] == '(')
	  && ((!strncmp (sen_0 + 2, S_UNV, S_CL)
	       || !strncmp (sen_0 + 2, S_EXL, S_CL))
	      || (!strncmp (sen_1 + 2, S_UNV, S_CL)
		  || !strncmp (sen_1 + 2, S_EXL, S_CL))))
	{
	  if (!(strncmp (sen_0 + 2, S_UNV, S_CL) && strncmp (sen_0 + 2, S_EXL, S_CL))
	      && !(strncmp (sen_1 + 2, S_UNV, S_CL)
		   && strncmp (sen_1 + 2, S_EXL, S_CL)))
	    return -2;

	  unsigned char * scope_0, * scope_1, * var_0, * var_1;
	  unsigned char quant_0[S_CL + 1], quant_1[S_CL + 1];

	  scope_0 = sexpr_elim_quant (sen_0, quant_0, &var_0);
	  if (!scope_0)
	    return -1;

	  scope_1 = sexpr_elim_quant (sen_1, quant_1, &var_1);
	  if (!scope_1)
	    return -1;

	  int cmp;
	  cmp = !strcmp (var_0, var_1);

	  free (var_0);
	  free (var_1);

	  if (!cmp || strcmp (quant_0, quant_1))
	    {
	      free (scope_0);
	      free (scope_1);
	      return -2;
	    }

	  cmp = recurse_id (scope_0, scope_1);
	  if (cmp == -1)
	    return -1;

	  free (scope_0);
	  free (scope_1);
	  return cmp;
	}

      return -2;
    }

  con_0 = 1;
  if (strcmp (conn_0, S_AND) && strcmp (conn_0, S_OR))
    con_0 = 0;

  con_1 = 1;
  if (gg_1 != 1 && (strcmp (conn_1, S_AND) && strcmp (conn_1, S_OR)))
    con_1 = 0;

  if ((con_0 && !con_1) || (!con_0 && con_1))
    {
      destroy_str_vec (gens_0);
      destroy_str_vec (gens_1);
      return -2;
    }

  if (con_0 && con_1)
    {
      int cmp;
      cmp = vec_str_sub (gens_1, gens_0);
      if (cmp == -1)
	return -1;

      if (cmp == 0)
	{
	  destroy_str_vec (gens_0);
	  destroy_str_vec (gens_1);
	  return 0;
	}
      else if (gg_0 != gg_1)
	{
	  destroy_str_vec (gens_0);
	  destroy_str_vec (gens_1);
	  return -2;
	}
    }

  int ret, i;

  for (i = 0; i < gens_0->num_stuff; i++)
    {
      unsigned char * cur_0, * cur_1;

      cur_0 = vec_str_nth (gens_0, i);
      cur_1 = vec_str_nth (gens_1, i);

      if (!strcmp (cur_0, cur_1))
	continue;

      ret = recurse_id (cur_0, cur_1);
      if (ret == -1)
	return -1;

      if (ret != 0)
	{
	  destroy_str_vec (gens_0);
	  destroy_str_vec (gens_1);
	  return ret;
	}
    }

  destroy_str_vec (gens_0);
  destroy_str_vec (gens_1);

  return 0;
}

char *
process_equivalence (unsigned char * conc, vec_t * prems, const char * rule)
{
  char * ret = NOT_MINE;
  unsigned char * prem;
  prem = vec_str_nth (prems, 0);

  if (!strcmp (rule, "im"))
    {
      if (prems->num_stuff != 1)
	return _("Implication requires one (1) reference.");

      ret = proc_im (prem, conc);
      if (!ret)
	return NULL;
    }  /* End of implication. */

  if (!strcmp (rule, "dm"))
    {
      if (prems->num_stuff != 1)
	return _("DeMorgan requires one (1) reference.");

      ret = proc_dm (prem, conc, -1);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "as"))
    {
      if (prems->num_stuff != 1)
	return _("Association requires one (1) reference.");

      ret = proc_as (prem, conc);
      if (!ret)
	return NULL;
    }  /* End of association. */

  if (!strcmp (rule, "co"))
    {
      if (prems->num_stuff != 1)
	return _("Commutativity requires one (1) reference.");

      ret = proc_co (prem, conc);
      if (!ret)
	return NULL;
    }  /* End of commutativity. */

  if (!strcmp (rule, "id"))
    {
      if (prems->num_stuff != 1)
	return _("Idempotence requires one (1) reference.");

      ret = proc_id (prem, conc);
      if (!ret)
	return NULL;
    }  /* End of idempotence. */

  if (!strcmp (rule, "dt"))
    {
      if (prems->num_stuff != 1)
	return _("Distribution requires one (1) reference.");

      ret = proc_dt (prem, conc, -1);
      if (!ret)
	return NULL;
    } /* End of distribution. */

  if (!strcmp (rule, "eq"))
    {
      if (prems->num_stuff != 1)
	return _("Equivalence requires one (1) reference.");

      ret = proc_eq (prem, conc);
      if (!ret)
	return NULL;
    }  /* End of equivalence. */

  if (!strcmp (rule, "dn"))
    {
      if (prems->num_stuff != 1)
	return _("Double Negation requires one (1) reference.");

      ret = proc_dn (prem, conc);
      if (!ret)
	return NULL;
    }  /* End of double negation. */

  if (!strcmp (rule, "ep"))
    {
      if (prems->num_stuff != 1)
	return _("Exportation requires one (1) reference.");

      ret = proc_ep (prem, conc);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "sb"))
    {
      if (prems->num_stuff != 1)
	return _("Subsumption requires one (1) reference.");

      ret = proc_sb (prem, conc);
      if (!ret)
	return NULL;
    }

  return ret;
}

char *
proc_im (unsigned char * prem, unsigned char * conc)
{
  unsigned char * dis_sen, * con_sen;
  int p_len, c_len, d_len;

  p_len = strlen (prem);
  c_len = strlen (conc);

  if (p_len > c_len)
    {
      dis_sen = prem;  d_len = p_len;
      con_sen = conc;
    }
  else
    {
      dis_sen = conc;  d_len = c_len;
      con_sen = prem;
    }

  int i;

  i = find_difference (dis_sen, con_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  // i might be zero if this is an incorrect usage of im.
  // same with i == 1.
  // Only occurs if there is no top connective on one, or a quantifier.

  if (i < 2)
    return _("Incorrect usage of implication.");

  int tmp_pos;
  unsigned char * tmp_str;

  tmp_pos = parse_parens (dis_sen, i - 2, &tmp_str);
  if (tmp_pos == -2)
    return NULL;
  if (!tmp_str)
    return _("Implication constructed incorrectly.");

  int ftc;
  unsigned char * lsen, * rsen, * n_lsen;

  ftc = sexpr_find_top_connective (tmp_str, S_OR, &lsen, &rsen);
  if (ftc == -1)
    return NULL;

  free (tmp_str);

  if (ftc < 0)
    {
      if (lsen)  free (lsen);
      if (rsen)  free (rsen);

      return _("One sentence must contain a disjunction.");
    }

  if (!sexpr_not_check (lsen))
    {
      free (lsen);
      free (rsen);
      return _("The left disjunct must have a negation.");
    }

  n_lsen = sexpr_elim_not (lsen);
  if (!n_lsen)
    return NULL;

  free (lsen);

  // Construct what should be the other sentence.

  unsigned char * oth_sen;
  int oth_pos;

  oth_sen = (unsigned char * ) calloc (d_len - 2 - S_NL, sizeof (char));
  CHECK_ALLOC (oth_sen, NULL);
  strncpy (oth_sen, dis_sen, i - 1);
  oth_pos = i - 1;

  oth_pos += sprintf (oth_sen + oth_pos, "%s %s %s", S_CON, n_lsen, rsen);
  free (n_lsen);
  free (rsen);
  strcpy (oth_sen + oth_pos, dis_sen + tmp_pos);

  char * ret_str;
  if (dis_sen == conc)
    ret_str = proc_im (con_sen, oth_sen);
  else
    ret_str = proc_im (oth_sen, con_sen);
  if (!ret_str)
    return NULL;

  free (oth_sen);
  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Implication constructed incorrectly.");
}

char *
proc_dm (unsigned char * prem, unsigned char * conc, int mode_guess)
{
  int i;
  i = find_difference (prem, conc);
  if (i == -1)
    return NO_DIFFERENCE;

  int p_len, c_len, mode;

  if (mode_guess == -1)
    {
      p_len = strlen (prem);
      c_len = strlen (conc);
      if (p_len != c_len)
	mode = 0;
      else
	mode = 1;
    }
  else
    {
      mode = mode_guess;
    }

  if (mode == 0)
    {
      // Determine which sentence has the negation on it.

      unsigned char * not_sen, * oth_sen;
      int n_len;

      if (i == 0)
	return _("DeMorgan constructed incorrectly.");

      if (!strncmp (prem + i - 1, S_NOT, S_NL))
	{
	  not_sen = prem;  n_len = p_len;
	  oth_sen = conc;
	}
      else if (!strncmp (conc + i - 1, S_NOT, S_NL))
	{
	  not_sen = conc;  n_len = c_len;
	  oth_sen = prem;
	}
      else
	{
	  return _("There must be a negation at the difference.");
	}

      // Determine the scope, and remove the negation.

      int tmp_pos;
      unsigned char * tmp_str;

      tmp_pos = parse_parens (not_sen, i - 2, &tmp_str);
      if (tmp_pos == -2)
	return NULL;
      if (!tmp_str)
	return _("DeMorgan constructed incorrectly.");

      unsigned char * elim_sen;

      elim_sen = sexpr_elim_not (tmp_str);
      if (!elim_sen)
	return NULL;
      free (tmp_str);

      int gg;
      vec_t * gg_vec;
      unsigned char conn[S_CL + 1];

      gg_vec = init_vec (sizeof (char *));
      if (!gg_vec)
	return NULL;

      conn[0] = '\0';
      gg = sexpr_get_generalities (elim_sen, conn, gg_vec);
      if (gg == -1)
	return NULL;
      free (elim_sen);

      if (gg == 1)
	{
	  destroy_str_vec (gg_vec);
	  return _("There must be generalities on the negation sentence.");
	}

      if (strcmp (conn, S_AND) && strcmp (conn, S_OR))
	{
	  destroy_str_vec (gg_vec);
	  return _("The negation sentence must be negating either a conjunction or a disjunction.");
	}

      unsigned char oth_conn[S_CL + 1];
      int j;

      if (!strcmp (conn, S_AND))
	strcpy (oth_conn, S_OR);
      else
	strcpy (oth_conn, S_AND);

      // Construct what should be the other sentence.

      unsigned char * cons_sen;
      int cons_pos, alloc_size;

      alloc_size = n_len + (gg_vec->num_stuff - 1) * (S_NL + 3);
      cons_sen = (unsigned char *) calloc (alloc_size + 1, sizeof (char));
      CHECK_ALLOC (cons_sen, NULL);
      strncpy (cons_sen, not_sen, i - 1);
      cons_pos = i - 1;

      cons_pos += sprintf (cons_sen + cons_pos, "%s", oth_conn);

      for (j = 0; j < gg_vec->num_stuff; j++)
	{
	  cons_pos += sprintf (cons_sen + cons_pos, " (%s %s)",
			       S_NOT, vec_str_nth (gg_vec, j));
	}

      strcpy (cons_sen + cons_pos, not_sen + tmp_pos);

      destroy_str_vec (gg_vec);

      char * ret_str;
      if (not_sen == conc)
	ret_str = proc_dm (oth_sen, cons_sen, 0);
      else
	ret_str = proc_dm (cons_sen, oth_sen, 0);
      if (!ret_str)
	return NULL;

      free (cons_sen);
      if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
	return CORRECT;

      return _("DeMorgan constructed incorrectly.");
    }
  else
    {
      unsigned char * not_sen, * oth_sen;
      int n_len;

      if (!strncmp (prem + i, S_NOT, S_NL))
	{
	  not_sen = prem;  n_len = p_len;
	  oth_sen = conc;
	}
      else if (!strncmp (conc + i, S_NOT, S_NL))
	{
	  not_sen = conc;  n_len = c_len;
	  oth_sen = prem;
	}
      else
	{
	  return _("There must be a negation at the difference.");
	}

      int tmp_pos;
      unsigned char * tmp_str, * elim_sen;

      tmp_pos = parse_parens (not_sen, i - 1, &tmp_str);
      if (tmp_pos == -2)
	return NULL;
      if (!tmp_str)
	return _("DeMorgan constructed incorrectly.");

      elim_sen = sexpr_elim_not (tmp_str);
      if (!elim_sen)
	return NULL;
      free (tmp_str);

      if (elim_sen[1] != '(')
	return _("There must be a quantifier after the negation.");

      unsigned char * cmp_str;
      int oth_pos;
      unsigned char quant[S_CL + 1], oth_quant[S_CL + 1];
      unsigned char * var;

      cmp_str = sexpr_elim_quant (elim_sen, quant, &var);
      if (!cmp_str)
	return NULL;

      if (cmp_str[0] == '\0')
	return _("DeMorgan constructed incorrectly.");

      if (!strncmp (quant, S_UNV, S_CL))
	strcpy (oth_quant, S_EXL);
      else if (!strncmp (quant, S_EXL, S_CL))
	strcpy (oth_quant, S_UNV);
      else
	return _("There must be a quantifier after the negation.");

      // Construct what should be the other sentence.

      unsigned char * cons_sen;
      int cons_pos;

      cons_sen = (unsigned char *) calloc (n_len, sizeof (char));
      CHECK_ALLOC (cons_sen, NULL);
      strncpy (cons_sen, not_sen, i - 1);
      cons_pos = i - 1;
      cons_pos += sprintf (cons_sen + cons_pos, "((%s %s) (%s %s))",
			   oth_quant, var, S_NOT, cmp_str);
      free (cmp_str);
      free (var);

      strcpy (cons_sen + cons_pos, not_sen + tmp_pos + 1);

      char * ret_str;

      if (not_sen == conc)
	ret_str = proc_dm (oth_sen, cons_sen, 1);
      else
	ret_str = proc_dm (cons_sen, oth_sen, 1);
      if (!ret_str)
	return NULL;

      free (cons_sen);

      if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
	return CORRECT;

      return _("DeMorgan constructed incorrectly.");
    }
}

char *
proc_as (unsigned char * prem, unsigned char * conc)
{
  // At the difference:
  // rollback sh_sen;
  // sexpr_find_unmatched
  // run checks on the larger sentence.
  // strip the connective and recurse.

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

  int i;
  i = find_difference (ln_sen, sh_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  int li, si;

  si = li = i;
  if (sh_sen[si] == '(')
    si--;

  if (si < 0)
    return _("There must be a connective in both sentences.");

  si = find_unmatched_o_paren (sh_sen, si);
  if (si == -1)
    return _("There must be a connective in both sentences.");

  if (ln_sen[li] != '(')
    {
      li = find_unmatched_o_paren (ln_sen, li);
      if (li == -1)
	return _("There must be a connective in both sentences.");
    }

  int ret_chk;
  ret_chk = sexpr_find_unmatched (ln_sen, sh_sen, &li, &si);
  if (ret_chk == -1)
    return NULL;

  int tmp_pos;
  unsigned char * tmp_str;
  tmp_pos = parse_parens (ln_sen, li, &tmp_str);
  if (tmp_pos == -2)
    return NULL;
  free (tmp_str);

  int ti;
  ti = li - 1;
  if (ti < 0)
    return _("Association constructed incorrectly.");

  ti = find_unmatched_o_paren (ln_sen, ti);

  if (strncmp (ln_sen + ti + 1, S_AND, S_CL) && strncmp (ln_sen + ti + 1, S_OR, S_CL)
      && strncmp (ln_sen + ti + 1, ln_sen + li + 1, S_CL))
    {
      return _("Association must be done on a conjunction or disjunction.");
    }

  unsigned char * oth_sen;
  int oth_pos;

  oth_sen = (unsigned char *) calloc (l_len - 2 - S_CL, sizeof (char));
  CHECK_ALLOC (oth_sen, NULL);
  strncpy (oth_sen, ln_sen, li);
  oth_pos = li;

  strncpy (oth_sen + oth_pos, ln_sen + li + S_CL + 2, tmp_pos - li - 2 - S_CL);
  oth_pos += tmp_pos - li - 2 - S_CL;

  strcpy (oth_sen + oth_pos, ln_sen + tmp_pos + 1);

  char * ret_str;

  if (ln_sen == conc)
    ret_str = proc_as (sh_sen, oth_sen);
  else
    ret_str = proc_as (oth_sen, sh_sen);
  if (!ret_str)
    return NULL;

  free (oth_sen);
  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Association constructed incorrectly.");
}

char *
proc_co (unsigned char * prem, unsigned char * conc)
{
  int ret_chk;
  ret_chk = recurse_co (prem, conc);
  if (ret_chk == -1)
    return NULL;

  switch (ret_chk)
    {
    case 0:
      return CORRECT;
    case -3:
      return NO_DIFFERENCE;
    default:
      return _("Commutativity constructed incorrectly.");
    }
}

char *
proc_id (unsigned char * prem, unsigned char * conc)
{
  unsigned char * ln_sen, * sh_sen;
  int p_len, c_len;

  p_len = strlen (prem);
  c_len = strlen (conc);

  if (p_len < c_len)
    {
      sh_sen = prem;
      ln_sen = conc;
    }
  else
    {
      sh_sen = conc;
      ln_sen = prem;
    }

  int ret_chk;
  ret_chk = recurse_id (ln_sen, sh_sen);
  if (ret_chk == -1)
    return NULL;

  switch (ret_chk)
    {
    case 0:
      return CORRECT;
    case -3:
      return NO_DIFFERENCE;
    default:
      return _("Idempotence constructed incorrectly.");
    }
}

char *
proc_dt (unsigned char * prem, unsigned char * conc, int mode_guess)
{
  unsigned char * ln_sen, * sh_sen;
  int p_len, c_len, s_len;

  p_len = strlen (prem);
  c_len = strlen (conc);

  if (p_len < c_len)
    {
      sh_sen = prem;  s_len = p_len;
      ln_sen = conc;
    }
  else
    {
      sh_sen = conc;  s_len = c_len;
      ln_sen = prem;
    }

  int mode, i;

  i = find_difference (ln_sen, sh_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  if (p_len == c_len)
    return _("The two sentences must be of different lengths.");

  if (mode_guess == -1)
    {
      if (sh_sen[i] == '(')
	mode = 1;
      else
	mode = 0;
    }
  else
    {
      mode = mode_guess;
    }

  if (mode == 0)
    {
      int tmp_pos;
      unsigned char * tmp_str;

      if (i < 2
	  || (strncmp (sh_sen + i - 1, S_AND, S_CL)
	      &&  strncmp (sh_sen + i - 1, S_OR, S_CL)))
	{
	  return _("The top connective must change between sentences.");
	}

      tmp_pos = parse_parens (sh_sen, i - S_CL + 1, &tmp_str);
      if (tmp_pos == -2)
	return NULL;
      if (!tmp_str)
	return _("Distribution constructed incorrectly.");

      int ftc;
      unsigned char * lsen, * rsen, mc[S_CL + 1];
      
      mc[0] = '\0';

      ftc = sexpr_find_top_connective (tmp_str, mc, &lsen, &rsen);
      if (ftc == -1)
	return NULL;
      free (tmp_str);

      if (ftc < 0)
	{
	  if (lsen)  free (lsen);
	  if (rsen)  free (rsen);

	  return _("There must be only two parts for distribution.");
	}

      if (strcmp (mc, S_AND) && strcmp (mc, S_OR))
	{
	  free (lsen);
	  free (rsen);

	  return _("Distribution must be done around a conjunction or a disjunction.");
	}

      int r_gg, r_len;
      unsigned char rc[S_CL + 1];
      vec_t * rsens;

      r_len = strlen (rsen);
      rsens = init_vec (sizeof (char *));
      if (!rsens)
	return NULL;

      rc[0] = '\0';
      r_gg = sexpr_get_generalities (rsen, rc, rsens);
      if (r_gg == -1)
	return NULL;
      free (rsen);

      if (r_gg == 1)
	{
	  free (lsen);
	  destroy_str_vec (rsens);

	  return _("There must be more than one generality for distribution.");
	}

      unsigned char * oth_sen;
      int oth_pos, j, alloc_size;

      // length of lsen + S_CL (mc) + 4 (two spaces, two parens)
      // r_len - 1 (both of the parentheses, plus one for the null byte).
      alloc_size = s_len + (strlen (lsen) + S_CL + 4) * rsens->num_stuff + r_len - 1;
      oth_sen = (unsigned char *) calloc (alloc_size,  sizeof (char));
      CHECK_ALLOC (oth_sen, NULL);
      strncpy (oth_sen, sh_sen, i - 1);
      oth_pos = i - 1;

      oth_pos += sprintf (oth_sen + oth_pos, "%s", rc);

      for (j = 0; j < rsens->num_stuff; j++)
	{
	  oth_pos += sprintf (oth_sen + oth_pos, " (%s %s %s)",
			      mc, lsen, vec_str_nth (rsens, j));
	}

      strcpy (oth_sen + oth_pos, sh_sen + tmp_pos);

      free (lsen);
      destroy_str_vec (rsens);

      char * ret_str;

      if (sh_sen == conc)
	ret_str = proc_dt (ln_sen, oth_sen, 0);
      else
	ret_str = proc_dt (oth_sen, ln_sen, 0);

      if (!ret_str)
	return NULL;

      free (oth_sen);
      if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
	return CORRECT;

      return _("Distribution constructed incorrectly.");
    }
  else
    {
      unsigned char * tmp_str;
      int tmp_pos;

      if (mode_guess != -1)
	{
	  if (sh_sen[i] != '(')
	    return _("The difference must be a quantifier.");
	}

      tmp_pos = parse_parens (sh_sen, i - 1, &tmp_str);
      if (tmp_pos == -2)
	return NULL;

      unsigned char * scope, * var, quant[S_CL + 1];
      int v_len;

      scope = sexpr_elim_quant (tmp_str, quant, &var);
      if (!scope)
	return NULL;
      if (scope[0] == '\0')
	return _("Distribution constructed incorrectly.");
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
      if (gg == -1)
	return NULL;
      free (scope);

      if (gg == 1)
	{
	  destroy_str_vec (gg_vec);
	  return _("There must be generalities.");
	}

      if ((!strcmp (quant, S_UNV) && strcmp (conn, S_AND))
	  || (!strcmp (quant, S_EXL) && strcmp (conn, S_OR)))
	{
	  destroy_str_vec (gg_vec);
	  return _("The connective must be a conjunction or disjunction.");
	}

      unsigned char * oth_sen;
      int oth_pos, j, alloc_size;

      alloc_size = s_len + (gg_vec->num_stuff * (6 + S_CL + strlen (var)));
      oth_sen = (unsigned char *) calloc (alloc_size + 1, sizeof (char));
      CHECK_ALLOC (oth_sen, NULL);
      strncpy (oth_sen, sh_sen, i - 1);
      oth_pos = i - 1;

      oth_pos += sprintf (oth_sen + oth_pos, "(%s", conn);

      for (j = 0; j < gg_vec->num_stuff; j++)
	{
	  unsigned char * cur_gen, * str;
	  cur_gen = vec_str_nth (gg_vec, j);
	  oth_pos += sprintf (oth_sen + oth_pos, " ((%s %s) %s)",
			      quant, var, cur_gen);
	}

      oth_pos += sprintf (oth_sen + oth_pos, ")");

      strcpy (oth_sen + oth_pos, sh_sen + tmp_pos + 1);

      free (var);
      destroy_str_vec (gg_vec);

      char * ret_str;
      if (sh_sen == conc)
	ret_str = proc_dt (ln_sen, oth_sen, 1);
      else
	ret_str = proc_dt (oth_sen, ln_sen, 1);
      if (!ret_str)
	return NULL;

      free (oth_sen);
      if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
	return CORRECT;

      return _("Distribution constructed incorrectly.");      
    }
}

char *
proc_eq (unsigned char * prem, unsigned char * conc)
{
  unsigned char * bic_sen, * oth_sen;
  int p_len, c_len, b_len;

  c_len = strlen (conc);
  p_len = strlen (prem);

  if (c_len < p_len)
    {
      bic_sen = conc;  b_len = c_len;
      oth_sen = prem;
    }
  else
    {
      bic_sen = prem;  b_len = p_len;
      oth_sen = conc;
    }

  int i;
  i = find_difference (bic_sen, oth_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  if (i < 2)
    return _("Equivalence constructed incorrectly.");

  int tmp_pos;
  unsigned char * tmp_str;

  tmp_pos = parse_parens (bic_sen, i - 2, &tmp_str);
  if (tmp_pos == -2)
    return NULL;
  if (!tmp_str)
    return _("Equivalence constructed incorrectly.");

  int ftc;
  unsigned char * lsen, * rsen;

  lsen = rsen = NULL;
  ftc = sexpr_find_top_connective (tmp_str, S_BIC, &lsen, &rsen);
  if (ftc == -1)
    return NULL;

  if (ftc < 0)
    {
      if (lsen)  free (lsen);
      if (rsen)  free (rsen);

      return _("There must be a biconditional in one sentence.");
    }

  unsigned char * cons_sen;
  int cons_pos, alloc_size;

  alloc_size = b_len + (tmp_pos - (i - 2) + 1) + S_CL + 4;
  cons_sen = (unsigned char *) calloc (alloc_size + 1, sizeof (char));
  CHECK_ALLOC (cons_sen, NULL);
  strncpy (cons_sen, bic_sen, i - 1);
  cons_pos = i - 1;

  cons_pos += sprintf (cons_sen, "(%s (%s %s %s) (%s %s %s))",
		       S_AND, S_CON, lsen, rsen, S_CON, rsen, lsen);
  free (lsen);
  free (rsen);

  strcpy (cons_sen + cons_pos, bic_sen + tmp_pos);

  char * ret_str;

  if (bic_sen == conc)
    ret_str = proc_eq (oth_sen, cons_sen);
  else
    ret_str = proc_eq (cons_sen, oth_sen);

  free (cons_sen);

  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Equivalence constructed incorrectly.");
}

char *
proc_dn (unsigned char * prem, unsigned char * conc)
{
  // Standard long-short setup.

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

  int i;
  i = find_difference (ln_sen, sh_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  if (ln_sen[i] != '(' && strncmp (ln_sen + i, S_NOT, S_NL)
      && (i > 0 && l_len > S_NL && strncmp (ln_sen + i - 1, S_NOT, S_NL)))
    {
      return _("The difference must be at a negation.");
    }

  if (ln_sen[i] != '(')
    {
      if (!strncmp (ln_sen + i, S_NOT, S_NL))
	i--;
      else
	i -= 2;
    }

  int tmp_pos;
  unsigned char * tmp_str;
  tmp_pos = parse_parens (ln_sen, i, &tmp_str);
  if (tmp_pos == -2)
    return NULL;
  if (!tmp_str)
    return _("Double Negation constructed incorrectly.");

  int pos, num_negs;
  unsigned char * oth_sen;
  int oth_pos;

  pos = i;
  if (!sexpr_not_check (tmp_str) || strncmp (tmp_str + S_NL + 3, S_NOT, S_NL))
    {
      free (tmp_str);
      return _("There must be at least two negations at the difference.");
    }

  free (tmp_str);
  pos = i + 4 + 2 * S_NL;
  // Begin removing pairs of negations, until there are no more pairs.

  oth_sen = (unsigned char *) calloc (l_len - S_NL * 2 - 5, sizeof (char));
  CHECK_ALLOC (oth_sen, NULL);
  strncpy (oth_sen, ln_sen, i);
  oth_pos = i;

  strncpy (oth_sen + oth_pos, ln_sen + pos, tmp_pos - pos - 1);
  oth_pos += tmp_pos - pos - 1;

  strcpy (oth_sen + oth_pos, ln_sen + tmp_pos + 1);

  char * ret_str;

  if (ln_sen == conc)
    ret_str = proc_dn (sh_sen, oth_sen);
  else
    ret_str = proc_dn (oth_sen, sh_sen);

  if (!ret_str)
    return NULL;

  free (oth_sen);
  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Double Negation constructed incorrectly.");
}

char *
proc_ep (unsigned char * prem, unsigned char * conc)
{
  unsigned char * and_sen, * con_sen;
  int p_len, c_len, a_len;

  p_len = strlen (prem);
  c_len = strlen (conc);

  // Count number of conditionals in each sentence.

  int i, c_cons, p_cons;

  for (i = 0; i < p_len; i++)
    {
      if (!strncmp (prem + i, S_CON, S_CL))
	p_cons++;

      if (!strncmp (conc + i, S_CON, S_CL))
	c_cons++;
    }

  if (c_cons > p_cons)
    {
      and_sen = prem;  a_len = p_len;
      con_sen = conc;
    }
  else
    {
      and_sen = conc;  a_len = c_len;
      con_sen = prem;
    }

  i = find_difference (and_sen, con_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  int ai, ci;

  ai = ci = i;

  if (con_sen[ci] == '(')
    {
      ci--;
      if (ci == -1)
	return _("Exportation constructed incorrectly.");
    }

  ci = find_unmatched_o_paren (con_sen, ci);
  if (ci == -1)
    return _("There must be connectives on both sentences.");

  if (and_sen[ai] != '(')
    {
      ai = find_unmatched_o_paren (and_sen, ai);
      if (ai == -1)
	return _("There must be connectives on both sentences.");
    }

  int ret_chk;
  ret_chk = sexpr_find_unmatched (and_sen, con_sen, &ai, &ci);
  if (ret_chk == -1)
    return NULL;

  ai--;
  if (ai < 0)
    return _("Exportation constructed incorrectly.");

  ai = find_unmatched_o_paren (and_sen, ai);
  if (ai == -1)
    return _("Exportation constructed incorrectly.");
  
  int tmp_pos;
  unsigned char * tmp_str;

  tmp_pos = parse_parens (and_sen, ai, &tmp_str);
  if (tmp_pos == -2)
    return NULL;

  int ftc;
  unsigned char * lsen, * rsen;

  lsen = rsen = NULL;
  ftc = sexpr_find_top_connective (tmp_str, S_CON, &lsen, &rsen);
  if (ftc == -1)
    return NULL;
  free (tmp_str);

  if (ftc < 0)
    {
      if (lsen)  free (lsen);
      if (rsen)  free (rsen);

      return _("There must be a conditional in both sentences.");
    }

  vec_t * a_sens;

  a_sens = init_vec (sizeof (char *));
  if (!a_sens)
    return NULL;

  ftc = sexpr_get_generalities (lsen, S_AND, a_sens);
  if (ftc == -1)
    return NULL;
  free (lsen);

  if (ftc == 1)
    {
      destroy_str_vec (a_sens);
      free (rsen);
      return _("There must be a conjunctions in the conjunction sentence.");
    }

  unsigned char * oth_sen;
  int oth_pos, alloc_size;

  alloc_size = a_len + (3 + S_CL) * a_sens->num_stuff;
  oth_sen = (unsigned char *) calloc (alloc_size + 1, sizeof (char));
  CHECK_ALLOC (oth_sen, NULL);
  strncpy (oth_sen, and_sen, ai);
  oth_pos = ai;

  for (i = 0; i < a_sens->num_stuff; i++)
    {
      oth_pos += sprintf (oth_sen + oth_pos, "(%s %s ",
			  S_CON, vec_str_nth (a_sens, i));
    }

  oth_pos += sprintf (oth_sen + oth_pos, "%s", rsen);
  for (i = 0; i < a_sens->num_stuff; i++)
    oth_pos += sprintf (oth_sen + oth_pos, ")");

  destroy_str_vec (a_sens);
  free (rsen);

  strcpy (oth_sen + oth_pos, and_sen + tmp_pos + 1);

  char * ret_str;
  if (and_sen == conc)
    ret_str = proc_ep (con_sen, oth_sen);
  else
    ret_str = proc_ep (oth_sen, con_sen);
  if (!ret_str)
    return NULL;
  free (oth_sen);

  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Exportation constructed incorrectly.");
}

char *
proc_sb (unsigned char * prem, unsigned char * conc)
{
  // Standard long-short setup.

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

  int i;
  i = find_difference (ln_sen, sh_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  // Standard find_unmatched setup.

  int li, si;

  si = li = i;
  if (sh_sen[si] == '(')
    {
      si--;
      if (si < 0)
	return _("Subsumption constructed incorrectly.");
    }
  else if (si > 0
	   && (!strncmp (sh_sen + si - 1, S_AND, S_CL)
	       || !strncmp (sh_sen + si - 1, S_OR, S_CL)
	       || !strncmp (sh_sen + si - 1, S_CON, S_CL)
	       || !strncmp (sh_sen + si - 1, S_BIC, S_CL)))
    {
      while (sh_sen[si] != '(')
	si--;
      si--;
    }

  si = find_unmatched_o_paren (sh_sen, si);
  if (si == -1)
    {
      // This is the entire sentence.
      li = 0;
    }
  else
    {
      if (ln_sen[li] != '(')
	li = find_unmatched_o_paren (ln_sen, li);
      if (li < 0)
	return _("Subsumption constructed incorrectly.");

      int ret_chk;
      ret_chk = sexpr_find_unmatched (ln_sen, sh_sen, &li, &si);
      if (ret_chk == -1)
	return NULL;

      if (li < 0)
	return _("Subsumption constructed incorrectly.");
    }

  if (ln_sen[li] != '(')
    return _("There must be connectives on both sentences.");

  int tmp_pos;
  unsigned char * tmp_str;

  tmp_pos = parse_parens (ln_sen, li, &tmp_str);
  if (tmp_pos == -2)
    return NULL;
  if (!tmp_str)
    return _("Subsumption constructed incorrectly.");

  int ftc;
  unsigned char * t_lsen, * t_rsen, tconn[S_CL + 1];

  tconn[0] = '\0';
  t_lsen = t_rsen = NULL;
  ftc = sexpr_find_top_connective (tmp_str, tconn, &t_lsen, &t_rsen);
  if (ftc == -1)
    return NULL;

  free (tmp_str);

  if (ftc < 0)
    {
      if (t_lsen)  free (t_lsen);
      if (t_rsen)  free (t_rsen);

      return _("Subsumption must be done around a connective.");
    }

  if (strcmp (tconn, S_AND) && strcmp (tconn, S_OR))
    {
      free (t_lsen);
      free (t_rsen);

      return _("Subsumption must be done around a disjunction or a conjunction.");
    }

  int r_len;
  unsigned char * lsen, * rsen, conn[S_CL + 1];

  r_len = strlen (t_rsen);
  conn[0] = '\0';
  lsen = rsen = NULL;
  ftc = sexpr_find_top_connective (t_rsen, conn, &lsen, &rsen);
  if (ftc == -1)
    return NULL;
  free (t_rsen);
  if (rsen)  free (rsen);

  if (ftc < 0)
    {
      free (t_lsen);
      if (lsen)  free (lsen);

      return _("Subsumption must be done around two connectives.");
    }

  if ((!strcmp (tconn, S_AND) && strcmp (conn, S_OR))
      || (!strcmp (tconn, S_OR) && strcmp (conn, S_AND)))
    {
      free (t_lsen);
      free (lsen);

      return _("The two connectives must be complementary to one another.");
    }

  if (strcmp (t_lsen, lsen))
    {
      free (t_lsen);
      free (lsen);

      return _("Both of the left sentences must be the same.");
    }
  free (lsen);

  unsigned char * oth_sen;
  int oth_pos;

  oth_sen = (unsigned char *) calloc (l_len - r_len - S_CL - 3, sizeof (char));
  CHECK_ALLOC (oth_sen, NULL);
  strncpy (oth_sen, ln_sen, li);
  oth_pos = li;

  strcpy (oth_sen + oth_pos, t_lsen);
  oth_pos += strlen (t_lsen);
  free (t_lsen);

  strcpy (oth_sen + oth_pos, ln_sen + tmp_pos + 1);

  char * ret_str;

  if (ln_sen == conc)
    ret_str = proc_sb (sh_sen, oth_sen);
  else
    ret_str = proc_sb (oth_sen, sh_sen);

  if (!ret_str)
    return NULL;

  free (oth_sen);
  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Subsumption constructed incorrectly.");
}

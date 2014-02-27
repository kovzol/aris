/* Functions for processing boolean rules.

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

#include "sexpr-process.h"
#include "vec.h"

// Boolean rules follow the same structure as equivalence rules.

char *
process_bool (unsigned char * conc, vec_t * prems, const char * rule)
{
  char * ret = NOT_MINE;
  unsigned char ** prem;

  if (!strcmp (rule, "bi"))
    {
      if (prems->num_stuff != 1)
	return _("Boolean Identity requires one (1) reference.");

      prem = vec_nth (prems, 0);
      ret = proc_bi (*prem, conc);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "bd"))
    {
      if (prems->num_stuff != 1)
	return _("Boolean Domination requires one (1) reference.");

      prem = vec_nth (prems, 0);
      ret = proc_bd (*prem, conc);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "bn"))
    {
      if (prems->num_stuff != 1)
	return _("Boolean Negation requires one (1) reference.");

      prem = vec_nth (prems, 0);
      ret = proc_bn (*prem, conc);
      if (!ret)
	return NULL;
    }

  if (!strcmp (rule, "sn"))
    {
      if (prems->num_stuff != 1)
	return _("Symbol Negation require one (1) reference.");

      prem = vec_nth (prems, 0);
      ret = proc_sn (*prem, conc);
      if (!ret)
	return NULL;
    }

  return ret;
}

char *
proc_bi (unsigned char * prem, unsigned char * conc)
{
  // Standard long-short sentence setup.

  unsigned char * ln_sen, * sh_sen;
  int l_len;

  sen_put_len (prem, conc, &sh_sen, &ln_sen);
  l_len = strlen (ln_sen);

  int i;
  i = find_difference (ln_sen, sh_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  int li, si;

  // Rollback sh_sen to just before the first o-paren.
  si = li = i;
  if (sh_sen[si] == '(')
    {
      si--;
      if (si < 0)
	return _("Boolean Identity constructed incorrectly.");
    }
  else if (si > 0
	   && (!strncmp (sh_sen + si - 1, S_AND, S_CL)
	       || !strncmp (sh_sen + si - 1, S_OR, S_CL)
	       || !strncmp (sh_sen + si - 1, S_CON, S_CL)
	       || !strncmp (sh_sen + si - 1, S_BIC, S_CL)
	       || !strncmp (sh_sen + si - 1, S_NOT, S_NL)))
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
	{
	  li = find_unmatched_o_paren (ln_sen, li);
	  if (li == -1)
	    return _("There must be a connective in the longer sentence.");
	}

      int ret_chk;
      ret_chk = sexpr_find_unmatched (ln_sen, sh_sen, &li, &si);
      if (ret_chk == -1)
	return NULL;

      if (li < 0)
	return _("Boolean Identity constructed incorrectly.");

      if (si < 0)
	{
	  // The equivalence uses all of sh_sen.
	  // this means that li needs to be zero.
	  li = 0;
	}
    }

  if (ln_sen[li] != '(')
    return _("Boolean Identity constructed incorrectly.");

  int tmp_pos;
  unsigned char * tmp_str;

  tmp_pos = parse_parens (ln_sen, li, &tmp_str);
  if (tmp_pos == ERROR_CODE_MEMORY)
    return NULL;

  int ftc;
  unsigned char * lsen, * rsen, conn[S_CL + 1];

  conn[0] = '\0';
  lsen = rsen = NULL;

  ftc = sexpr_find_top_connective (tmp_str, conn, &lsen, &rsen);
  if (ftc == -1)
    return NULL;
  free (tmp_str);

  if (ftc < 0)
    {
      if (lsen)  free (lsen);
      if (rsen)  free (rsen);

      return _("There must be generalities at the difference.");
    }

  if (strcmp (conn, S_AND) && strcmp (conn, S_OR))
    {
      free (lsen);
      free (rsen);

      return _("There must be a conjunction or a disjunction in one sentence.");
    }

  unsigned char * good_side, * bad_side;

  if (ISSBOOL(lsen))
    {
      good_side = lsen;
      bad_side = rsen;
    }
  else if (ISSBOOL(rsen))
    {
      good_side = rsen;
      bad_side = lsen;
    }
  else
    {
      free (rsen);
      free (lsen);
      return _("There must be a tautology or a contradiction in the generalities.");
    }

  if ((strcmp (conn, S_AND) || strcmp (good_side, S_TAU))
      && (strcmp (conn, S_OR) || strcmp (good_side, S_CTR)))
    {
      free (lsen);
      free (rsen);

      return _("A tautology must be matched with a conjunction, and a contradiction \
with a disjunction.");
    }
  free (good_side);

  unsigned char * oth_sen;
  int alloc_size;

  alloc_size = l_len - 2 * S_CL - 4;
  oth_sen = construct_other (ln_sen, li, tmp_pos + 1, alloc_size, "%s", bad_side);
  if (!oth_sen)
    return NULL;

  char * ret_str;

  if (ln_sen == conc)
    ret_str = proc_bi (sh_sen, oth_sen);
  else
    ret_str = proc_bi (oth_sen, sh_sen);

  if (!ret_str)
    return NULL;
  free (oth_sen);

  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Boolean Identity constructed incorrectly.");
}

char *
proc_bd (unsigned char * prem, unsigned char * conc)
{
  unsigned char * ln_sen, * sh_sen;
  int l_len;

  sen_put_len (prem, conc, &sh_sen, &ln_sen);
  l_len = strlen (ln_sen);

  int i;
  i = find_difference (ln_sen, sh_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  if (ln_sen[i] != '(')
    return _("There must be a connective at the difference.");

  int tmp_pos, t_len;
  unsigned char * tmp_str;

  tmp_pos = parse_parens (ln_sen, i, &tmp_str);
  if (tmp_pos == ERROR_CODE_MEMORY)
    return NULL;
  t_len = strlen (tmp_str);

  int gg;
  vec_t * gens;
  unsigned char conn[S_CL + 1];

  conn[0] = '\0';
  gens = init_vec (sizeof (char *));
  if (!gens)
    return NULL;

  gg = sexpr_get_generalities (tmp_str, conn, gens);
  if (gg == -1)
    return NULL;
  free (tmp_str);

  if (gg == 1)
    {
      destroy_str_vec (gens);
      return _("There must be a connective at the difference.");
    }

  if (strcmp (conn, S_AND) && strcmp (conn, S_OR))
    {
      destroy_str_vec (gens);
      return _("There must be a conjunction or a disjunction at the difference.");
    }

  unsigned char sym[S_CL + 1];
  int j;

  for (j = 0; j < gens->num_stuff; j++)
    {
      unsigned char ** cur_gen;
      cur_gen = vec_nth (gens, j);

      if (!strcmp (*cur_gen, S_CTR) && !strcmp (conn, S_AND))
	{
	  strncpy (sym, S_CTR, S_CL);
	  sym[S_CL] = '\0';
	  break;
	}

      if (!strcmp (*cur_gen, S_TAU) && !strcmp (conn, S_OR))
	{
	  strncpy (sym, S_TAU, S_CL);
	  sym[S_CL] = '\0';
	  break;
	}
    }

  destroy_str_vec (gens);

  if (j == gg)
    return _("There must be a tautology or a contradiction within the difference.");

  unsigned char * oth_sen;
  int alloc_size;
  alloc_size = l_len - t_len + S_CL;

  oth_sen = construct_other (ln_sen, i, tmp_pos + 1, alloc_size, "%s", sym);
  if (!oth_sen)
    return NULL;

  char * ret_str;

  if (ln_sen == conc)
    ret_str = proc_bd (sh_sen, oth_sen);
  else
    ret_str = proc_bd (oth_sen, sh_sen);

  if (!ret_str)
    return NULL;
  free (oth_sen);

  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Boolean Dominance constructed incorrectly.");
}

char *
proc_bn (unsigned char * prem, unsigned char * conc)
{
  unsigned char * ln_sen, * sh_sen;
  int l_len;

  sen_put_len (prem, conc, &sh_sen, &ln_sen);
  l_len = strlen (ln_sen);

  int i;
  i = find_difference (ln_sen, sh_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  if (ln_sen[i] != '(' ||
      (strncmp (ln_sen + i + 1, S_AND, S_CL) && strncmp (ln_sen + i + 1, S_OR, S_CL)))
    return _("There must be a connective at the difference.");

  int tmp_pos;
  unsigned char * tmp_str;

  tmp_pos = parse_parens (ln_sen, i, &tmp_str);
  if (tmp_pos == ERROR_CODE_MEMORY)
    return NULL;

  int ftc;
  unsigned char * lsen, * rsen;
  unsigned char conn[S_CL + 1];
  conn[0] = '\0';

  ftc = sexpr_find_top_connective (tmp_str, conn, &lsen, &rsen);
  if (ftc == -1)
    return NULL;
  free (tmp_str);

  if (ftc < 0)
    {
      if (lsen)  free (lsen);
      if (rsen)  free (rsen);

      return _("There must be only two connected parts.");
    }

  if (!sexpr_not_check (rsen) || strcmp (sexpr_add_not (lsen), rsen))
    {
      free (lsen);
      free (rsen);

      return _("The second part must be the negation of the first.");
    }

  unsigned char sym[S_CL + 1];
  if (!strcmp (conn, S_AND))
    strncpy (sym, S_CTR, S_CL);
  else
    strncpy (sym, S_TAU, S_CL);
  sym[S_CL] = '\0';

  free (rsen);

  unsigned char * oth_sen;
  int alloc_size;

  alloc_size = l_len - 2 * strlen (lsen) - S_NL - 7;
  free (lsen);

  oth_sen = construct_other (ln_sen, i, tmp_pos + 1, alloc_size, "%s", sym);
  if (!oth_sen)
    return NULL;

  char * ret_str;

  if (ln_sen == conc)
    ret_str = proc_bn (sh_sen, oth_sen);
  else
    ret_str = proc_bn (oth_sen, sh_sen);

  if (!ret_str)
    return NULL;

  free (oth_sen);
  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Boolean Negation constructed incorrectly.");
}

char *
proc_sn (unsigned char * prem, unsigned char * conc)
{
  unsigned char * ln_sen, * sh_sen;
  int l_len;

  sen_put_len (prem, conc, &sh_sen, &ln_sen);
  l_len = strlen (ln_sen);

  int i;
  i = find_difference (ln_sen, sh_sen);
  if (i == -1)
    return NO_DIFFERENCE;

  if (ln_sen[i] != '(' || strncmp (ln_sen + i + 1, S_NOT, S_NL))
    return _("There must be a negation in one sentence.");

  int tmp_pos;
  unsigned char * tmp_str, * elm_str;

  tmp_pos = parse_parens (ln_sen, i, &tmp_str);
  if (tmp_pos == ERROR_CODE_MEMORY)
    return NULL;

  elm_str = sexpr_elim_not (tmp_str);
  if (!elm_str)
    return NULL;
  free (tmp_str);

  if (strcmp (elm_str, S_CTR) && strcmp (elm_str, S_TAU))
    {
      free (elm_str);
      return _("There must be a negated symbol in one sentence.");
    }

  unsigned char sym[S_CL + 1];
  if (!strcmp (elm_str, S_CTR))
    strncpy (sym, S_TAU, S_CL);
  else
    strncpy (sym, S_CTR, S_CL);
  sym[S_CL] = '\0';
  free (elm_str);

  unsigned char * oth_sen;
  int alloc_size;

  alloc_size = l_len - S_NL - 3;
  oth_sen = construct_other (ln_sen, i, tmp_pos + 1, alloc_size, "%s", sym);
  if (!oth_sen)
    return NULL;

  char * ret_str;

  if (ln_sen == conc)
    ret_str = proc_sn (sh_sen, oth_sen);
  else
    ret_str = proc_sn (oth_sen, sh_sen);
  free (oth_sen);

  if (!ret_str)
    return NULL;

  if (ret_str == NO_DIFFERENCE || ret_str == CORRECT)
    return CORRECT;

  return _("Symbol Negation constructed incorrectly.");
}

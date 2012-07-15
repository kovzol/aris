/* Proof data type functions.

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
#include <stdio.h>

#include "proof.h"
#include "sen-data.h"
#include "var.h"
#include "list.h"
#include "vec.h"
#include "rules.h"
#include "process.h"
#include "sexpr-process.h"

/* Initializes a proof.
 *  input:
 *    none.
 *  output:
 *    the newly initialized proof, or NULL on error.
 */
proof_t *
proof_init ()
{
  proof_t * pf = (proof_t *) calloc (1, sizeof (proof_t));
  if (!pf)
    {
      perror (NULL);
      return NULL;
    }

  pf->everything = init_list ();
  if (!pf->everything)
    return NULL;

  pf->goals = init_list ();
  if (!pf->goals)
    return NULL;

  return pf;
}

/* Destroys a proof.
 *  input:
 *    proof - the proof to destroy.
 *  output:
 *    none.
 */
void
proof_destroy (proof_t * proof)
{
  item_t * itm, * n_itm;

  for (itm = proof->everything->head; itm != NULL; itm = n_itm)
    {
      item_t * n_itm;
      n_itm = itm->next;
      free (itm);
    }

  itm = proof->goals->head;
  for (itm = proof->goals->head; itm != NULL; itm = n_itm)
    {
      item_t * n_itm;
      n_itm = itm->next;
      free (itm->value);
    }
}

/* Evaluates a proof object.
 *  input:
 *    proof - The proof that is being evaluated.
 *    rets - A vector to store the lines that correctly evaluated.
 *    verbose - A flag denoting verbosity.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
proof_eval (proof_t * proof, vec_t * rets, int verbose)
{
  item_t * sen_itr;
  int got_prems, cur_line, num_correct;
  list_t * pf_vars;
  vec_t * sexpr_text;
  int ret;

  got_prems = 0;
  num_correct = cur_line = 0;

  pf_vars = init_list ();
  if (!pf_vars)
    return -1;

  sexpr_text = init_vec (sizeof (char *));
  if (!sexpr_text)
    return -1;

  for (sen_itr = proof->everything->head; sen_itr; sen_itr = sen_itr->next)
    {
      sen_data * sd;
      sd = sen_itr->value;

      ret = check_text (sd->text);
      if (ret == -2)
	return -1;

      if (ret != 0)
	continue;

      unsigned char * tmp_str, * sexpr;
      tmp_str = die_spaces_die (sd->text);
      if (!tmp_str)
	return -1;

      sexpr = convert_sexpr (tmp_str);
      if (!sexpr)
	return -1;
      sd->sexpr = sexpr;
      free (tmp_str);
    }

  for (sen_itr = proof->everything->head; sen_itr != NULL;
       sen_itr = sen_itr->next)
    {
      // ln | text
      //---------------------
      // ln | text [rule <file> refs]
      cur_line++;
      sen_data * sd;
      sd = sen_itr->value;

      char * ret_chk;
      int ret_val;
      ret_chk = sen_data_evaluate (sd, &ret_val, pf_vars,
				   proof->everything);

      if (!ret_chk)
	return -1;

      if (verbose)
	{
	  if (sd->premise)
	    {
	      printf (" %3i | %s\n", cur_line, sd->text);
	    }

	  if (!sd->premise)
	    {
	      if (got_prems)
		{
		  if (verbose)
		    printf ("----------------\n");
		  got_prems = 0;
		}

	      printf (" %3i | %s %s", cur_line, sd->text,
		      rules_list[sd->rule]);
	      if (sd->rule == RULE_LM)
		printf  (":%s", sd->file);

	      printf (" ");

	      int j;
	      for (j = 0; sd->refs[j] != -1; j++)
		{
		  printf ("%i", sd->refs[j]);
		  if (sd->refs[j + 1] != -1)
		    printf (",");
		}

	      printf ("\n");
	    }
	}

      if (!strcmp (ret_chk, CORRECT))
	{
	  num_correct++;
	  if (rets)
	    {
	      ret = vec_add_obj (rets, &cur_line);
	      if (ret < 0)
		return -1;
	    }

	  if (verbose)
	    printf ("%i: %s\n", sd->line_num, ret_chk);
	}

      int arb = (sd->premise || sd->rule == RULE_EI || sd->subproof) ? 0 : 1;
      ret = sexpr_collect_vars_to_proof (pf_vars, sd->sexpr, arb);
      if (ret < 0)
	return -1;
    }

  return 0;
}

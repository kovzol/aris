/* Proof data type functions.

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
  item_t * itm;

  for (itm = proof->everything->head; itm != NULL;)
    {
      item_t * n_itm;
      n_itm = itm->next;
      free (itm);
      itm = n_itm;
    }

  for (itm = proof->goals->head; itm != NULL;)
    {
      item_t * n_itm;
      n_itm = itm->next;
      free (itm->value);
      itm = n_itm;
    }
}

/* Evaluates a proof object.
 *  input:
 *    proof - The proof that is being evaluated.
 *    rets - A vector to store the return values.
 *    verbose - A flag denoting verbosity.
 *  output:
 *    0 on success, -1 on memory error.
 */
int
proof_eval (proof_t * proof, vec_t * rets, int verbose)
{
  int rc;
  rc = eval_proof (proof->everything, rets, verbose);

  return rc;
}

/* Evaluates a list of sentences.
 *  input:
 *    everything - the list of sentences to evaluate.
 *    rets - a vector in which to store the return values.
 *    verbose - a flag denoting verbosity (1 if verbose).
 *  output:
 *    0 on success, -1 on memory error.
 */
int
eval_proof (list_t * everything, vec_t * rets, int verbose)
{
  item_t * sen_itr;
  int got_prems, cur_line;
  list_t * pf_vars;
  vec_t * sexpr_text;
  int ret;

  got_prems = 0;
  cur_line = 0;

  pf_vars = init_list ();
  if (!pf_vars)
    return -1;

  sexpr_text = init_vec (sizeof (char *));
  if (!sexpr_text)
    return -1;

  for (sen_itr = everything->head; sen_itr; sen_itr = sen_itr->next)
    {
      sen_data * sd;
      sd = sen_itr->value;

      ret = sd_convert_sexpr (sd);
      if (ret == -1)
	return -1;
      if (ret == -2)
	continue;
    }

  for (sen_itr = everything->head; sen_itr != NULL;
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
				   everything);

      if (!ret_chk)
	return -1;

      if (verbose)
	{
	  if (sd->premise)
	    printf (" %3i | %s\n", cur_line, sd->text);

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
	      for (j = 0; sd->refs[j] != REF_END; j++)
		{
		  printf ("%i", sd->refs[j]);
		  if (sd->refs[j + 1] != REF_END)
		    printf (",");
		}

	      printf ("\n");
	    }
	}

      if (rets)
	{
	  ret = vec_str_add_obj (rets, ret_chk);
	  if (ret == -1)
	    return -1;
	}
      if (verbose)
	printf ("%i: %s\n", sd->line_num, ret_chk);

      int arb = (sd->premise || sd->rule == RULE_EI || sd->subproof) ? 0 : 1;
      if (sd->sexpr)
	{
	  ret = sexpr_collect_vars_to_proof (pf_vars, sd->sexpr, arb);
	  if (ret < 0)
	    return -1;
	}
    }

  return 0;
}

/* Converts a proof into a LaTeX file.
 *  input:
 *   proof - the proof to convert.
 *   file - the file to write to.
 *  output:
 *   0 on success, -1 on error.
 */
int
convert_proof_latex (proof_t * proof, const char * filename)
{
  FILE * file;

  file = fopen (filename, "w");
  if (!file)
    {
      perror ("proof.c: ");
      exit (EXIT_FAILURE);
    }

  fprintf (file, "\\documentclass{article}\n");
  fprintf (file, "\\usepackage{amsmath}\n");
  fprintf (file, "\\usepackage{amsfonts}\n");
  fprintf (file, "\\usepackage{longtable}\n");
  fprintf (file, "\\usepackage[cm]{fullpage}\n");
  fprintf (file, "\\begin{document}\n");
  fprintf (file, "\\newcommand{\\eline}{--------}\n");
  fprintf (file, "\\newcommand{\\prmline}[2]{#1.&$#2$&}\n");
  fprintf (file, "\\newcommand{\\stdline}[3]{#1.&$#2$&\\texttt{#3 }}\n");
  fprintf (file, "\\newcommand{\\pquad}{| \\;}\n");
  fprintf (file, "\\newcommand{\\spquad}{\\text{ } \\quad}\n");
  fprintf (file, "\n");

  fprintf (file, "\\begin{longtable}{r|p{14.5cm}|l}\n");

  item_t * ev_itr;
  char * text;
  int i;

  for (ev_itr = proof->everything->head; ev_itr; ev_itr = ev_itr->next)
    {
      sen_data * sd;
      sd = ev_itr->value;

      if (!sd->premise)
	break;

      text = convert_sd_latex (sd);
      if (!text)
	return -1;

      fprintf (file, "\t\\prmline{%i}{%s}\\\\\n", sd->line_num, text);
      free (text);
    }

  fprintf (file, "\t\\hline\n");

  for (; ev_itr; ev_itr = ev_itr->next)
    {
      sen_data * sd;
      sd = ev_itr->value;

      text = convert_sd_latex (sd);
      if (!text)
	return -1;

      const char * rule = sd->subproof ? "assume" : rules_list[sd->rule];

      fprintf (file, "\t\\stdline{%i}{%s}{%s} ", sd->line_num,
	       text, rule);

      if (!(sd->rule == RULE_EX || sd->rule == RULE_II
	    || sd->rule == RULE_SQ || sd->subproof))
	{
	  fprintf (file, "(");
	  for (i = 0; sd->refs[i] != REF_END; i++)
	    {
	      fprintf (file, "%i", sd->refs[i]);
	      if (sd->refs[i + 1] != REF_END)
		fprintf (file, ",");
	    }

	  fprintf (file, ")");
	}

      fprintf (file, "\\\\\n");
      free (text);

      if (sd->subproof)
	{
	  fprintf (file, "&$");
	  for (i = 0; i < sd->depth; i++)
	    fprintf (file, "\\pquad ");
	  fprintf (file, "\\eline&\\\\\n");
	}
    }

  fprintf (file, "\\end{longtable}\n");

  fprintf (file, "\\end{document}\n");

  fclose (file);

  return 0;
}

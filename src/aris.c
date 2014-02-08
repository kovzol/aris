/* The GNU Aris program.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "process.h"
#include "vec.h"
#include "list.h"
#include "var.h"
#include "sen-data.h"
#include "proof.h"
#include "aio.h"
#include "rules.h"
#include "config.h"
#include "interop-isar.h"
#include "menu.h"

#ifdef ARIS_GUI
#include <gtk/gtk.h>
#include "app.h"
#include "aris-proof.h"
#include "rules-table.h"
#endif

// The options array for getopt_long.

static struct option const long_opts[] =
  {
    {"evaluate", no_argument, NULL, 'e'},
    {"premise", required_argument, NULL, 'p'},
    {"conclusion", required_argument, NULL, 'c'},
    {"rule", required_argument, NULL, 'r'},
    {"variable", required_argument, NULL, 'a'},
    {"text", no_argument, NULL, 't'},
    {"file", required_argument, NULL, 'f'},
    {"grade", no_argument, NULL, 'g'},
    {"isar", required_argument, NULL, 'i'},
    {"sexpr", required_argument, NULL, 's'},
    {"boolean", no_argument, NULL, 'b'},
    {"list", no_argument, NULL, 'l'},
    {"verbose", no_argument, NULL, 'v'},
    {"latex", required_argument, NULL, 'x'},
    {"version", no_argument, NULL, 0},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
  };

// The structure for holding the argument flags.

enum ARG_FLAGS {
  ARG_FLAG_VERBOSE = 1 << 0,
  ARG_FLAG_EVALUATE = 1 << 1,
  ARG_FLAG_BOOLEAN = 1 << 2,
  ARG_FLAG_GRADE = 1 << 3
};

#define AF_VERBOSE(flags) (flags & 1)
#define AF_EVALUATE(flags) ((flags >> 1) & 1)
#define AF_BOOLEAN(flags) ((flags >> 2) & 1)
#define AF_GRADE(flags) ((flags >> 3) & 1)

struct arg_items {
  char flags;
  char * file_name[256];
  char * latex_name[256];
  char * conclusion;
  vec_t * prems;
  char rule[3];
  char * rule_file;
  vec_t * vars;
};

/* Lists the rules.
 *  input:
 *    none.
 *  output:
 *    none.
 */
void
list_rules ()
{
  printf ("Inference rules:\n");
  printf ("  mp - Modus Ponens\n");
  printf ("  ad - Addition\n");
  printf ("  sm - Simplification\n");
  printf ("  cn - Conjunction\n");
  printf ("  hs - Hypothetical Syllogism\n");
  printf ("  ds - Disjunctive Syllogism\n");
  printf ("  ex - Excluded Middle\n");
  printf ("  cd - Constructive Dilemma\n");
  printf ("\n");
  printf ("Equivalence Rules\n");
  printf ("  im - Implication\n");
  printf ("  dm - DeMorgan*\n");
  printf ("  as - Association*\n");
  printf ("  co - Commutativity*\n");
  printf ("  id - Idempotence*\n");
  printf ("  dt - Distribution*\n");
  printf ("  eq - Equivalence\n");
  printf ("  dn - Double Negation*\n");
  printf ("  ep - Exportation\n");
  printf ("  sb - Subsumption*\n");
  printf ("\n");
  printf ("Predicate Rules\n");
  printf ("  ug - Universal Generalization\n");
  printf ("  ui - Universal Instantiation\n");
  printf ("  eg - Existential Generalization\n");
  printf ("  ei - Existential Instantiation\n");
  printf ("  bv - Bound Variable\n");
  printf ("  nq - Null Quantifier\n");
  printf ("  pr - Prenex\n");
  printf ("  ii - Identity\n");
  printf ("  fv - Free Variable\n");
  printf ("\n");
  printf ("Miscellaneous Rules\n");
  printf ("  lm - Lemma\n");
  printf ("  sp - Subproof\n");
  printf ("  sq - Sequence Instantiation\n");
  printf ("  in - Induction\n");
  printf ("\n");
  printf ("Boolean Rules\n");
  printf ("  bi - Boolean Identity*\n");
  printf ("  bn - Boolean Negation*\n");
  printf ("  bd - Boolean Dominance*\n");
  printf ("  sn - Symbol Negation*\n");
  printf ("\n");
  printf ("* = This rule is available in boolean mode.\n");

  exit (EXIT_SUCCESS);
}

/* Prints the version information and exits.
 *  input:
 *    none.
 *  output:
 *    none.
 */
void
version ()
{
  printf ("%s - %s\n", PACKAGE_NAME, VERSION);
  printf ("Copyright (C) 2012, 2013 Ian Dunn.\n");
  printf ("License GPLv3: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n");
  printf ("This is free software: you are free to change and redistribute it.\n");
  printf ("There is NO WARRANTY, to the extent premitted by the law.\n");
  printf ("\n");
  printf ("Written by Ian Dunn.\n");
  exit (EXIT_SUCCESS);
}

/* Prints usage and exits.
 *  input:
 *    status - status to exit with.
 *  output:
 *    none.
 */
void
usage (int status)
{
  printf ("Usage: aris [OPTIONS]... [-f FILE] [-g]\n");
  printf ("   or: aris [OPTIONS]... [-p PREMISE]... -r RULE -c CONCLUSION\n");
  printf ("\n");
  printf ("Options:\n");
  printf ("  -a, --variable=VARIABLE        Use VARIABLE as a variable.\n");
  printf ("                                  Place an '*' next to the variable to designate it as arbitrary.\n");
  printf ("  -b, --boolean                  Run Aris in boolean mode.\n");
  printf ("  -c, --conclusion=CONCLUSION    Set CONCLUSION as the conclusion.\n");
  printf ("  -e, --evaluate                 Run Aris in evaluation mode.\n");
  printf ("  -f, --file=FILE                Evaluate FILE.\n");
  printf ("  -g, --grade                    Grade files specified in the file flag.\n");
  printf ("  -l, --list                     List the available rules.\n");
  printf ("  -p, --premise PREMISE          Use PREMISE as a premise.\n");
  printf ("  -r, --rule RULE                Set RULE as the rule.\n");
  printf ("                                  Use 'lm:/path/to/file' to designate a file.\n");
  printf ("  -t, --text TEXT                Simply check the correctness of TEXT.\n");
  printf ("  -v, --verbose                  Print status and error messages.\n");
  printf ("  -x, --latex=FILE               Convert FILE to a LaTeX proof file.\n");
  printf ("  -h, --help                     Print this help and exit.\n");
  printf ("      --version                  Print the version and exit.\n");
  printf ("\n");
  printf ("Report %s bugs to %s\n", PACKAGE_NAME, PACKAGE_BUGREPORT);
  printf ("%s home page: <http://www.gnu.org/software/%s/>\n",
	  PACKAGE_NAME, PACKAGE);
  printf ("General help using GNU software: <http://www.gnu.org/gethelp/>\n");

  exit (status);
}

/* Checks argument text.
 *  input:
 *   arg_text - the text to check.
 *  output:
 *   0 if unsuccessful, 1 if successful, -1 on error.
 */
int
check_arg_text (unsigned char * arg_text)
{
  int ret_chk;

  ret_chk = check_text (arg_text);
  if (ret_chk == -1)
    return -1;

  switch (ret_chk)
    {
    case 0:
      break;
    case -2:
      fprintf (stderr, "Text Error - there are mismatched parentheses in the string - '%s'\n", arg_text);
      return 0;
    case -3:
      fprintf (stderr, "Text Error - there are invalid connectives in the string - '%s'\n", arg_text);
      return 0;
    case -4:
      fprintf (stderr, "Text Error - there are invalid quantifiers in the string - '%s'\n", arg_text);
      return 0;
    case -5:
      fprintf (stderr, "Text Error - there are syntactical errors in the string - '%s'\n", arg_text);
      return 0;
    }

  return 1;
}

/* Grades a single proof.
 *  input:
 *    c_file - the proof to grade.
 *  output:
 *    0 if it failed, 1 if the proof passsed.
 *    -1 on memory error.
 */
int
grade_file (proof_t * c_file)
{
  int ret_chk, grade;
  vec_t * rets;

  grade = 0;

  rets = init_vec (sizeof (char *));
  if (!rets)
    return -1;

  ret_chk = proof_eval (c_file, rets, 0);
  if (ret_chk == -1)
    return -1;

  int i, wrong = 0;
  item_t * ev_itr;

  ev_itr = c_file->everything->head;
  for (i = 0; i < rets->num_stuff; i++)
    {
      char * cur_ret, * cur_line;
      cur_ret = vec_str_nth (rets, i);
      cur_line = ((sen_data *) ev_itr->value)->text;

      if (strcmp (cur_ret, CORRECT))
	{
	  wrong = 1;
	  grade = 0;
	  printf ("Error in line %i - %s\n", i, cur_line);
	  printf ("  %s\n", cur_ret);
	  printf ("\n");
	}
      ev_itr = ev_itr->next;
    }

  // Check the goals.
  for (ev_itr = c_file->goals->head; ev_itr; ev_itr = ev_itr->next)
    {
      unsigned char * cur_goal;

      cur_goal = die_spaces_die ((unsigned char *) ev_itr->value);
      if (!cur_goal)
	return -1;

      item_t * sen_itr;

      for (sen_itr = c_file->everything->head; sen_itr; sen_itr = sen_itr->next)
	{
	  unsigned char * cur_sen;
	  int cmp;

	  cur_sen = die_spaces_die (((sen_data *) sen_itr->value)->text);
	  if (!cur_sen)
	    return -1;

	  cmp = !strcmp (cur_goal, cur_sen);
	  free (cur_sen);

	  if (cmp)
	    break;
	}

      if (!sen_itr)
	{
	  printf ("Goal '%s' was not met.\n", cur_goal);
	  free (cur_goal);
	  break;
	}

      free (cur_goal);
    }

  grade = (wrong) ? 0 : 1;

  if (wrong)
    printf ("Errors were found - See output for more information\n");
  else
    printf ("No errors found!  Well done!\n");

  return grade;
}

/* Parses the supplied arguments.
 *  input:
 *   argc, argv - should be self-explanatory.
 *   ai - a pointer to an argument structure to get the arguments.
 *  output:
 *   0 on success.
 */
int
parse_args (int argc, char * argv[], struct arg_items * ai)
{
  int c;

  int cur_file, cur_grade, cur_latex;
  int opt_len;
  int c_ret;

  cur_file = cur_grade = cur_latex = 0;
  ai->flags = '\0';
  ai->rule_file = NULL;
  for (c = 0; c < 256; c++)
    {
      ai->file_name[c] = NULL;
      ai->latex_name[c] = NULL;
    }

  ai->prems = init_vec (sizeof (char*));
  ai->vars = init_vec (sizeof (variable));
  memset ((char *) ai->rule, 0, sizeof (char) * 3);

  //Only one conclusion and one rule can exist.
  //int got_conc = false, got_rule = false;
  int got_conc = 0, got_rule = 0;

  main_conns = cli_conns;

  while (1)
    {
      int opt_idx = 0;

      c = getopt_long (argc, argv, "ep:c:r:t:a:f:gi:s:x:lbvh", long_opts, &opt_idx);

      if (c == -1)
	break;

      switch (c)
	{
	case 'e':
	  ai->flags |= ARG_FLAG_EVALUATE;
	  break;

	case 'p':
	  if (optarg)
	    {
	      unsigned char * sexpr_prem;
	      unsigned char * tmp_str;

	      c_ret = check_arg_text (optarg);
	      if (c_ret == -1 || c_ret == 0)
		exit (EXIT_FAILURE);

	      tmp_str = die_spaces_die (optarg);
	      if (!tmp_str)
		exit (EXIT_FAILURE);

	      sexpr_prem = convert_sexpr (tmp_str);
	      if (!sexpr_prem)
		exit (EXIT_FAILURE);
	      free (tmp_str);

	      c_ret = vec_str_add_obj (ai->prems, sexpr_prem);
	      if (c_ret == -1)
		exit (EXIT_FAILURE);
	      free (sexpr_prem);
	      break;
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - premise flag requires an argument, ignoring flag.\n");
	      break;
	    }
	  break;

	case 'c':

	  if (optarg)
	    {
	      if (got_conc)
		{
		  fprintf (stderr, "Argument Error - only one (1) conclusion must be specified, ignoring conclusion \"%s\".\n", optarg);
		  break;
		}

	      c_ret = check_arg_text (optarg);
	      if (c_ret == -1 || c_ret == 0)
		exit (EXIT_FAILURE);

	      unsigned char * tmp_str;
	      tmp_str = die_spaces_die (optarg);
	      if (!tmp_str)
		exit (EXIT_FAILURE);

	      ai->conclusion = convert_sexpr (tmp_str);
	      if (!ai->conclusion)
		exit (EXIT_FAILURE);
	      free (tmp_str);

	      got_conc = 1;
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - conclusion flag requires an argument, ignoring flag.\n");
	      break;
	    }
	  break;

	case 't':
	  if (optarg)
	    {
	      c_ret = check_arg_text (optarg);
	      if (c_ret == -1)
		exit (EXIT_FAILURE);
	      printf ("Correct!\n");
	      exit (EXIT_SUCCESS);
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - text flag requires an argument, ignoring flag.\n");
	      break;
	    }
	  break;

	case 'r':

	  if (optarg)
	    {
	      if (got_rule)
		{
		  fprintf (stderr, "Argument Warning - only one (1) rule must be specified, ignoring rule \"%s\".\n", optarg);
		  break;
		}

	      opt_len = strlen (optarg);

	      if (opt_len > 2 && strncmp (optarg, "lm:", 3))
		{
		  fprintf (stderr, "Argument Warning - a rule must be two (2) characters long, ignoring rule \"%s\".\n", optarg);
		  break;
		}

	      strncpy (ai->rule, optarg, 2);
	      ai->rule[2] = '\0';

	      if (strlen (optarg) > 3)
		{
		  ai->rule_file = (char *) calloc (opt_len - 2, sizeof (char));

		  strncpy (ai->rule_file, optarg + 3, opt_len - 3);
		  ai->rule_file[opt_len - 3] = '\0';
		}
	      got_rule = 1;
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - rule flag requires an argument, ignoring flag.\n");
	      break;
	    }
	  break;

	case 'a':
	  if (optarg)
	    {
	      if (!islower (optarg[0]))
		{
		  fprintf (stderr, "Argument Warning - the first character of a variable must be lowercase, ignoring variable \"%s\".\n", optarg);
		  break;
		}

	      variable v;
	      opt_len = strlen (optarg);

	      v.text = (unsigned char *) calloc (opt_len, sizeof (char));
	      strcpy (v.text, optarg);

	      if (v.text[opt_len - 1] == '*')
		{
		  v.arbitrary = 1;
		  v.text[opt_len - 1] = '\0';
		}
	      else
		{
		  v.arbitrary = 0;
		}

	      vec_add_obj (ai->vars, &v);
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - variable flag requires an argument, ignoring flag.\n");
	      break;
	    }
	  break;

	case 'f':

	  if (optarg)
	    {
	      if (ai->file_name[255])
		{
		  fprintf (stderr, "Argument Warning - a maximum of 256 filenames can be specified, ignoring file \"%s\".\n", optarg);
		  break;
		}

	      int arg_len = strlen (optarg);

	      ai->file_name[cur_file] = (char *) calloc (arg_len + 1,
							 sizeof (char));
	      if (!ai->file_name[cur_file])
		{
		  perror (NULL);
		  exit (EXIT_FAILURE);
		}

	      strcpy (ai->file_name[cur_file], optarg);
	      cur_file++;
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - file flag requires a filename, ignoring flag.\n");
	      break;
	    }
	  break;

	case 'g':
	  ai->flags |= ARG_FLAG_GRADE;
	  break;

	case 'i':
	  if (optarg)
	    {
	      proof_t * proof;

	      proof = proof_init ();
	      if (!proof)
		exit (EXIT_FAILURE);

	      main_conns = cli_conns;
	      parse_thy (optarg, proof);
	      exit (EXIT_SUCCESS);
	    }

	case 's':
	  if (optarg)
	    {
	      main_conns = cli_conns;

	      unsigned char * no_spaces, * sexpr_str;
	      no_spaces = die_spaces_die (optarg);
	      sexpr_str = convert_sexpr (no_spaces);
	      printf ("%s\n", sexpr_str);
	      exit (EXIT_SUCCESS);
	    }

	case 'b':
	  ai->flags |= ARG_FLAG_BOOLEAN;
	  break;

	case 'l':
	  list_rules ();
	  break;

	case 'v':
	  ai->flags |= ARG_FLAG_VERBOSE;
	  break;

	case 'x':
	  if (optarg)
	    {
	      if (ai->latex_name[255])
		{
		  fprintf (stderr, "Argument Warning - a maximum of 256 filenames can be specified, ignoring file \"%s\".\n", optarg);
		  break;
		}

	      int arg_len = strlen (optarg);

	      ai->latex_name[cur_latex] = (char *) calloc (arg_len + 1,
							   sizeof (char));
	      if (!ai->latex_name[cur_latex])
		{
		  perror (NULL);
		  exit (EXIT_FAILURE);
		}

	      strcpy (ai->latex_name[cur_latex], optarg);
	      cur_latex++;
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - grade flag requires a filename, ignoring flag.\n");
	    }
	  break;

	case 0:
	  if (opt_idx == 14)
	    version ();
	  break;

	case 'h':
	  usage (EXIT_SUCCESS);
	  break;

	default:
	  fprintf (stderr, "Argument Error - ignoring unrecognized option: \"%c\" .\n",c);
	  break;
	}
    }

  return 0;
}

/* Main function.  There REALLY shouldn't be any explaination needed. */
int
main (int argc, char *argv[])
{
  int c;

  vec_t * prems;
  unsigned char * conc = NULL;
  char * rule;
  vec_t * vars;
  char ** file_name, ** latex_name;
  proof_t ** proof;
  int cur_file, cur_latex, grade;
  char * rule_file = NULL;
  int opt_len, verbose, boolean, evaluate_mode;
  int c_ret;
  struct arg_items args;

  c_ret = parse_args (argc, argv, &args);

  prems = args.prems;
  conc = args.conclusion;
  vars = args.vars;
  verbose = AF_VERBOSE (args.flags);
  evaluate_mode = AF_EVALUATE (args.flags);
  boolean = AF_BOOLEAN (args.flags);
  file_name = (char **) args.file_name;
  latex_name = (char **) args.latex_name;
  grade = AF_GRADE (args.flags);
  rule = args.rule;
  rule_file = args.rule_file;

  cur_file = cur_latex = -1;

  for (c = 0; c < 256; c++)
    {
      if (cur_file != -1 && cur_latex != -1)
	break;

      if (cur_file == -1 && !file_name[c])
	cur_file = c;

      if (cur_latex == -1 && !latex_name[c])
	cur_latex = c;
    }

  if (conc == NULL && evaluate_mode && !file_name[0] && !latex_name[0])
    {
      fprintf (stderr, "Argument Error - a conclusion must be specified in evaluation mode.\n");
      exit (EXIT_FAILURE);
    }

  if (cur_latex > 0)
    {
      proof = (proof_t **) calloc (cur_latex, sizeof (proof_t *));
      CHECK_ALLOC (proof, EXIT_FAILURE);

      for (c = 0; c < cur_latex; c++)
	{
	  proof[c] = aio_open (latex_name[c]);
	  if (!proof[c])
	    exit (EXIT_FAILURE);

	  char * fname;
	  int n_len;

	  n_len = strlen (latex_name[c]);
	  fname = (char *) calloc (n_len + 1, sizeof (char));
	  CHECK_ALLOC (fname, EXIT_FAILURE);

	  strncpy (fname, latex_name[c], n_len - 3);
	  sprintf (fname + n_len - 3, "tex");

	  c_ret = convert_proof_latex (proof[c], fname);
	  if (c_ret == -1)
	    exit (EXIT_FAILURE);

	  proof_destroy (proof[c]);
	  free (proof[c]);
	}

      exit (EXIT_SUCCESS);
    }

  if (cur_file > 0)
    {
      proof = (proof_t **) calloc (cur_file, sizeof (proof_t *));
      if (!proof)
	{
	  perror (NULL);
	  exit (EXIT_FAILURE);
	}

      for (c = 0; c < cur_file; c++)
	{
	  proof[c] = aio_open (file_name[c]);
	  if (!proof[c])
	    exit (EXIT_FAILURE);
	}
    }

  if (evaluate_mode)
    {
      main_conns = cli_conns;

      if (cur_file > 0)
	{
	  if (grade)
	    {
	      int g;
	      for (c = 0; c < cur_file; c++)
		{
		  if (verbose)
		    printf ("Grading file: '%s'\n", file_name[c]);
		  g = grade_file (proof[c]);
		  if (g == -1)
		    exit (EXIT_FAILURE);
		  printf ("\n");
		}

	      exit (EXIT_SUCCESS);
	    }

	  for (c = 0; c < cur_file; c++)
	    {
	      int ret_chk;
	      ret_chk = proof_eval (proof[c], NULL, verbose);
	      if (ret_chk == -1)
		exit (EXIT_FAILURE);
	    }
	}
      else
	{
	  char * p_ret;

	  proof_t * proof = NULL;

	  if (rule_file)
	    {
	      int f_len;
	      f_len = strlen (rule_file);
	      if (!strcmp (rule_file + f_len - 4, ".thy"))
		{
		  int ret_chk;
		  proof = proof_init ();
		  if (!proof)
		    exit (EXIT_FAILURE);

		  ret_chk = parse_thy (rule_file, proof);
		  if (ret_chk == -1)
		    exit (EXIT_FAILURE);
		}
	      else
		{
		  main_conns = gui_conns;
		  proof = aio_open (rule_file);
		  if (!proof)
		    exit (EXIT_FAILURE);
		  main_conns = cli_conns;
		}
	    }

	  p_ret = process (conc, prems, rule, vars, proof);
	  if (!p_ret)
	    exit (EXIT_FAILURE);

	  printf ("%s\n", p_ret);
	}

      return 0;
    }
  else
    {
#ifndef ARIS_GUI
      fprintf (stderr, "Fatal Error - evaluate flag not specified in non-gui mode.\n");
      exit (EXIT_FAILURE);
#else

      main_conns = cli_conns;

      gtk_init (&argc, &argv);

      the_app = init_app (boolean, verbose);

      // Get the current working directory from arg0, then determine the help file.
      GFile * arg0, * parent;

      arg0 = g_file_new_for_commandline_arg (argv[0]);
      parent = g_file_get_parent (arg0);
      the_app->working_dir = g_file_get_path (parent);

      if (the_app->working_dir)
	{
	  parent = g_file_get_parent (parent);
	  sprintf (the_app->help_file, "file://%s/doc/aris/index.html", g_file_get_path (parent));
	}

      int ret;
      if (cur_file > 0)
	{
	  for (c = 0; c < cur_file; c++)
	    {
	      aris_proof * new_gui = aris_proof_init_from_proof (proof[c]);
	      if (!new_gui)
		return -1;

	      aris_proof_set_filename (new_gui, file_name[c]);
	      new_gui->edited = 0;
              free (file_name[c]);

	      ret = the_app_add_gui (new_gui);
	      if (ret < 0)
		return -1;
	    }

	  gtk_widget_show_all (the_app->rt->window);
	  rules_table_align (the_app->rt, the_app->focused);
	}
      else
	{
	  aris_proof * main_gui;
	  main_gui = aris_proof_init ();
	  if (!main_gui)
	    return -1;

	  ret = the_app_add_gui (main_gui);
	  if (ret < 0)
	    return -1;

	  gtk_widget_show_all (the_app->rt->window);
	  rules_table_align (the_app->rt, main_gui);
	}

      if (the_app->fonts[FONT_TYPE_CUSTOM])
	{
	  rules_table_set_font (the_app->rt, FONT_TYPE_CUSTOM);
	  GList * gl;
	  GtkWidget * font_menu, * font_submenu;

	  gl = gtk_container_get_children (GTK_CONTAINER (the_app->rt->menubar));
	  font_menu = (GtkWidget *) g_list_nth_data (gl, 1);
	  font_submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (font_menu));

	  gl = gtk_container_get_children (GTK_CONTAINER (font_submenu));

	  gtk_widget_set_sensitive ((GtkWidget *) gl->data, TRUE);
	}
      else
	{
	  rules_table_set_font (the_app->rt, FONT_TYPE_SMALL);
	}


      gtk_main ();
#endif
    }

  return 0;
}

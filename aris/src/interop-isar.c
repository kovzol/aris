#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "vec.h"
#include "process.h"
#include "interop-isar.h"
#include "list.h"
#include "proof.h"
#include "sen-data.h"

// Defined sequences.
vec_t * seqs;

// List of null keys.
static const char * null_keys[] = {
  "text",
  "text_raw",
  "subsection",
  "subsubsection",
  NULL
};

static char * conn_prec[4] = {
  "%",
  "$",
  "|",
  "&"
};

/* Main Idea:
 *  Parse the definitions from the .thy file,
 *  then parse the lemmas and theorems.
 *  Create a vector of references from the definitions,
 *  then, create a proof_t with the definitions and such as premises,
 *  and the lemmas as goals.
 *  Use (R0 & R1 & R2 & ... & RN) $ Conc for each lemma.
 */

// Might help to keep track of annotations, such as infixl, etc.
// Keep track of function name, annotation string, association, and priority.

/*TODO:
 *  Most of these functions don't do anything yet, but stand as place holders.
 *  Still need to parse notation, such as 'infix'.
 *  Still need:
 *    class
 *    instance
 *    instantiation
 *    abbreviation
 */

int
get_std_seqs ()
{
  int chk;
  in_type seq_n, seq_s;

  seqs = init_vec (sizeof (in_type));
  if (!seqs)
    return -1;

  seq_n.type = "nat";
  seq_n.seq = "n";
  chk = vec_add_obj (seqs, &seq_n);
  if (chk < 0)
    return -1;

  seq_s.type = "set";
  seq_n.seq = "s";
  chk = vec_add_obj (seqs, &seq_s);
  if (chk < 0)
    return -1;

  return 0;
}

char *
get_new_seq ()
{
  // Change this to a global variable.
  int cur_seq = 0;

  int alloc_size;
  char * ret;

  alloc_size = (int) log ((double) cur_seq) + 1;
  alloc_size += 2;
  ret = (char *) calloc (alloc_size + 1, sizeof (char));
  CHECK_ALLOC (ret, NULL);
  sprintf (ret, "ts%i", ++cur_seq);

  return ret;
}

int
parse_quant (char * in_str, int sym_size, unsigned char * quant, char ** out_str)
{
  int alloc_size, pos, init_pos, ret_chk;
  char * chk_str;
  vec_t * vars;

  alloc_size = 0;
  pos = sym_size;

  vars = init_vec (sizeof (char *));
  if (!vars)
    return -1;

  while (in_str[pos] != '.')
    {
      while (isspace (in_str[pos]))
	pos++;

      init_pos = pos;

      while (isalpha (in_str[pos]))
	pos++;

      char * tmp_str;
      tmp_str = (char *) calloc (pos - init_pos + 1, sizeof (char));
      CHECK_ALLOC (tmp_str, -1);
      strncpy (tmp_str, in_str + init_pos, pos - init_pos);
      tmp_str[pos - init_pos] = '\0';
      alloc_size += pos - init_pos + CL;

      ret_chk = vec_str_add_obj (vars, tmp_str);
      if (ret_chk < 0)
	return -1;

      free (tmp_str);
    }

  *out_str = (char *) calloc (alloc_size + 2, sizeof (char));
  CHECK_ALLOC (*out_str, -1);

  int i, out_pos = 0;
  for (i = 0; i < vars->num_stuff; i++)
    {
      out_pos += sprintf (*out_str + out_pos, "%s%s",
			  quant, vec_str_nth (vars, i));
    }
  destroy_str_vec (vars);

  out_pos += sprintf (*out_str + out_pos, "(");

  return pos;
}

// Functions associate to the left.
// For example
// f a b c = ((f a) b) c

int
parse_pred_func (char * in_str, char ** out_str)
{
  int pos, init_pos, ret_chk;
  char * pred;

  init_pos = pos = 0;

  while (isalnum (in_str[pos]))
    pos++;

  pred = (char *) calloc (pos + 1, sizeof (char));
  CHECK_ALLOC (pred, -1);
  strncpy (pred, in_str, pos);
  pred[pos] = '\0';

  int tmp_pos;
  tmp_pos = pos;

  while (isspace (in_str[tmp_pos]))
    tmp_pos++;

  //TODO: Check for a connective or closing parenthesis.
  if (in_str[tmp_pos] == '|' || in_str[tmp_pos] == '&'
      || !strncmp (in_str + tmp_pos, "-->", 3)
      || !strncmp (in_str + tmp_pos, "==>", 3)
      || in_str[tmp_pos] == ':'
      || in_str[tmp_pos] == ')' || in_str[tmp_pos] == '\0')
    {
      *out_str = (char *) calloc (pos + 1, sizeof (char));
      CHECK_ALLOC (*out_str, -1);
      strcpy (*out_str, pred);
      free (pred);
      return pos;
    }

  vec_t * args;
  int alloc_size = 0;

  args = init_vec (sizeof (char *));
  if (!args)
    return -1;

  while (in_str[pos] != ')' && in_str[pos] != '\0'
	 && in_str[pos] != '>' && in_str[pos] != '*'
	 && in_str[pos] != '<' && in_str[pos] != '+'
	 && strncmp (in_str + pos, "-->", 3)
	 && strncmp (in_str + pos, "==>", 3))
    {
      while (isspace (in_str[pos]))
	pos++;

      if (in_str[pos] == ')' || in_str[pos] == '\0'
	  || in_str[pos] == '>' || in_str[pos] == '*'
	  || in_str[pos] == '<' || in_str[pos] == '+'
	  || !strncmp (in_str + pos, "-->", 3)
	  || !strncmp (in_str + pos, "==>", 3))
	{
	  break;
	}

      init_pos = pos;

      if (in_str[pos] == '(')
	{
	  tmp_pos = parse_parens (in_str, pos, NULL);
	  pos = tmp_pos + 1;
	}
      else
	{
	  while (isalnum (in_str[pos]) || in_str[pos] == ':')
	    pos++;
	}

      char * cur_arg;

      cur_arg = (char *) calloc (pos - init_pos + 1, sizeof (char));
      CHECK_ALLOC (cur_arg, -1);
      strncpy (cur_arg, in_str + init_pos, pos - init_pos);
      cur_arg[pos - init_pos] = '\0';
      alloc_size += 2 + pos - init_pos;

      ret_chk = vec_str_add_obj (args, cur_arg);
      if (ret_chk == -1)
	return -1;
      free (cur_arg);
    }

  int i, out_pos = 0;

  *out_str = (char *) calloc (strlen (pred) + 2, sizeof (char));
  CHECK_ALLOC (*out_str, -1);
  out_pos += sprintf (*out_str + out_pos, "%s(", pred);
  free (pred);

  for (i = 0; i < args->num_stuff; i++)
    {
      char * elm_str, * cur_arg;
      cur_arg = vec_str_nth (args, i);
      printf ("arg[%i] = '%s'\n", i, cur_arg);
      if (cur_arg[0] == '(')
	{
	  elm_str = elim_par (cur_arg);
	  if (!elm_str)
	    return -1;
	}
      else
	{
	  elm_str = (char *) calloc (strlen (cur_arg) + 1, sizeof (char));
	  CHECK_ALLOC (elm_str, -1);
	  strcpy (elm_str, cur_arg);
	}

      char * mod_arg;
      ret_chk = parse_pred_func (elm_str, &mod_arg);
      if (ret_chk == -1)
	return -1;
      free (elm_str);

      *out_str = (char *) realloc (*out_str,
				   (out_pos + strlen (mod_arg) + 2) * sizeof (char));
      CHECK_ALLOC (*out_str, -1);
      out_pos += sprintf (*out_str + out_pos, "%s", mod_arg);
      free (mod_arg);

      if (i < args->num_stuff - 1)
	out_pos += sprintf (*out_str + out_pos, ",");
    }

  *out_str = (char *) realloc (*out_str, (out_pos + 2) * sizeof (char));
  CHECK_ALLOC (*out_str, -1);
  out_pos += sprintf (*out_str + out_pos, ")");

  return pos;
}

// Check the connectives, and group them as necessary.
// The priority in Isar is as follows:
// ~, &, |, $, %
// This should be reversed, so that they group properly.
// These all associate to the right (except, of course, ~).
// parses a specific connective, and ignore the others.
// Runs for each connective in order.
//IDEA: Run this for ALL KNOWN SYMBOLS.
// When each is defined, a priority and association is defined with it.
//ALSO: Main is not always imported, so it might need to be processed.
// Might be helpful to have functions for processing standard imports.
// HOL, Orderings, Lattices, Sets, etc.
// A complete list is in Isabelle/doc/main.pdf

int
parse_connectives (char * in_str, int cur_conn, char ** out_str)
{
  if (cur_conn > 3)
    {
      *out_str = (char *) calloc (strlen (in_str), sizeof (char));
      CHECK_ALLOC (*out_str, -1);
      strcpy (*out_str, in_str);
      return 0;
    }

  vec_t * gens;
  int gg = 0, ret_chk, gen_pos, in_len;
  unsigned char * new_gen, * conn, * gen = NULL;

  //Process negations and quantifiers first.
  //Similar manner to that of 'convert_sexpr' from process.c.

  gens = init_vec (sizeof (char *));
  if (!gens)
    return -1;
  conn = conn_prec[cur_conn];

  // It would be better if spaces were gone when this is processing,
  // so it would be good if predicates and functions are fixed BEFORE
  // parse_connectives is called.

  vec_t * got_conns;
  int got_con;

  got_conns = init_vec (sizeof (int));
  if (!got_conns)
    return -1;

  while (in_str[gg] != '\0')
    {
      gen_pos = 0;
      gen = (unsigned char *) calloc (1, sizeof (char));
      CHECK_ALLOC (gen, -1);

      while (strncmp (in_str + gg, conn, CL))
	{
	  gg = get_gen (in_str, gg, &new_gen);
	  if (gg == -1)
	    return -1;

	  // Need to check for the existence of an actual connective.
	  // Parent parse_connectives will need to know if parentheses are necessary

	  int g_len;

	  g_len = strlen (new_gen);
	  gen = realloc (gen, (gen_pos + g_len) * sizeof (char));
	  CHECK_ALLOC (gen, -1);
	  
	  gen_pos += sprintf (gen + gen_pos, "%s", new_gen);
	  free (new_gen);

	  if (in_str[gg] == '\0')
	    {
	      got_con = 0;
	      if (got_conns->num_stuff == gens->num_stuff)
		{
		  ret_chk = vec_add_obj (got_conns, &got_con);
		  if (ret_chk < 0)
		    return -1;
		}
	      break;
	    }

	  if (strncmp (in_str + gg, conn, CL))
	    {
	      strncpy (gen + gen_pos, in_str + gg, CL);
	      gen_pos += CL;
	      got_con = 1;
	      if (got_conns->num_stuff == gens->num_stuff)
		{
		  ret_chk = vec_add_obj (got_conns, &got_con);
		  if (ret_chk < 0)
		    return -1;
		}
	    }
	  else
	    {
	      got_con = 0;
	      if (got_conns->num_stuff == gens->num_stuff)
		{
		  ret_chk = vec_add_obj (got_conns, &got_con);
		  if (ret_chk < 0)
		    return -1;
		}
	      break;
	    }

	  gg += CL;
	}

      ret_chk = vec_str_add_obj (gens, gen);
      if (ret_chk < 0)
	return -1;

      free (gen);
      if (in_str[gg] != '\0')
	gg += CL;
    }

  if (gens->num_stuff == 1)
    {
      destroy_str_vec (gens);

      char * tmp_str, * mod_in_str;
      int mod = 0, out_pos, con_mod = 0, * ent, entire;

      // Confirm that the symbol covers the ENTIRE sentence.
      ent = vec_nth (got_conns, 0);
      entire = *ent;
      destroy_vec (got_conns);

      if (entire)
	{
	  mod_in_str = (char *) calloc (strlen (in_str), sizeof (char));
	  CHECK_ALLOC (mod_in_str, -1);
	  strcpy (mod_in_str, in_str);
	  con_mod = 1;
	}
      else
	{
	  if (!strncmp (in_str, NOT, NL))
	    {
	      mod_in_str = elim_not (in_str);
	      if (!mod_in_str)
		return -1;

	      mod = NL;
	    }
	  else if (in_str[0] == '(')
	    {
	      mod_in_str = elim_par (in_str);
	      if (!mod_in_str)
		return -1;

	      mod = 1;
	    }
	  else if (!strncmp (in_str, UNV, CL) || !strncmp (in_str, EXL, CL))
	    {
	      for (mod = CL; in_str[mod] != '\0'; mod++)
		{
		  if (!strncmp (in_str + mod, EXL, CL) || !strncmp (in_str + mod, UNV, CL)
		      || in_str[mod] == '(' || !strncmp (in_str + mod, NOT, NL))
		    break;
		}

	      mod_in_str = (char *) calloc (strlen (in_str) - mod, sizeof (char));
	      CHECK_ALLOC (mod_in_str, -1);
	      strcpy (mod_in_str, in_str + mod);
	    }
	  else
	    {
	      mod_in_str = (char *) calloc (strlen (in_str), sizeof (char));
	      CHECK_ALLOC (mod_in_str, -1);
	      strcpy (mod_in_str, in_str);
	      con_mod = 4;
	    }
	}

      gg = parse_connectives (mod_in_str, cur_conn + con_mod, &tmp_str);
      if (gg == -1)
	return -1;

      free (mod_in_str);
      out_pos = mod;

      *out_str = (char *) calloc (strlen (tmp_str) + mod + 2, sizeof (char));
      CHECK_ALLOC (*out_str, -1);
      strncpy (*out_str, in_str, mod);
      out_pos += sprintf (*out_str + out_pos, "%s", tmp_str);
      free (tmp_str);
      if (in_str[0] == '(' && !entire)
	(*out_str)[out_pos++] = ')';

      (*out_str)[out_pos] = '\0';

      return 0;
    }

  vec_t * adjust_gens;
  adjust_gens = init_vec (sizeof (char *));
  if (!adjust_gens)
    return -1;

  int i, alloc_size = 0;
  for (i = 0; i < gens->num_stuff; i++)
    {
      char * adj_gen;
      gg = parse_connectives (vec_str_nth (gens, i), 0, &adj_gen);
      if (gg == -1)
	return -1;

      ret_chk = vec_str_add_obj (adjust_gens, adj_gen);
      if (ret_chk < 0)
	return -1;

      alloc_size += strlen (adj_gen) + CL;
      free (adj_gen);

      // If parentheses need to be fixed, then increase alloc_size accordingly.
      int * cur_con;
      cur_con = vec_nth (got_conns, i);
      if (*cur_con)
	alloc_size += 2;
    }

  destroy_str_vec (gens);

  int out_pos = 0;

  if (cur_conn == 1 && adjust_gens->num_stuff > 2)
    alloc_size += 2 * adjust_gens->num_stuff;

  *out_str = (char *) calloc (alloc_size + 1, sizeof (char));
  CHECK_ALLOC (*out_str, -1);
  for (i = 0; i < adjust_gens->num_stuff; i++)
    {
      if (cur_conn == 1 && i > 0 && adjust_gens->num_stuff > 2 && i < adjust_gens->num_stuff - 1)
	out_pos += sprintf (*out_str + out_pos, "(");

      int * cur_con;
      cur_con = vec_nth (got_conns, i);
      if (*cur_con)
	out_pos += sprintf (*out_str + out_pos, "(");
      out_pos += sprintf (*out_str + out_pos, "%s", vec_str_nth (adjust_gens, i));
      if (*cur_con)
	out_pos += sprintf (*out_str + out_pos, ")");
      if (i < adjust_gens->num_stuff - 1)
	out_pos += sprintf (*out_str + out_pos, "%s", conn);
    }

  if (cur_conn == 1 && adjust_gens->num_stuff > 2)
    {
      for (i = 0; i < adjust_gens->num_stuff - 2; i++)
	out_pos += sprintf (*out_str + out_pos, ")");
    }

  destroy_vec (got_conns);
  destroy_vec (adjust_gens);

  return 0;
}

int
isar_parse_null (char * in_str)
{
  // Find '{' character, then find matching '}'.
  // Return offset.
  // Should be able to use modified 'parse_parens'.

  printf ("Got null\n");

  int pos = 0;

  while (strncmp (in_str + pos, "*}", 2))
    pos++;

  return pos;
}

char *
isar_mod_fun (char * in_str, char * fun_name)
{
  // Determine the input from in_str.
  // If from the nats, use 'v(n,x)'...
  // If suc (x) => v(n,s(x))
  // If 1 => v(n,s(z(x)))
  // Will need to know type sequences.
  // Change any instance of 'fun_name' to 'v(y,z)', using the appropriate arguments.

  // Suc 0 => suc (0) / v(n,s(x))

  // Summation starts as (\<Sum>i::nat=0..n. i) (Taken from Summation.thy)
  // change to sum(n, x);

  return NULL;
}

//IDEA: Perhaps don't process type_synonyms, but instead store them.
// When a synonym is come across during processing, expand it.

int
isar_parse_syn (char * syn, char ** out_str)
{
  // type_synonym

  char * left, * right;

  // Determine the sequence both are defined by,
  // and return (maybe):
  // @x(v(L,x) = v(R,x))

  printf ("Got synonym\n");
  // Syntax should be:
  // 'a seq_0 = 'a set_b
  // isn't always that way, though.
  // Looks like right-hand side always surrounded by quotes.

  int pos = 13, rest, start;

  while (1)
    {
      while (syn[pos] != '=')
	pos++;
      if (syn[pos] != '>')
	break;
    }

  start = pos;
  while (syn[start] != '\"')
    start++;

  rest = ++start;
  while (syn[rest] != '\"')
    rest++;

  // pos points to the '='.

  char * lsen, * rsen;

  lsen = (char *) calloc (pos - 12, sizeof (char));
  CHECK_ALLOC (lsen, -1);
  strncpy (lsen, syn + 13, pos);
  lsen[pos] = '\0';

  rsen = (char *) calloc (rest - start + 1, sizeof (char));
  CHECK_ALLOC (rsen, -1);
  strncpy (rsen, syn + start, rest - start);
  rsen[rest - start] = '\0';

  free (lsen);
  free (rsen);

  return rest;
}

// Remember kids: Logic is FUN!!
int
isar_parse_fun (char * fun, char ** out_str)
{
  printf ("Got fun\n");

  char * fun_name;
  int init_pos, pos;
  int chk;

  init_pos = 4;
  while (isspace(fun[init_pos]))
    init_pos++;

  pos = init_pos;
  while (fun[pos] != ':')
    pos++;

  pos -= 1;

  fun_name = (char *) calloc (pos - init_pos + 1, sizeof (char));
  CHECK_ALLOC (fun_name, -1);
  strncpy (fun_name, fun + init_pos, pos - init_pos);
  fun_name[pos - init_pos] = '\0';

  // Next should come "::"
  // pos points to two characters before it.
  // Determine what this function is mapping.

  char * map_str;

  init_pos = pos + 5;

  if (fun[init_pos] == '\"')
    init_pos++;

  pos = init_pos;

  while (fun[pos] != '\"')
    pos++;

  map_str = (char *) calloc (pos - init_pos + 1, sizeof (char));
  CHECK_ALLOC (map_str, -1);
  strncpy (map_str, fun + init_pos, pos - init_pos);
  map_str[pos - init_pos] = '\0';

  //TODO: Use map_str to determine what is being mapped.

  // Next, find the word 'where'

  init_pos = pos + 1;
  while (strncmp (fun + init_pos, "where", 5))
    init_pos++;

  // Now, find the next '"'

  while (fun[init_pos] != '\"')
    init_pos++;

  init_pos++;

  vec_t * base_cases;
  unsigned char * tmp_str;

  base_cases = init_vec (sizeof (char *));
  if (!base_cases)
    return -1;

  pos = init_pos;

  while (fun[pos] != '\"')
    pos++;

  tmp_str = (char *) calloc (pos - init_pos + 1, sizeof (char));
  CHECK_ALLOC (tmp_str, -1);
  strncpy (tmp_str, fun + init_pos, pos - init_pos);
  tmp_str[pos - init_pos] = '\0';

  while (fun[pos + 2] == '|')
    {
      char * mod_str;
      /*
      mod_str = isar_mod_fun (tmp_str, fun_name);
      if (!mod_str)
	return -1;
      */

      mod_str = tmp_str;

      chk = vec_str_add_obj (base_cases, mod_str);
      if (chk < 0)
	return -1;
      free (tmp_str);
      //free (mod_str);

      pos += 2;
      init_pos = pos;

      while (fun[init_pos] != '\"')
	init_pos++;

      init_pos++;
      pos = init_pos;

      while (fun[pos] != '\"')
	pos++;

      tmp_str = (char *) calloc (pos - init_pos + 1, sizeof (char));
      CHECK_ALLOC (tmp_str, -1);
      strncpy (tmp_str, fun + init_pos, pos - init_pos);
      tmp_str[pos - init_pos] = '\0';

      init_pos = pos + 2;
    }

  // Now, base_cases will hold the base cases, and tmp_str will hold the recursive step.

  int fin_pos = pos;

  char * mod_tmp_str;
  /*
  mod_tmp_str = isar_mod_fun (tmp_str, fun_name);
  if (!mod_tmp_str)
    return -1;
  */
  mod_tmp_str = tmp_str;

  // Use seqlog to construct what is needed.


  // Example:
  // From fibonacci.thy => 'fib' should look like this in the end:
  //@y(@z(v(y,z) = fib(v(n,z))) $ @z(v(y,z(z)) = 0 & v(y,s(z(z))) = v(n,s(z(z))) & v(y,s(s(z))) = v(y,z) + v(y,s(z))))

  int num_inputs = 0;
  char ** inputs;
  // Parse map_str for '\<Rightarrow>' to determine the amount of inputs.

  for (pos = 0; map_str[pos] != '\0'; pos++)
    {
      if (!strncmp (map_str + pos, "\\<Rightarrow>", 13))
	{
	  num_inputs++;
	  pos += 12;
	}
    }

  inputs = (char **) calloc (num_inputs, sizeof (char *));
  CHECK_ALLOC (inputs, -1);

  int i,j;

  i = 0;
  pos = 0;
  while (1)
    {
      char * type;
      init_pos = pos;
      while (strncmp (map_str + pos, "\\<Rightarrow>", 13) && map_str[pos] != '\0')
	pos++;

      int alloc_size = pos - init_pos - 1;
      if (map_str[pos] == '\0')
	alloc_size++;

      type = (char *) calloc (alloc_size + 1, sizeof (char));
      CHECK_ALLOC (type, -1);
      strncpy (type, map_str + init_pos, alloc_size);
      type[alloc_size] = '\0';

      for (j = 0; j < seqs->num_stuff; j++)
	{
	  in_type * cur;
	  cur = vec_nth (seqs, j);
	  if (!strcmp (type, cur->type))
	    {
	      inputs[i] = cur->seq;
	      break;
	    }
	}

      if (j == seqs->num_stuff)
	{
	  // New sequence.
	  // Not sure how to handle this exactly (yet).
	  in_type new_seq;

	  new_seq.type = type;
	  new_seq.seq = get_new_seq ();

	  chk = vec_add_obj (seqs, &new_seq);
	  if (chk == -1)
	    return -1;

	  inputs[i] = new_seq.seq;
	}

      if (map_str[pos] == '\0')
	break;
      else
	pos += 14;
    }

  int fun_pos;

  //TODO: Calculate the ACTUAL size this should be.

  *out_str = (char *) calloc (1024, sizeof (char));
  CHECK_ALLOC (*out_str, -1);

  for (i = 0; i < num_inputs - 1; i++)
    fun_pos += sprintf ((*out_str) + fun_pos, "%sx%i(", UNV, i);

  fun_pos += sprintf ((*out_str) + fun_pos, "%sy(%sz(v(y,z) = %s(",
		      UNV, UNV, fun_name);

  // TODO: Determine how the inputs are ordered, maybe.

  for (i = 0; i < num_inputs - 1; i++)
    fun_pos += sprintf ((*out_str) + fun_pos, "v(%s,x%i),", inputs[i], i);

  fun_pos += sprintf ((*out_str) + fun_pos, "v(%s,z)))", inputs[i]);
  fun_pos += sprintf ((*out_str) + fun_pos, "%s", CON);
  fun_pos += sprintf ((*out_str) + fun_pos, "%sz(", UNV);

  for (i = 0; i < base_cases->num_stuff; i++)
    {
      fun_pos += sprintf ((*out_str) + fun_pos, "%s%s",
			  vec_str_nth (base_cases, i), AND);
    }

  fun_pos += sprintf ((*out_str) + fun_pos, "%s))", mod_tmp_str);

  for (i = 0; i < num_inputs - 1; i++)
    fun_pos += sprintf ((*out_str) + fun_pos, ")");

  printf ("fun_str = '%s'\n", *out_str);

  free (tmp_str);
  //free (mod_tmp_str);

  return fin_pos;
}

int
isar_parse_lemma (char * lemma, char ** out_str)
{
  printf ("Got lemma\n");

  // Find the next instance of proof.
  //char * proof;
  //proof = strstr (lemma, "proof");

  int pos = 5;
  while (isspace (lemma[pos]))
    pos++;

  int init_pos;
  if (lemma[pos] != '\"')
    {
      if (!strncmp (lemma + pos, "proof", 5))
	{
	  pos += 5;
	  while (isspace (lemma[pos]))
	    pos++;
	}

      if (!strncmp (lemma + pos, "assume", 6))
	{
	  pos += 6;
	  if (lemma[pos] == 's')
	    {
	      // Move to assumption.
	    }

	  // Move to show statement.
	  // Exit.
	}

      while (!isspace (lemma[pos]))
	pos++;

      while (isspace (lemma[pos]))
	pos++;
    }

  char * tmp_str;

  init_pos = pos + 1;
  pos = init_pos;

  while (lemma[pos] != '\"')
    pos++;

  tmp_str = (char *) calloc (pos - init_pos + 1, sizeof (char));
  CHECK_ALLOC (tmp_str, -1);

  strncpy (tmp_str, lemma + init_pos, pos - init_pos);
  tmp_str[pos - init_pos] = '\0';

  *out_str = isar_to_aris (tmp_str);
  if (!(*out_str))
    return -1;
  free (tmp_str);

  // Find any instances of 'assume' and 'show'
  // May not be necessary.
  // Most instances of 'lemma' come with the sentence needed.
  // Might be able to grab everything from the starting line.

  /*
  char ** assume;
  char * show;

  assume = (char **) calloc (1024, sizeof (char *));
  if (!assume)
    {
      perror (NULL);
      return -1;
    }

  char * find_tmp, * show_pos;
  int i, pos;
  i = pos = 0;

  find_tmp = strstr (proof, "assume");
  show_pos = strstr (proof, "show");

  while (find_tmp < show_pos)
    {
      // Find the next newline or the next instance of the word 'and'.
      // Copy from find_tmp to the newline into assume[i]
      pos = 0;

      while (find_tmp[pos] != '\n' && strncmp (find_tmp + pos, "and", 3))
	pos++;

      int init_pos = 0;
      if (!strncmp (find_tmp, "assume", 6))
	init_pos = 6;
      if (!strncmp (find_tmp, "and", 3))
	init_pos = 3;

      assume[i] = (char *) calloc (pos - init_pos, sizeof (char));
      if (!assume[i])
	{
	  perror (NULL);
	  return -1;
	}

      strncpy (assume[i], find_tmp + init_pos, pos - init_pos);
      assume[i][pos - init_pos] = '\0';

      if (find_tmp[pos] == '\n')
	find_tmp = strstr (find_tmp + pos, "assume");
      else
	find_tmp += pos;

      i++;
    }

  pos = 0;
  // Also might want to check for '..' at the end of the show statement.
  while (show_pos[pos] != '\n' && strncmp (show_pos + pos, "by", 2))
    pos++;

  show = (char *) calloc (pos, sizeof (char *));
  if (!show)
    {
      perror (NULL);
      return -1;
    }

  strncpy (show, show_pos + 4, pos - 4);
  show[pos - 4] = '\0';

  // For each instance of assume, load a premise.

  unsigned char ** prems, * conc;

  prems = (unsigned char **) calloc (i + 1, sizeof (char *));
  if (!prems)
    {
      perror (NULL);
      return -1;
    }

  //printf ("Proof: %i\n", count);
  for (i = 0; assume[i]; i++)
    {
      //isar is an anagram of aris.
      //SO COOL!!
      printf ("assume[%i]: %s\n", i, assume[i]);
      prems[i] = isar_to_aris (assume[i]);
      if (!prems[i])
	return -1;
      printf ("prems[%i]: %s\n", i, prems[i]);
    }
  prems[i] = NULL;

  printf ("show: %s\n", show);
  conc = isar_to_aris (show);
  if (!conc)
    return -1;

  printf ("conc: %s\n", conc);

  int out_pos = 0;

  if (prems[1])
    out_pos += sprintf (*out_str + out_pos, "(");

  for (i = 0; prems[i]; i++)
    {
      out_pos += sprintf (*out_str + out_pos, "%s", prems[i]);
      if (prems[i + 1])
	out_pos += sprintf (*out_str + out_pos, "%s", AND);
    }

  if (prems[1])
    out_pos += sprintf (*out_str + out_pos, ")");

  */

  // In order to determine the end of the lemma,
  // find 'qed' or 'done',
  // although that might not work.

  return pos;
}

int
isar_parse_theorem (char * thm, char ** out_str)
{
  int pos;

  pos = isar_parse_lemma (thm + 2, out_str);
  if (pos == -1)
    return -1;

  return pos + 2;
}

int
isar_parse_datatype (char * type, vec_t * constructors)
{
  int init_pos, pos;
  int i, chk;

  init_pos = 0;

  // Determine the type name.
  // Determine the constructors.

  // Create a sequence, ts_n, for the new datatype.
  // After that, most of it is just syntax and notations.
  // Add the new functions to the function definitons.

  //IDEA: For each constructor with n args, create the string:
  // @x0@x1...@xn(#y0(y0 = v(tk, x0)) & ... & #yn(yn = v(tk, xn)) $ #z(z = constructor(x0,...,xn)))

  return pos;
}

int
isar_parse_primrec (char * prim, char ** out_str)
{
  //Should be similar to isar_parse_fun
  int ret;
  ret = isar_parse_fun (prim + 4, out_str);
  if (ret == -1)
    return -1;

  return ret + 4;
}

int
isar_parse_case (char * cs, char ** out_str)
{
  vec_t * pre, * post;
  int i, pos, init_pos;
  char * var;
  int out_pos;

  // get var

  init_pos = pos = 5;

  while (!isspace (cs[pos]))
    pos++;

  var = (char *) calloc (pos - init_pos + 1, sizeof (char));
  CHECK_ALLOC (var, -1);

  strncpy (var, cs + init_pos, pos - init_pos);
  var[pos - init_pos] = '\0';

  pos = init_pos + 4;

  // case 'var' of arg0 | arg1 | ... | argn

  // parse the arguments.
  // get the pred variable

  while (1)
    {
      char * tmp_str;

      init_pos = pos;

      while (cs[pos] != '|')
	pos++;

      tmp_str = (char *) calloc (pos - init_pos + 1, sizeof (char));
      CHECK_ALLOC (tmp_str, -1);

      strncpy (tmp_str, cs + init_pos, pos - init_pos);
      tmp_str[pos - init_pos] = '\0';

      // Process it, converting it to a notation aris will recognize.
      char * new_str;
      //new_str = isar_convert (tmp_str);
      //if (!new_str)
      //  return -1;

      free (tmp_str);

      int gg;
      unsigned char * lsen, * rsen;

      gg = get_gen (new_str, 0, &lsen);
      if (gg == -1)
	return -1;

      strcpy (rsen, new_str + gg + CL);

      free (new_str);

      gg = vec_str_add_obj (pre, lsen);
      if (gg < 0)
	return -1;

      gg = vec_str_add_obj (post, rsen);
      if (gg < 0)
	return -1;

      free (lsen);
      free (rsen);

      break;
    }

  int fin_pos = pos;

  // Get the last argument.

  // case(xs) = y % ((xs = pre0 $ y = post0) & ...)

  for (i = 0; i < pre->num_stuff; i++)
    {
    }

  return fin_pos;
}

int
isar_parse_def (char * def, char ** out_str)
{
  // Definition keyword.
  // Not particularly certain what to do with this yet,
  // since I'm not sure how definitions are used in Isabelle.

  //definition def where "stuff = more"
  // Logical eq (three horizontal lines) MUST be used in this.

  // Basic parsing.

  int pos, init_pos;
  char * defn;

  init_pos = 10;

  while (isspace(def[init_pos]))
    init_pos++;

  pos = init_pos;
  while (strncmp (def + pos, "where", 5))
    pos++;

  defn = (char *) calloc (pos - init_pos + 1, sizeof (char));
  CHECK_ALLOC (defn, -1);

  strncpy (defn, def + init_pos, pos - init_pos);
  defn[pos - init_pos] = '\0';

  // Need to check for '::',
  // and possibly determine the sequences (again).

  init_pos += 6;
  if (def[init_pos] == '\"')
    init_pos++;
  pos = init_pos;

  while (def[pos] != '\"')
    pos++;

  char * defin;
  int fin_pos = pos;

  defin = (char *) calloc (pos - init_pos + 1, sizeof (char));
  CHECK_ALLOC (defin, -1);
  strncpy (defin, def + init_pos, pos - init_pos);
  defin[pos - init_pos] = '\0';

  // Parse the definition for the logical eq symbol.

  free (defn);
  free (defin);

  return fin_pos;
}

unsigned char *
isar_to_aris (char * isar)
{
  printf ("isar = '%s'\n", isar);

  unsigned char * aris;
  int i, j, is_len;

  is_len = strlen (isar);
  aris = (unsigned char *) calloc (is_len, sizeof (char *));
  CHECK_ALLOC (aris, NULL);

  for (i = 0, j = 0; i < is_len; i++)
    {
      if (isar[i] == '\"')
	continue;

      if (!strncmp (isar + i, "-->", 3) || !strncmp (isar + i, "==>", 3))
	{
	  strcpy (aris + j, CON);
	  j += CL;
	  i += 2;
	  continue;
	}

      if (!strncmp (isar + i, "\\<longrightarrow>", 17))
	{
	  strcpy (aris + j, CON);
	  j += CL;
	  i += 16;
	  continue;
	}

      if (isar[i] == '&')
	{
	  strcpy (aris + j, AND);
	  j += CL;
	  continue;
	}

      if (isar[i] == '|')
	{
	  strcpy (aris + j, OR);
	  j += CL;
	  continue;
	}

      //TODO: For quantifiers;
      //1.  Check for a parenthesis BEFORE the quantifier.
      //2.  If there is one, remove it from the aris string,
      //3.  and make a note of this somehow,
      //      so the processor knows how to handle it later.

      if (!strncmp (isar + i, "EX", 2)
	  || !strncmp (isar + i, "\\<exists>", 9))
	{
	  if (j != 0 && aris[j - 1] == '(')
	    j--;

	  int size = 0;

	  if (!strncmp (isar + i, "EX", 2))
	    size = 2;
	  if (!strncmp (isar + i, "\\<exists>", 9))
	    size = 9;

	  int new_pos;
	  char * new_str;
	  new_pos = parse_quant (isar + i, size, EXL, &new_str);
	  if (new_pos == -1)
	    return NULL;

	  strcpy (aris + j, new_str);
	  j += strlen (new_str);
	  free (new_str);
	  i += new_pos;
	  continue;

	}

      if (!strncmp (isar + i, "!!", 2)
	  || !strncmp (isar + i, "\\<And>", 6)
	  || !strncmp (isar + i, "ALL", 3)
	  || !strncmp (isar + i, "\\<forall>", 9))
	{
	  if (j != 0 && aris[j - 1] == '(')
	    j--;

	  int size = 0;
	  if (!strncmp (isar + i, "!!", 2))
	    size = 2;
	  if (!strncmp (isar + i, "\\<And>", 6))
	    size = 6;
	  if (!strncmp (isar + i, "ALL", 3))
	    size = 3;
	  if (!strncmp (isar + i, "\\<forall>", 9))
	    size = 9;

	  int new_pos;
	  char * new_str;
	  new_pos = parse_quant (isar + i, size, UNV, &new_str);
	  if (new_pos == -1)
	    return NULL;

	  strcpy (aris + j, new_str);
	  j += strlen (new_str);
	  free (new_str);
	  i += new_pos;
	  continue;
	}

      if (!strncmp (isar + i, "<->", 3))
	{
	  strcpy (aris + j, BIC);
	  j += CL;
	  i += 2;
	  continue;
	}

      if (!strncmp (isar + i, "\\<longleftrightarrow>", 21))
	{
	  strcpy (aris + j, BIC);
	  j += CL;
	  i += 20;
	  continue;
	}

      if (!strncmp (isar + i, "\\<not>", 6))
	{
	  strcpy (aris + j, NOT);
	  j += NL;
	  i += 5;
	  continue;
	}

      if (isar[i] == '~')
	{
	  strcpy (aris + j, NOT);
	  j += NL;
	  continue;
	}

      if (isalpha (isar[i]))
	{
	  int new_pos, mod = 1;
	  char * new_str;
	  new_pos = parse_pred_func (isar + i, &new_str);
	  if (new_pos == -1)
	    return NULL;

	  j += sprintf (aris + j, "%s", new_str);
	  i += new_pos - mod;
	  continue;
	}

      aris[j] = isar[i];
      j++;
    }

  aris[j] = '\0';

  printf ("aris = '%s'\n", aris);

  return aris;
}

int
parse_thy (char * filename, proof_t * proof)
{
  FILE * file;

  file = fopen (filename, "r");
  CHECK_ALLOC (file, -1);

  int chk;
  long file_size;
  chk = fseek (file, 0, SEEK_END);
  if (chk)
    {
      fclose (file);
      perror (NULL);
      return -1;
    }

  file_size = ftell (file);

  chk = fseek (file, 0, SEEK_SET);
  if (chk)
    {
      fclose (file);
      perror (NULL);
      return -1;
    }

  char * buffer;

  buffer = (char *) calloc (file_size, sizeof (char));
  CHECK_ALLOC (buffer, -1);

  fread (buffer, 1, file_size, file);
  fclose (file);
  // Read in the file buffer to buffer.

  // Determine the main start of the file.
  char * ret_chk_str;
  ret_chk_str = strstr (buffer, "theory");
  if (!ret_chk_str)
    {
      // Invalid .thy file, return an error.
      return -2;
    }

  item_t * ls_chk;
  int i;

  //TODO: Check declarations (datatypes, functions, etc.)
  //TODO: Create a seqlog sequence (n for nats, etc.) for each one.
  //IDEA: Thinking t0, t1, (type 0, etc.) and store each in a map-like structure.
  //      Makes it easier to determine the input from functions.
  //NOTE: Datatypes are declared by constructor functions.
  //NOTE: ALL functions are recursive.

  vec_t * refs, * lms;

  char * buf_pos = ret_chk_str;
  //TODO: Parse the imports.
  // Will need to keep track of which files have already been parsed.
  // A chart of predefined files and their imports is shown here:
  //http://isabelle.in.tum.de/dist/library/HOL/large.html
  //EDIT: Only check the imports for definitions.
  //DO NOT REQUIRE THE USER TO CREATE LINES FOR ALL OF THEM,
  //or at least not for everything in Main.
  //That would be INSANE.

  chk = get_std_seqs ();
  if (chk == -1)
    return -1;

  refs = init_vec (sizeof (char *));
  if (!refs)
    return -1;

  lms = init_vec (sizeof (char *));
  if (!lms)
    return -1;

  // After this, buf_pos will point to the position of 'begin'.
  buf_pos = strstr (buffer, "begin");

  if (strncmp (buf_pos, "begin", 5))
    {
      // Error stuff.
      return -2;
    }

  buf_pos += 5;

  while (isspace (*buf_pos))
    buf_pos++;

  while (1)
    {
      char * tmp_str, * cur_key;
      // Determine how to handle the current keyword.

      int pos = 0;
      while (isalnum (buf_pos[pos]) || buf_pos[pos] == '_')
	pos++;

      cur_key = (char *) calloc (pos + 1, sizeof (char));
      CHECK_ALLOC (cur_key, -1);
      strncpy (cur_key, buf_pos, pos);
      cur_key[pos] = '\0';

      // Determine the keyword.

      if (!strcmp (cur_key, "end"))
	{
	  free (cur_key);
	  break;
	}

      pos = 0;
      tmp_str = NULL;

      for (i = 0; i < KF_NUM_FUNCS; i++)
	{
	  if (!strcmp (cur_key, kfs[i].key))
	    {
	      pos = kfs[i].func (buf_pos, &tmp_str);
	      if (pos == -1)
		return -1;
	      break;
	    }
	}

      // Might need to parse brackets in the event of 'text', 'section', etc.

      for (i = 0; null_keys[i]; i++)
	{
	  if (!strcmp (cur_key, null_keys[i]))
	    {
	      pos = isar_parse_null (buf_pos);
	      if (pos == -1)
		return -1;
	      break;
	    }
	}

      if (!null_keys[i])
	{
	  if (!strcmp (cur_key, "datatype"))
	    {
	      pos = isar_parse_datatype (buf_pos, refs);
	      if (pos == -1)
		return -1;
	    }
	  else if (!strcmp (cur_key, "lemma") || !strcmp (cur_key, "theorem"))
	    {
	      char * mod_str, * tmp_mod_str;
	      tmp_mod_str = die_spaces_die (tmp_str);
	      if (!tmp_mod_str)
		return -1;

	      chk = parse_connectives (tmp_mod_str, 0, &mod_str);
	      if (chk == -1)
		return -1;
	      free (tmp_mod_str);

	      printf ("lemma = '%s'\n", mod_str);
	      chk = vec_str_add_obj (lms, mod_str);
	      if (chk < 0)
		return -1;
	      free (mod_str);
	    }
	  else if (tmp_str)
	    {
	      chk = vec_str_add_obj (refs, tmp_str);
	      if (chk < 0)
		return -1;
	    }
	}

      if (pos == 0)
	{
	  // Not a recognized keyword.
	}

      while (strncmp (buf_pos + pos, "\n\n", 2))
	buf_pos++;

      // Find the next non-whitespace character.
      while (isspace(buf_pos[pos]))
	pos++;

      buf_pos += pos;
      free (tmp_str);
      free (cur_key);
    }

  // Construct the proof.

  for (i = 0; i < refs->num_stuff; i++)
    {
      sen_data * sd;
      sd = sen_data_init (i, -1, vec_str_nth (refs, i), NULL, 1, NULL, 0, 0, NULL);
      if (!sd)
	return -1;

      ls_chk = ls_push_obj (proof->everything, sd);
      if (!ls_chk)
	return -1;
    }

  int ln_offset = refs->num_stuff;

  for (i = 0; i < lms->num_stuff; i++)
    {
      sen_data * sd;
      sd = sen_data_init (i + ln_offset, -1, vec_str_nth (lms, i),
			  NULL, 0, NULL, 0, 0, NULL);
      if (!sd)
	return -1;

      ls_chk = ls_push_obj (proof->everything, sd);
      if (!ls_chk)
	return -1;

      ls_chk = ls_push_obj (proof->goals, vec_str_nth (lms, i));
      if (!ls_chk)
	return -1;
    }

  //destroy_str_vec (refs);
  //destroy_str_vec (lms);
  free (buffer);

  //destroy_vec (seqs);
  return 0;
}

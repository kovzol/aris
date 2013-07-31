#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <gtk/gtk.h>

#include "vec.h"
#include "process.h"
#include "interop-isar.h"
#include "list.h"
#include "proof.h"
#include "sen-data.h"

static const char * greek_syms[] = {
  "\\<alpha>", "\\<beta>", "\\<gamma>",
  "\\<delta>", "\\<epsilon>", "\\<zeta>",
  "\\<eta>", "\\<theta>", "\\<iota>", "\\<kappa>",
  "\\<mu>", "\\<nu>", "\\<xi>", "\\<pi>", "\\<rho>",
  "\\<sigma>", "\\<tau>", "\\<upsilon>", "\\<phi>",
  "\\<chi>", "\\<psi>", "\\<omega>", "\\<Gamma>",
  "\\<Delta>", "\\<Theta>", "\\<Lambda>", "\\<Xi>",
  "\\<Pi>", "\\<Sigma>", "\\<Upsilon>", "\\<Phi>",
  "\\<Psi>", "\\<Omega>", NULL
};

static char * latin_regexp = "[a-zA-Z]";
static char * digit_regexp = "[0-9]";
static char * sym_regexp = "[!#$%&*+-/<=>?@^_|~]";
static char * string_regexp = "\"[^\"]\"";
static char * astring_regexp = "\'[^\']\'";
static char * letter_regexp, * greek_regexp, * nat_regexp,
  * ql_regexp, * float_regexp, * ident_regexp, * lident_regexp,
  * symident_regexp, * var_regexp;

#define ISABELLE_PATH "/home/skolar/source/isar/Isabelle2013/bin/"
#define ISABELLE_EXEC "/home/skolar/source/isar/Isabelle2013/bin/isabelle-process"

static const char * is_args[3] = {ISABELLE_EXEC,
				  "-I",
				  NULL};

int
isar_run_cmds (unsigned char ** inputs, unsigned char ** output)
{
  GPid new_pid;
  int sout, sin;
  GError * g_err;

  g_spawn_async_with_pipes (ISABELLE_PATH,
			    (gchar **) is_args,
			    NULL,
			    0,
			    NULL, NULL,
			    &new_pid,
			    &sin, &sout, NULL,
			    &g_err);

  FILE * in_stream, * out_stream;
  in_stream = fdopen (sin, "w");
  int i;
  for (i = 0; inputs[i]; i++)
    {
      fprintf (in_stream, "%s;\r\n", inputs[i]);
    }
  fclose (in_stream);

  out_stream = fdopen (sout, "r");
  char c;
  vec_t * c_vec;
  c_vec = init_vec (sizeof(char));
  if (!c_vec)
    return -1;

  while ((c = fgetc(out_stream)) != EOF)
    {
      char tmp_c = c;
      int v_ret;
      v_ret = vec_add_obj (c_vec, &tmp_c);
      if (v_ret == -1)
	return -1;
    }
  fclose (out_stream);

  *output = (unsigned char *) calloc (c_vec->num_stuff + 1, sizeof (char));
  CHECK_ALLOC ((*output), -1);

  for (i = 0; i < c_vec->num_stuff; i++)
    {
      char * tmp_c = vec_nth (c_vec, i);
      (*output)[i] = (unsigned char) *tmp_c;
    }

  (*output)[i] = '\0';
  destroy_vec (c_vec);

  g_spawn_close_pid (new_pid);

  return 0;
}


int
isar_prep_regexps ()
{
  /*
  int l_len, d_len, s_len, lt_len, gr_len, nat_len, ql_len,
    var_len;

  int tf_len, tv_len;
  char * tf_regexp, * tv_regexp;

  l_len = 8, d_len = 5, s_len = 20;

  nat_regexp = (char *) calloc (d_len + 2, sizeof (char));
  CHECK_ALLOC (nat_regexp, -1);
  nat_len = sprintf (nat_regexp, "%s+", digit_regexp);

  letter_regexp = (char *) calloc (l_len * 4 + gr_len + 32,
				   sizeof (char));
  CHECK_ALLOC (letter_regexp, -1);
  lt_len = sprintf (letter_regexp,
		    "%s|\\\\<%s>|\\\\<%s%s>|%s|\\\\<^isub>|\\\\<^isup>",
		    latin_regexp, latin_regexp, latin_regexp,
		    latin_regexp, greek_regexp);

  sprintf (float_regexp, "%s.%s|-%s.%s",
	   nat_regexp, nat_regexp,
	   nat_regexp, nat_regexp);

  sprintf (ql_regexp, "%s|%s|_|'",
	   letter_regexp, digit_regexp);

  sprintf (ident_regexp, "%s%s*", letter_regexp,
	   ql_regexp);

  sprintf (lident_regexp, "%s(.%s)+", ident_regexp, ident_regexp);

  sprintf (symident_regexp, "%s+ | \\\\<%s>",
	   sym_regexp,
	   ident_regexp);

  var_len = sprintf (var_regexp, "?%s|?%s.%s", ident_regexp,
		     ident_regexp, nat_regexp);

  tf_len = sprintf (tf_regexp, "'%s", ident_regexp);

  tv_len = sprintf (tv_regexp, "?%s|?%s.%s", tf_regexp, tf_regexp, nat_regexp);
  */

  return 0;
}

int
is_latin (char * input)
{
  return isalpha (*input);
}

int
is_greek (char * input)
{
  int i;
  for (i = 0; greek_syms[i]; i++)
    {
      if (!strcmp (input, greek_syms[i]))
	break;
    }

  return (greek_syms[i] != NULL);
}

int
is_sym (char * input)
{
  return ispunct (*input);
}

int
is_digit (char * input)
{
  return isdigit (*input);
}

int
is_nat (char * input)
{
  int i;
  for (i = 0; input[i]; i++)
    {
      if (!is_digit (input + i))
	break;
    }

  return (input[i] == '\0');
}

int
is_float (char * input)
{
  char * chk;
  int chk_0, chk_1;
  // nat.nat | -nat.nat
  chk = strtok (input, ".");
  if (chk[0] == '-')
    chk += 1;

  chk_0 = is_nat (chk);
  if (!chk_0)
    return 0;

  chk = strtok (NULL, ".");
  chk_1 = is_nat (chk);

  return chk_1;
}

int
is_letter (char * input)
{
  int check, chk, chk_0, chk_1;
  check = is_latin (input);
  chk = (input[0] == '\\'
	 && input[1] == '<'
	 && is_latin (input + 2));

  chk_0 = (chk
	   && input[3] == '>'
	   && input[4] == '\0');
  chk_1 = (chk
	   && input[3] == ' '
	   && is_latin (input + 4)
	   && input[5] == '>'
	   && input[6] == '\0');

  if (check || chk_0 || chk_1)
    return 1;

  check = is_greek (input);
  if (check)
    return 1;

  chk = !strcmp (input, "\\<^isub>")
    || !strcmp (input, "\\<^isup>");

  if (chk)
    return 1;

  return 0;
}

int
is_quasiletter (char * input)
{
  return (is_letter (input) || is_digit (input)
	  || *input == '_' || *input == '\'');
}

int
parse_verbatim (const unsigned char * in_str, const int init_pos,
		unsigned char ** out_str)
{
    //The opening and closing verbatim count.
  int o_verb, c_verb;

  //The offset of the last verbs.
  int verb_pos;

  //Temporary strings that take the return from strchr.
  unsigned char * o_str, * c_str;

  o_verb = 1;
  c_verb = 0;

  verb_pos = init_pos + 1;

  while (o_verb != c_verb)
    {
      //Find the next opening and closing verbtheses.
      o_str = (unsigned char*) strstr ((const char *) in_str + verb_pos, "{*");
      c_str = (unsigned char*) strstr ((const char *) in_str + verb_pos, "*}");

      //If the offset of c_str from in_str,
      // is less than the offset of o_str from in_str,
      //then this is a closing verbtheses.
      if (c_str != NULL
	  && (o_str == NULL || (c_str - in_str) < (o_str - in_str)))
	{
	  c_verb += 1;
	  verb_pos = c_str - in_str + 2;
	}
      else if (o_str != NULL)
	{
	  o_verb += 1;
	  verb_pos = o_str - in_str + 2;
	}
      else
	{
	  //If both strings are NULL, then return -1.
	  return -1;
	}
    }

  verb_pos--;

  if (out_str)
    {
      // Allocate enough room for out_str,
      // and copy the verbtheses construct from in_str.
      *out_str = (unsigned char*) calloc (verb_pos - init_pos + 2, sizeof (char));
      CHECK_ALLOC (*out_str, -2);
      strncpy (*out_str, in_str + init_pos, verb_pos - init_pos + 1);
      (*out_str)[verb_pos - init_pos + 1] = '\0';
    }

  //Return verb_pos.
  return verb_pos;
}

/*
  verbatim: {* [^"*}"] *}  // Not valid regexp

  name: ident | symident | string | nat
  parname: \( name \)
  nameref: name | longident

  int: nat | -nat
  real: float | int

  text: verbatim : nameref
  comment: -- text

  classdecl: name (< | \subseteq) nameref (,nameref)*
  sort: nameref
  arity: \( sort (,sort)* \) sort

  type: nameref | typefree | typevar
  term: nameref | var
  prop: term

  inst: _ | term
  insts: inst*

  typespec: (typefree | \( typefree (,typefree)* \))name
  typespec_sorts: (typefree (:: sort)? | \( typefree (:: sort)? (, typefree (:: sort)?)* \)) name

  term_pat: \( ('is' term)+ \)
  prop_pat: \( ('is' prop)+ \)

  vars: name+ (:: type)?
  props: (thmdecl)? (prop (prop_pat)?)+

  atom: nameref | typefree | typevar | var | nat | float | keyword
  arg: atom | \( args \) | \[ args \]
  args: arg*
  attributes: \[ (nameref args (, nameref args)*)? \]

  axmdecl: name (attributes)? ':'
  thmdecl: thmbind ':'
  thmdef: thmbind '='
  thmref: ( (nameref (selection)? | altstring) (attributes)? | \[ attributes \])
  thmrefs: thmref+
  thmbind: (name attributes | name | attributes)
  selection: \( (nat | nat '-' (nat)?) (, (nat | nat '-' (nat)?))* \)


  ('chapter' | 'section' | 'subsection' | 'subsubsection' | 'text') target? text
  ('header' | 'text_raw' | 'sect' | 'subsect' | 'subsubsect' | 'txt' | 'txt_raw') text
  '@{' antiquotation '}'
  antiquotation:
  ('theory' options name
  | 'thm' options styles thmrefs
  | 'lemma' options prop 'by' method method?
  | 'prop' options styles prop
  | 'term' options styles term
  | 'value' options styles term
  | 'term_type' options styles term
  | 'typeof' options styles term
  | 'const' options term
  | 'abbrev' options term
  | 'typ' options name
  | 'type' options name
  | 'class' options name
  | 'text' options name
  | 'goals' options
  | 'subgoals' options
  | 'prf' options thmrefs
  | 'full_prf' options thmrefs
  | 'ML' options name
  | 'ML_type' options name
  | 'ML_struct' options name
  | 'file' options name)

  options: \[ option (, option)* \]
  option: (name | name = name)
  styles: \( style (, style)* \)
  style: name+

  tags: tag*
  tag: '%' (ident | string)

  rule, body, concatenation, atom: 71-72

  'theory' name 'imports' name+ uses? 'begin'
  uses: 'uses' (name | parname)+
  'context' name 'begin'
  target: \( 'in' name \)

  'axiomatization' fixes? ('where' specs)?
  'definition' target? (decl 'where')? thmdecl? prop
  'abbreviation' target? mode? (decl 'where')? prop
  fixes: (name (::type)? (mixfix)? | vars) ('and' (name (::type)? mixfix? | vars))*
  specs: thmdecl? props ('and' thmdecl? props)*
  decl: name (::type)? mixfix?

  ('declaration' | 'syntax_declaration') (\( 'pervasive' \))? target? text
  'declare' target? thmrefs ('and' thmrefs)*


  locale_expr, instance, qualifier, pos_insts, named_insts: 83

  ---------

  context_elem: ('fixes' fixes ('and' fixes)* | 'constrains' name::type ('and' name::type)*
  | 'assumes' props ('and' props)* | 'defines' thmdecl? prop prop_pat? ('and' thmdecl? prop prop_pat?)*
  | 'notes' thmdef? thmrefs ('and' thmdef? thmrefs))

  equations: 88

  'class' class_spec 'begin'?
  class_spec: name = (nameref '+' context_elem+ | nameref | context_elem+)
  'instantiation' nameref ('and' nameref)* :: arity 'begin'
  'instance' (nameref ('and' nameref)* ::arity | nameref ('<' | \subseteq) nameref)
  'subclass' target? nameref

  spec: 94

  'type_synonym' typespec = type mixfix?
  'typedecl' typespec mixfix?
  'arities' (nameref::arity)+

  'consts' (name::type mixfix?)+
  'defs' opt? (axmdecl prop)+
  opt: \( 'unchecked' 'overloaded' \)

  'axioms' (axmdecl prop)+
  ('lemmas' | 'theorems') target? thmdef? thmrefs ('and' thmdef? thmrefs)

  'notepad' 'begin'
  'end'

  'fix' vars ('and' vars)*
  ('assume' | 'presume') props ('and' props)*
  'def' def ('and' def)*
  def: thmdecl? name ('==' | \cong) term term_pat?

  'let' term ('and' term)* = term ('and' term ('and' term)* = term)*

  'note' thmdef? thmrefs ('and' thmdef? thmrefs)
  ('from' | 'with' | 'using' | 'unfolding') thmrefs ('and' thmrefs)*

  ('lemma' | 'theorem' | 'corollary' | 'schematic_lemma' | 'schematic_theorm' | 'schematic_corollary') target? (goal | longgoal)
  ('have' | 'show' | 'hence' | 'thus') goal
  'print_statement' modes? thmrefs
  goal: props ('and' props)*
  longgoal: thmdecl? context_elem* conclusion
  ('shows' goal | 'obtains' parname? case ('|' parname? case))
  case: vars ('and' vars)* 'where' props ('and' props)*

  method: (nameref | '(' methods ')') ('?' | '+' | '[' nat? ']')?
  methods: (nameref args | method) ((',' | '|') (nameref args | method))*

  goal_spec: '[' ((nat '-' nat) | (nat '-') | nat | '!') ']'

  'proof' method?
  'qed' method?
  'by' method method?
  '.' | '..' | 'sorry'

  'fact' thmrefs?
  'rule' thmrefs?
  rulemod: ('intro' | 'elim' | 'dest') ('del' | (('!' | '?')? nat?)) ':' thmrefs
  ('intro' | 'elim' | 'dest') ('!' | '?')? nat?
  'rule' 'del'
  'OF' thmrefs
  'of' insts ('concl' ':' insts)?

  'where' (((name | var | typefree | typevar) '=' (type | term)) ('and' ((name | var | typefree | typevar) '=' (type | term)))*)?

  ('apply' | 'apply_end') method
  'defer' nat?
  'prefer' nat

  'method_setup' name '=' text text?

  'obtain' parname? vars ('and' vars)* 'where' props ('and' props)*
  'guess' vars ('and' vars)

  ('also' | 'finally') ('(' thmrefs ')')?
  'trans' ('add' | 'del')?

  'case' (caseref | '(' caseref ('_' | name)+ ')')
  caseref: nameref attributes?

  'case_names' (name ('[' ('_' | name)+ ']')?)+
  'case_conclusion' name name*
  'params' name* ('and' name*)*
  'consumes' nat?

  'cases' ('(' 'no_simp' ')')? (insts ('and' insts)*)? rule?
  ('induct' | 'induction') ('(' 'no_simp' ')')? (definsts ('and' definsts)*)? arbitrary? taking? rule?
  'coinduct' insts taking rule?
  rule: ('rule' ':' thmref+) | (('type' | 'pred' | 'set') ':' nameref+)
  definst: (name ('==' | \cong) term) | ('(' term ')') | inst
  definsts: definst*
  arbitrary: 'arbitrary' ':' (term* 'and')+
  taking: 'taking' ':' insts

  'cases' spec
  'induct' spec
  'coinduct' spec
  spec: 'del' | (('type' | 'pred' | 'set') ':' nameref)

  'typ' modes? type
  'term' modes? term
  'prop' modes? prop
  'thm' modes? thmrefs
  ('prf' | 'full_prf') modes? thmrefs?
  'pr' modes? nat?
  modes: '(' name+ ')'

  7.1.2 (138 theirs, not mine)

  infix: '(' ('infix' | 'infxl' | 'infixr') string nat ')'
  mixfix: infix | ('(' string prios? nat? ')') | ('(' 'binder' string prios? nat ')')
  struct_mixfix: mixfix | ('(' 'structure' ')')
  prios: '[' nat (',' nat)* ']'

  'simp' | 'cong' | 'split' ('add' | 'del')?

  'simproc_setup' name '(' term ('|' term)* ') '=' text ('identifier' nameref+)?
  'simproc' ('del' ':' | 'add' ':')? name+

  'simplified' opt? thmrefs?
  '(' ('no_asm' | 'no_asm_simp' | 'no_asm_use') ')'

  ('intro' | 'elim' | 'dest') ('!' | '?')? nat?
  'rule' 'del'
  'iff' ('del' | 'add'? '?'?)
  'rule' thmrefs?

  'blast' nat? clamod*
  'auto' (nat nat)? clasimpmod*
  'force' clasimpmod*
  ('fast' | 'slow' | 'best') clamod*
  ('fastfoce' | 'slowsimp' | 'bestsimp') clasimpmod*
  'deepen' nat? clamod*
  clamod: ('del' | (('intro' | 'elim' | 'dest') ('!' | '?')?)) ':' thmrefs
  clasimpmod: (('simp' ('add' | 'del' | 'only')?) | (('cong' | 'split') ('add' | 'del')?)
  | ('iff' ('del' | ('add'? '?'?)))
  | ('del' | (('intro' | 'elim' | 'dest') ('!' | '?')?))) ':' thmrefs

  ('safe' | 'clarify') clamod*
  'clarsimp' clasimpmod*

  194 (mine)

  'primrec' target? fixes 'where' equations
  ('fun' | 'function') target? functionopts? fixes 'where' equations
  equations: thmdecl? prop ('|' thmdecl? prop)*
  functionopts: '(' ('sequential' | 'domintros') (',' ('sequential' | 'domintros')) ')'
  'termination' term?

 */


// Returns the position of the end of the markup.
int
isar_parse_markup (char * input_str)
{
  int pos = 0;
  // Skip over the keyword.
  while (!isspace(input_str[pos]))
    pos++;
  // Depending on the command, it may have an optional name afterwards.
  // Will end up at {*,*} pair.
  // Skip the whitespace.
  while (isspace(input_str[pos]))
    pos++;

  int ret;
  ret = parse_tags (input_str, pos, NULL, "{*", "*}");
  if (ret == -1)
    return -1;

  return ret;
}

// only really need the imports.
// theory name imports keywords? 'begin'
int
isar_parse_theory (char * input_str)
{
  // start of input_str is 'theory';
  // next will be the name.
  // That's not exactly important though.
  // Mostly want the includes.
}

int
isar_parse_axiomatiziation (char * input_str)
{
  return 0;
}

// This will need return values.
int
isar_parse_definition (char * input_str)
{
  // Basically definition c where t ==> c \equiv t
  // For Aris, this will most likely be (<b> c t)
  return 0;
}

int
isar_parse_abbreviation (char * input_str)
{
  // Similar to parse_definition.
  return 0;
}

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

/* Definitions of Aris' processing engine.

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

#ifndef ARIS_PROC_H
#define ARIS_PROC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "typedef.h"


#define ISLEGIT(c) (islower (c) || isdigit (c) || c == '_')

#define ISSEP(c) (c == ')' || c == ',' || c == '+' || c == '*' || c == '(')

#define IS_TYPE_CONN(s,t) (!strncmp (s, t.and, t.cl)		\
			   || !strncmp (s, t.or, t.cl)		\
			   || !strncmp (s, t.not, t.nl)		\
			   || !strncmp (s, t.con, t.cl)		\
			   || !strncmp (s, t.bic, t.cl)		\
			   || !strncmp (s, t.unv, t.cl)		\
			   || !strncmp (s, t.exl, t.cl)		\
			   || !strncmp (s, t.tau, t.cl)		\
			   || !strncmp (s, t.ctr, t.cl)		\
			   || !strncmp (s, t.elm, t.cl)		\
			   || !strncmp (s, t.nil, t.cl))

#define IS_BIN_CONN(s) (!strncmp (s,AND,CL)	\
			|| !strncmp (s,OR,CL)	\
			|| !strncmp (s,CON,CL)	\
			|| !strncmp (s,BIC,CL))

#define ISCONN(s) IS_TYPE_CONN (s, main_conns)

#define ISGOOD(s) (!strncmp (s, UNV, CL)     \
		   || !strncmp (s, EXL, CL)  \
		   || *(s) == '('	     \
		   || isupper (*(s))	     \
		   || !strncmp (s, NOT, NL)  \
		   || !strncmp (s, CTR, CL)  \
		   || !strncmp (s, NIL, CL)  \
		   || !strncmp (s, TAU, CL))

#define ISSBOOL(s) (!strncmp (s, S_TAU, S_CL) || !strncmp (s, S_CTR, S_CL))


enum CONN_ORDER {
  AND_CONN=0,
  OR_CONN,
  NOT_CONN,
  CON_CONN,
  BIC_CONN,
  UNV_CONN,
  EXL_CONN,
  TAU_CONN,
  CTR_CONN,
  ELM_CONN,
  NIL_CONN,
  NUM_CONNS
};

// The struct for connectives handling.

struct connectives_list {
  char * and;
  char * or;
  char * not;
  char * con;
  char * bic;
  char * unv;
  char * exl;
  char * tau;
  char * ctr;
  char * elm;
  char * nil;
  int cl;
  int nl;
};

// Command-line interface connectives.

static struct connectives_list cli_conns = {
  "&", "|", "~", "$", "%", "@", "#", "!", "^", ":", ">", 1, 1
};

// GUI connectives.

// Windows still seems to refuse to display these properly.
// And I'm still out of ideas as to why.
// wxWidgets got it to work, although I'm not entirely certain how.
// Something about the unicode support.
// It also works just fine under Wine, which is why it's hard to test it.
// Gtk only displays utf-8 strings.  Maybe that has something to do
//  with it.
// Got a work around - using pixbufs.

static struct connectives_list gui_conns = {
  "\u2227",
  "\u2228",
  "\u00AC",
  "\u2192",
  "\u2194",
  "\u2200",
  "\u2203",
  "\u22A4",
  "\u22A5",
  "\u2208",
  "\u2349",
  3, 2
};


static const char * conn_list_back[] = {
  "\u2227",
  "\u2228",
  "\u00AC",
  "\u2192",
  "\u2194",
  "\u2200",
  "\u2203",
  "\u22A4",
  "\u22A5",
  "\u2208",
  "\u2349"
};

static const char * conn_list[] = {
  "&", "|", "~", "$", "%", "@", "#", "!", "^", ":", ">"
};

// Sexpr connectives.

static struct connectives_list sexpr_conns = {
  "<a>",
  "<o>",
  "<n>",
  "<i>",
  "<b>",
  "<u>",
  "<e>",
  "<t>",
  "<c>",
  "<l>",
  "<d>",
  3, 3
};

// The main connectives.

struct connectives_list main_conns;

// Definitions.

#define AND main_conns.and
#define OR main_conns.or
#define NOT main_conns.not
#define CON main_conns.con
#define BIC main_conns.bic
#define UNV main_conns.unv
#define EXL main_conns.exl
#define TAU main_conns.tau
#define CTR main_conns.ctr
#define ELM main_conns.elm
#define NIL main_conns.nil
#define CL main_conns.cl
#define NL main_conns.nl

#define U_AND (unsigned char *) AND
#define U_OR (unsigned char *) OR
#define U_NOT (unsigned char *) NOT
#define U_CON (unsigned char *) CON
#define U_BIC (unsigned char *) BIC
#define U_UNV (unsigned char *) UNV
#define U_EXL (unsigned char *) EXL
#define U_TAU (unsigned char *) TAU
#define U_CTR (unsigned char *) CTR
#define U_ELM (unsigned char *) ELM
#define U_NIL (unsigned char *) NIL

// Commonly used error messages.

#define NOT_MINE _("This rule is not one of mine")
#define CORRECT _("Correct!")
#define NO_DIFFERENCE _("No difference was found in the reference and conclusion.")
#define SAME_LENGTH _("The reference and conclusion must not be the same length")

// Sentence ids.

enum SEN_IDS {
  SEN_ID_START = 1,
  SEN_ID_OPAREN = 0,
  SEN_ID_CPAREN = -1,
  SEN_ID_NOT = -2,
  SEN_ID_AND = -3,
  SEN_ID_OR = -4,
  SEN_ID_CON = -5,
  SEN_ID_BIC = -6,
  SEN_ID_UNV = -7,
  SEN_ID_EXL = -8,
  SEN_ID_SPACE = -9,
  SEN_ID_EQ = -10,
  SEN_ID_LT = -11,
  SEN_ID_ELM = -12,
  SEN_ID_NIL = -13,
  SEN_ID_END = -14
};

// Sentence id structure.

struct sen_id {
  unsigned char * sen;
  int id;
};

/* Parse functions. */

int parse_parens (const unsigned char * in_str,
		  const int init_pos,
		  unsigned char ** out_str);

int reverse_parse_parens (const unsigned char * in_str,
			  const int init_pos,
			  unsigned char ** out_str);

int parse_tags (const unsigned char * in_str,
		const int init_pos,
		unsigned char ** out_str,
		const char * o_tag, const char * c_tag);

/* Check functions. */

int check_parens (const unsigned char * chk_str);

int check_sides_conn (const unsigned char * chk_str,
		       const unsigned int init_pos);

int check_conns (const unsigned char * chk_str);

int check_sides_quant (const unsigned char * chk_str,
			const unsigned int init_pos);

int check_quants (const unsigned char * chk_str);

int check_text (unsigned char * text);

int check_generalities (unsigned char * text);

int check_symbols (unsigned char * in_str, int pred);

int check_infix (unsigned char * in_str, int pred);

// Helper functions

unsigned char * die_spaces_die (unsigned char * in_str);
unsigned char * remove_comment (unsigned char * in_str);
unsigned char * format_string (unsigned char * in_str);

int find_difference (unsigned char * sen_0, unsigned char * sen_1);

unsigned char * elim_not (const unsigned char * not_str);

unsigned char * elim_par (const unsigned char * par_str);

int get_gen (unsigned char * in_str,
	     int in_pos,
	     unsigned char ** out_str);

int get_generalities (unsigned char * chk_str,
		      unsigned char * conn,
		      vec_t * vec);

// Process functions

char * process (unsigned char * conc,
		vec_t * prems,
		const char * rule,
		vec_t * vars,
		proof_t * proof);


char * process_inference (unsigned char * conc,
			  vec_t * prems,
			  const char * rule);

char * process_equivalence (unsigned char * conc,
			    vec_t * prems,
			    const char * rule);

char * process_quantifiers (unsigned char * conc,
			    vec_t * prems,
			    const char * rule,
			    vec_t * vars);

char * process_misc (unsigned char * conc,
		     vec_t * prems,
		     const char * rule,
		     vec_t * vars,
		     proof_t * proof);

char * process_bool (unsigned char * conc,
		     vec_t * prems,
		     const char * rule);


// Sexpr conversion functions.

unsigned char * convert_sexpr (unsigned char * in_str);

int get_pred_func_args (unsigned char * in_str, int init_pos,
			unsigned char ** sym, vec_t * args);

unsigned char * infix_to_prefix (unsigned char * in_str);

unsigned char * infix_to_prefix_func (unsigned char * in_str);

#endif  /* ARIS_PROC_H */

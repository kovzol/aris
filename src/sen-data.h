/* The sentence data structure.

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

#ifndef ARIS_SEN_DATA_H
#define ARIS_SEN_DATA_H

#include "typedef.h"

#define SEN_TAB "    "
#define SEN_COMMENT_CHAR ';'

// Different value types.

enum VALUE_TYPES {
  VALUE_TYPE_BLANK = 0,
  VALUE_TYPE_TRUE,
  VALUE_TYPE_FALSE,
  VALUE_TYPE_ERROR,
  VALUE_TYPE_REF,
  VALUE_TYPE_RULE
};

// The sentence data structure.
struct sen_data {
  int line_num;           // Keeps track of the line number.
  int rule;               // Index of the rule of this sentence.
  unsigned char * text;   // Contains the text of this item.
  unsigned char * sexpr;  // Sexpr text.

  int premise : 1;   // Whether or not this sentence is a premise.
  int subproof : 1;  // Whether or not this sentence starts a subproof.
  int depth;         // The depth of this sentence.  0 for all top levels.
  int * indices;     // The line numbers of the subproofs that contain this sentence.

  short * refs;            // A list of sentences that are references.

  unsigned char * file;   // The file name if lemma is used on this sentence.
};

#define SEN_DATA_DEFAULT(p,s,d) sen_data_init (-1, -1, NULL, NULL, p, NULL, s, d, NULL)

#define DEPTH_DEFAULT -2

sen_data * sen_data_init (int line_num, int rule, unsigned char * text,
			  short * refs, int premise, unsigned char * file,
			  int subproof, int depth, unsigned char * sexpr);
void sen_data_destroy (sen_data * sd);

int sen_convert_sexpr (unsigned char * text, unsigned char ** sexpr);

char * sen_data_evaluate (sen_data * sd, int * ret_val,
			  list_t * vars, list_t * lines);
int sen_data_can_select_as_ref (sen_data * sen, sen_data * ref);
int sen_data_can_sel_as_ref (int sen_line, int * sen_indices,
			     int ref_line, int * ref_indices,
			     int ref_prem);

#endif /* ARIS_SEN_DATA_H */

/* Functions and tags for opening and saving files.

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

#ifndef ARIS_INTEROP_ISAR_H
#define ARIS_INTEROP_ISAR_H

struct input_type {
  char * type;
  char * seq;
};

struct key_function {
  char * key;
  int (* func) (char *, char **);
};

typedef struct input_type in_type;
typedef struct key_function key_func;
typedef struct proof proof_t;

enum KEY_FUNCS {
  KF_SYN = 0,
  KF_FUN,
  KF_CASE,
  KF_PRIMREC,
  KF_DEF,
  KF_LEMMA,
  KF_THEOREM,
  KF_NUM_FUNCS,
};

unsigned char * correct_conditionals (unsigned char * in_str);
unsigned char * isar_to_aris (char * isar);
int parse_thy (char * filename, proof_t * proof);
int parse_connectives (char * in_str, int cur_conn, char ** out_str);
int parse_pred_func (char * in_str, char ** out_str);

int isar_parse_syn (char * syn, char ** out_str);
int isar_parse_fun (char * fun, char ** out_str);
int isar_parse_lemma (char * lemma, char ** out_str);
int isar_parse_theorem (char * thm, char ** out_str);
int isar_parse_case (char * cs, char ** out_str);
int isar_parse_primrec (char * primrec, char ** out_str);
int isar_parse_def (char * def, char ** out_str);
int isar_parse_datatype (char * dt, vec_t * constructors);

static key_func kfs[KF_NUM_FUNCS] = {
  "type_synonym", isar_parse_syn,
  "fun", isar_parse_fun,
  "case", isar_parse_case,
  "primrec", isar_parse_primrec,
  "definition", isar_parse_def,
  "lemma", isar_parse_lemma,
  "theorem", isar_parse_theorem,
};

#endif  /*  ARIS_INTEROP_ISAR_H  */

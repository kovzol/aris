#ifndef INTEROPISAR_H
#define INTEROPISAR_H

#ifdef __cplusplus
extern "C" {
#endif
#include "typedef.h"

struct input_type {
    char * type;
    char * seq;
};

struct key_function {
    char * key;
    int (* func) (char *, char **);
};


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

int isar_run_cmds (unsigned char ** inputs, unsigned char ** output);

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

/*
const char * marksups_target[] = {
  "chapter",
  "section",
  "subsection",
  "subsubsection",
  "text"
};

const char * marksups[] = {
  "header",
  "text_raw",
  "sect",
  "subsect",
  "subsubsect",
  "txt",
  "txt_raw"
};
*/

static key_func kfs[KF_NUM_FUNCS] = {
    "type_synonym", isar_parse_syn,
    "fun", isar_parse_fun,
    "case", isar_parse_case,
    "primrec", isar_parse_primrec,
    "definition", isar_parse_def,
    "lemma", isar_parse_lemma,
    "theorem", isar_parse_theorem,
    /*
  "header", isar_parse_markup,
  "section", isar_parse_markup,
  "subsection", isar_parse_markup,
  "subsubsection", isar_parse_markup
  */
};

struct regexp_assoc {
    char * ident;
    char * regexp;
};

/*
struct regexp_assoc reg_assocs[] = {
  "greek", "(\\<alpha> | \\<beta> | \\<gamma> | \\<delta> | \\<epsilon>\
 | \\<zeta> | \\<eta> | \\<theta> | \\<iota> | \\<kappa> | \\<mu>      \
 | \\<nu> | \\<xi> | \\<pi> | \\<rho> | \\<sigma> | \\<tau>	       \
 | \\<upsilon> | \\<phi> | \\<chi> | \\<psi> | \\<omega> | \\<Gamma>   \
 | \\<Delta> | \\<Theta> | \\<Lambda> | \\<Xi> | \\<Pi> | \\<Sigma>    \
 | \\<Upsilon> | \\<Phi> | \\<Psi> | \\<Omega>)"
  "sym", "[!#$%&*+-/<=>?@^_|~]"
  "digit", "[0-9]",
  "latin", "[a-zA-Z]"
};
*/
#ifdef __cplusplus
}
#endif

#endif // INTEROPISAR_H

/* Definitions of aris' sexpr processing engine.

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

#ifndef ARIS_SEXPR_PROCESS_H
#define ARIS_SEXPR_PROCESS_H

#include "process.h"
#include "typedef.h"

#define S_AND sexpr_conns.and
#define S_OR  sexpr_conns.or
#define S_NOT sexpr_conns.not
#define S_CON sexpr_conns.con
#define S_BIC sexpr_conns.bic
#define S_UNV sexpr_conns.unv
#define S_EXL sexpr_conns.exl
#define S_TAU sexpr_conns.tau
#define S_CTR sexpr_conns.ctr
#define S_ELM sexpr_conns.elm
#define S_NIL sexpr_conns.nil
#define S_CL  sexpr_conns.cl
#define S_NL  sexpr_conns.nl

int sexpr_get_part (unsigned char * in_str,
		    unsigned int init_pos,
		    unsigned char ** out_str);

unsigned char * sexpr_car (unsigned char * in_str);

unsigned char * sexpr_cdr (unsigned char * in_str);

int sexpr_car_cdr (unsigned char * in_str,
		   unsigned char ** car,
		   vec_t * cdr);

int sexpr_str_car_cdr (unsigned char * in_str,
		       unsigned char ** car,
		       unsigned char ** cdr);

void sen_put_len (unsigned char * in0, unsigned char * in1,
		  unsigned char ** sh_sen, unsigned char ** ln_sen);

unsigned char *
construct_other (unsigned char * main_str,
		 int init_pos,
		 int fin_pos,
		 int alloc_size,
		 char * template,
		 ...);

int sexpr_not_check (unsigned char * in_str);

unsigned char * sexpr_add_not (unsigned char * in_str);

unsigned char * sexpr_elim_not (unsigned char * in_str);

int sexpr_get_generalities (unsigned char * in_str, unsigned char * conn, vec_t * vec);

int sexpr_find_top_connective (unsigned char * in_str, unsigned char * conn,
			       unsigned char ** lsen, unsigned char ** rsen);

int find_unmatched_o_paren (unsigned char * in_str, int in_pos);

int sexpr_find_unmatched (unsigned char * sen_a, unsigned char * sen_b, int * ai, int * bi);

int sexpr_get_pred_args (unsigned char * in_str, unsigned char ** pred, vec_t * args);

unsigned char * sexpr_elim_quant (unsigned char * in_str, unsigned char * quant,
				  unsigned char ** var);

int sexpr_get_quant_vars (unsigned char * in_str, vec_t * vars);

int sexpr_replace_var (unsigned char * in_str, unsigned char * new_var,
		       unsigned char * old_var, vec_t * off_var,
		       unsigned char ** out_str);

int sexpr_quant_infer (unsigned char * quant_sen, unsigned char * elim_sen,
		       unsigned char * quant, int cons, vec_t * cur_vars);

int sexpr_find_vars (unsigned char * in_str, unsigned char * var, vec_t * offsets);

int sexpr_parse_vars (unsigned char * in_str, vec_t * vars, int quant);

int sexpr_collect_vars_to_proof (list_t * vars, unsigned char * text, int arb);

int sexpr_get_ids (unsigned char * sen, int ** ids, vec_t * sen_ids);

/* Inference rule functions. */

char * proc_mp (unsigned char * prem_0, unsigned char * prem_1, unsigned char * conc);

char * proc_ad (unsigned char * prem, unsigned char * conc);

char * proc_sm (unsigned char * prem, unsigned char * conc);

char * proc_cn (vec_t * prems, unsigned char * conc);

char * proc_hs (vec_t * prems, unsigned char * conc);

char * proc_ds (vec_t * prems, unsigned char * conc);

char * proc_ex (unsigned char * conc);

char * proc_cd (vec_t * prems, unsigned char * conc);

/* Equivalence rule functions */

char * proc_im (unsigned char * prem, unsigned char * conc);

char * proc_dm (unsigned char * prem, unsigned char * conc, int mode_guess);

char * proc_as (unsigned char * prem, unsigned char * conc);

char * proc_co (unsigned char * prem, unsigned char * conc);

char * proc_id (unsigned char * prem, unsigned char * conc);

char * proc_dt (unsigned char * prem, unsigned char * conc, int mode_guess);

char * proc_eq (unsigned char * prem, unsigned char * conc);

char * proc_dn (unsigned char * prem, unsigned char * conc);

char * proc_ep (unsigned char * prem, unsigned char * conc);

char * proc_sb (unsigned char * prem, unsigned char * conc);

/* Predicate rule functions. */

char * proc_ug (unsigned char * prem, unsigned char * conc, vec_t * vars);

char * proc_ui (unsigned char * prem, unsigned char * conc);

char * proc_eg (unsigned char * prem, unsigned char * conc);

char * proc_ei (unsigned char * prem, unsigned char * conc, vec_t * vars);

char * proc_bv (unsigned char * prem, unsigned char * conc);

char * proc_nq (unsigned char * prem, unsigned char * conc);

char * proc_pr (unsigned char * prem, unsigned char * conc);

char * proc_ii (unsigned char * conc);

char * proc_fv (unsigned char * prem_0,  unsigned char * prem_1, unsigned char * conc);

/* Boolean rule functions. */

char * proc_bi (unsigned char * prem, unsigned char * conc);

char * proc_bd (unsigned char * prem, unsigned char * conc);

char * proc_bn (unsigned char * prem, unsigned char * conc);

char * proc_sn (unsigned char * prem, unsigned char * conc);

/* Misc rule functions. */

char * proc_lm (vec_t * prems, unsigned char * conc, proof_t * proof);

char * proc_sp (unsigned char * prem_0, unsigned char * prem_1, unsigned char * conc);

char * proc_sq (unsigned char * conc, vec_t * vars);

char * proc_in (unsigned char * prem_0, unsigned char * prem_1, unsigned char * conc, vec_t * vars);
#endif  /*  ARIS_SEXPR_PROCESS_H  */

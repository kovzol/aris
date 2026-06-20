/* Functions for processing inference rules.

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

#include "sexpr-process.h"
#include "../src/vec.h"

char *
process_inference (unsigned char * conc, vec_t * prems, const char * rule)
{
    char * ret = NULL;

    if (!strcmp (rule, "mp"))
    {
        if (prems->num_stuff != 2)
            return _("Modus Ponens requires two (2) references.");

        unsigned char * prem_0, * prem_1;
        prem_0 = vec_str_nth (prems, 0);
        prem_1 = vec_str_nth (prems, 1);

        ret = proc_mp (prem_0, prem_1, conc);
        if (!ret)
            return NULL;
    }  /* End of modus ponens */

    if (!strcmp (rule, "ad"))
    {
        if (prems->num_stuff != 1)
            return _("Addition requires one (1) references.");

        unsigned char * prem;
        prem = vec_str_nth (prems, 0);

        ret = proc_ad (prem, conc);
        if (!ret)
            return NULL;
    }  /* End of addition. */

    if (!strcmp (rule, "sm"))
    {
        if (prems->num_stuff != 1)
            return _("Simplification requires one (1) reference.");

        unsigned char * prem;
        prem = vec_str_nth (prems, 0);

        ret = proc_sm (prem, conc);
        if (!ret)
            return NULL;

    }  /* End of simplification. */

    if (!strcmp (rule, "cn"))
    {
        if (prems->num_stuff < 2)
            return _("Conjunction requires at least two (2) references.");

        ret = proc_cn (prems, conc);
        if (!ret)
            return NULL;

    }  /* End of conjunction. */

    if (!strcmp (rule, "hs"))
    {
        if (prems->num_stuff < 2)
            return _("Hypothetical Syllogism requires at least two (2) references.");

        ret = proc_hs (prems, conc);
        if (!ret)
            return NULL;
    }  /* End of hypothetical syllogism. */

    if (!strcmp (rule, "ds"))
    {
        if (prems->num_stuff < 2)
            return _("Disjunctive Syllogism requires at least two (2) arguemnts.");

        ret = proc_ds (prems, conc);
        if (!ret)
            return NULL;
    }  /* End of disjunctive syllogism. */

    if (!strcmp (rule, "ex"))
    {
        if (prems->num_stuff != 0)
            return _("Excluded Middle requires zero (0) references.");

        ret = proc_ex (conc);
        if (!ret)
            return NULL;
    }  /* End of excluded middle. */

    if (!strcmp (rule, "cd"))
    {
        if (prems->num_stuff < 3)
            return _("Constructive Dilemma requires at least three (3) references.");

        ret = proc_cd (prems, conc);
        if (!ret)
            return NULL;
    }  /* End of constructive dilemma. */

    if (!strcmp (rule, "xi"))
    {
        if (prems->num_stuff != 2)
            return _("XOR Introduction requires exactly two (2) references.");

        unsigned char * prem_0, * prem_1;
        prem_0 = vec_str_nth (prems, 0);
        prem_1 = vec_str_nth (prems, 1);

        ret = proc_xi (prem_0, prem_1, conc);
        if (!ret)
            return NULL;
    }  /* End of XOR introduction. */

    if (!strcmp (rule, "xe"))
    {
        if (prems->num_stuff != 2)
            return _("XOR Elimination requires exactly two (2) references.");

        ret = proc_xe (prems, conc);
        if (!ret)
            return NULL;
    }  /* End of XOR elimination. */

    if (!ret)
        return NOT_MINE;
    return ret;
}

char *
proc_mp (unsigned char * prem_0, unsigned char * prem_1, unsigned char * conc)
{
    unsigned char * con_sen, * oth_sen;
    sen_put_len (prem_0, prem_1, &oth_sen, &con_sen);

    int ftc;
    unsigned char * lsen, * rsen;

    ftc = sexpr_find_top_connective (con_sen, S_CON, &lsen, &rsen);
    if (ftc == AEC_MEM)
        return NULL;

    if (ftc < 0)
    {
        if (lsen) free (lsen);
        if (rsen) free (rsen);

        return _("The top connective must be a conditional.");
    }

    int ant_eq, con_eq;

    ant_eq = !strcmp (lsen, oth_sen);
    con_eq = !strcmp (rsen, conc);

    free (lsen);
    free (rsen);

    if (!ant_eq)
        return _("The antecedent of the conditional reference must be the other reference.");

    if (!con_eq)
        return _("The consequence of the conditional reference must be the conclusion.");

    return CORRECT;
}

char *
proc_ad (unsigned char * prem, unsigned char * conc)
{
    int gg;
    vec_t * gg_vec;

    gg_vec = init_vec (sizeof (char *));
    if (!gg_vec)
        return NULL;

    gg = sexpr_get_generalities (conc, S_OR, gg_vec);
    if (gg == AEC_MEM)
        return NULL;

    if (gg == 1)
    {
        destroy_str_vec (gg_vec);
        return _("There must be a disjunction in the conclusion.");
    }

    int str_chk;
    unsigned char * gen_0;

    gen_0 = vec_str_nth (gg_vec, 0);
    str_chk = !strcmp (gen_0, prem);

    destroy_str_vec (gg_vec);
    if (!str_chk)
        return _("The reference must be the first disjunct in the conclusion.");

    return CORRECT;
}

char *
proc_sm (unsigned char * prem, unsigned char * conc)
{
    int gg;
    vec_t * gg_vec;

    gg_vec = init_vec (sizeof (char *));
    if (!gg_vec)
        return  NULL;

    gg = sexpr_get_generalities (prem, S_AND, gg_vec);
    if (gg == AEC_MEM)
        return NULL;

    if (gg == 1)
    {
        destroy_str_vec (gg_vec);
        return _("There must be a conjunction in the reference.");
    }

    int i;

    for (i = 0; i < gg_vec->num_stuff; i++)
    {
        unsigned char * cur_gen;

        cur_gen = vec_str_nth (gg_vec, i);

        if (!strcmp (cur_gen, conc))
        {
            destroy_str_vec (gg_vec);
            return CORRECT;
        }
    }

    destroy_str_vec (gg_vec);
    return _("One of the conjuncts in the reference must match the conclusion.");
}

char *
proc_cn (vec_t * prems, unsigned char * conc)
{
    int gg;
    vec_t * gg_vec;

    gg_vec = init_vec (sizeof (char *));
    if (!gg_vec)
        return NULL;

    gg = sexpr_get_generalities (conc, S_AND, gg_vec);
    if (gg == AEC_MEM)
        return NULL;

    if (gg == 1)
    {
        destroy_str_vec (gg_vec);
        return _("There must be a conjunction in the conclusion.");
    }

    int cmp_chk;
    cmp_chk = vec_str_cmp (prems, gg_vec);
    if (cmp_chk == AEC_MEM)
        return NULL;

    destroy_str_vec (gg_vec);

    switch (cmp_chk)
    {
    case 0:
        return CORRECT;
    case -2:
        return _("One of the references does not match up with a conjunct in the conclusion.");
    case -3:
        return _("One of the conjuncts in the conclusion does not match up with a reference.");
    }
    return _("Error");
}

char *
proc_hs (vec_t * prems, unsigned char * conc)
{
    int c_ftc;
    unsigned char * c_lsen, * c_rsen;

    c_ftc = sexpr_find_top_connective (conc, S_CON, &c_lsen, &c_rsen);
    if (c_ftc < 0)
    {
        if (c_lsen) free (c_lsen);
        if (c_rsen) free (c_rsen);

        return _("There must be a conditional in the conclusion.");
    }

    // Gather the antecedents and the consequences from the references.

    vec_t * ants, * cons;
    int i;

    ants = init_vec (sizeof (char *));
    if (!ants)
        return NULL;

    cons = init_vec (sizeof (char *));
    if (!cons)
        return NULL;

    for (i = 0; i < prems->num_stuff; i++)
    {
        int ftc_chk, ret_chk;
        unsigned char * lsen, * rsen;
        unsigned char * cur_prem;

        cur_prem = vec_str_nth (prems, i);

        ftc_chk = sexpr_find_top_connective (cur_prem, S_CON, &lsen, &rsen);
        if (ftc_chk == AEC_MEM)
            return NULL;

        if (ftc_chk < 0)
        {
            if (lsen) free (lsen);
            if (rsen) free (rsen);

            destroy_str_vec (ants);
            destroy_str_vec (cons);

            return _("All of the references must contain a conditional.");
        }

        ret_chk = vec_str_add_obj (ants, lsen);
        if (ret_chk < 0)
            return NULL;

        ret_chk = vec_str_add_obj (cons, rsen);
        if (ret_chk < 0)
            return NULL;
    }

    // Iterate through each antecedent, matching it with a consequence.

    unsigned char * cur_con;
    short * check;

    cur_con = c_lsen;
    check = (short *) calloc (prems->num_stuff, sizeof (short));
    CHECK_ALLOC (check, NULL);

    while (strcmp (cur_con, c_rsen))
    {
        for (i = 0; i < ants->num_stuff; i++)
        {
            if (check[i])
                continue;

            unsigned char * cur_ant;
            cur_ant = vec_str_nth (ants, i);

            if (!strcmp (cur_con, cur_ant))
            {
                check[i] = 1;
                cur_con = vec_str_nth (cons, i);
                break;
            }
        }

        if (i == ants->num_stuff)
        {
            free (check);

            destroy_str_vec (ants);
            destroy_str_vec (cons);

            free (c_lsen);
            free (c_rsen);

            return _("One of the consequences of a reference does not match an antecedent.");
        }
    }

    free (c_lsen);
    free (c_rsen);

    destroy_str_vec (ants);
    destroy_str_vec (cons);

    // Confirm that each reference was used.

    for (i = 0; i < prems->num_stuff; i++)
    {
        if (!check[i])
        {
            free (check);
            return _("All of the references must be used.");
        }
    }

    return CORRECT;
}

char *
proc_ds (vec_t * prems, unsigned char * conc)
{
    // Determine the longest reference.

    int gg, i, l_ref;
    vec_t * gg_vec;
    unsigned char * dis_ref;
    int long_len = -1;

    for (i = 0; i < prems->num_stuff; i++)
    {
        int cur_len;
        unsigned char * cur_ref;

        cur_ref = vec_str_nth (prems, i);
        cur_len = strlen (cur_ref);

        if (cur_len > long_len)
        {
            long_len = cur_len;
            dis_ref = cur_ref;
            l_ref = i;
        }
    }

    gg_vec = init_vec (sizeof (char *));
    if (!gg_vec)
        return NULL;

    gg = sexpr_get_generalities (dis_ref, S_OR, gg_vec);
    if (gg == AEC_MEM)
        return NULL;

    if (gg == 1)
    {
        destroy_str_vec (gg_vec);
        return _("There must be a disjunction in the longest reference.");
    }

    vec_t * not_refs;
    int ret_chk;

    not_refs = init_vec (sizeof (char *));
    if (!not_refs)
        return  NULL;

    for (i = 0; i < prems->num_stuff; i++)
    {
        if (i == l_ref)
            continue;

        unsigned char ** cur_ref, * not_cur_ref;

        cur_ref = vec_nth (prems, i);

        if (sexpr_not_check (*cur_ref))
        {
            not_cur_ref = sexpr_elim_not (*cur_ref);
            if (!not_cur_ref)
                return NULL;
        }
        else
        {
            not_cur_ref = sexpr_add_not (*cur_ref);
            if (!not_cur_ref)
                return NULL;
        }

        ret_chk = vec_str_add_obj (not_refs, not_cur_ref);
        if (ret_chk < 0)
            return NULL;
    }

    ret_chk = vec_str_add_obj (not_refs, conc);
    if (ret_chk < 0)
        return NULL;

    ret_chk = vec_str_cmp (gg_vec, not_refs);
    if (ret_chk == AEC_MEM)
        return NULL;

    destroy_str_vec (gg_vec);
    destroy_str_vec (not_refs);

    switch (ret_chk)
    {
    case 0:
        return CORRECT;
    case -2:
        return _("One of the disjuncts does not correspond to a reference or the conclusion.");
    case -3:
        return _("One of the references or conclusion does not match up with a disjunct.");
    }
    return _("Error with Disjunctive Syllogism.");
}

char *
proc_ex (unsigned char * conc)
{
    int ftc;
    unsigned char * lsen, * rsen;

    ftc = sexpr_find_top_connective (conc, S_OR, &lsen, &rsen);
    if (ftc == AEC_MEM)
        return NULL;

    if (ftc < 0)
    {
        if (lsen) free (lsen);
        if (rsen) free (rsen);

        return _("There must be a disjunction in the conclusion.");
    }

    unsigned char * not_lsen;
    int str_chk;

    not_lsen = sexpr_add_not (lsen);
    if (!not_lsen)
        return NULL;

    str_chk = !strcmp (rsen, not_lsen);

    free (not_lsen);
    free (rsen);
    free (lsen);

    if (!str_chk)
        return _("The left disjunct must be the negation of the right disjunct.");

    return CORRECT;
}

char *
proc_cd (vec_t * prems, unsigned char * conc)
{
    unsigned char * dis_ref;
    int i, d_ref;

    for (i = 0; i < prems->num_stuff; i++)
    {
        unsigned char * cur_ref;
        cur_ref = vec_str_nth (prems, i);

        if (!strncmp (cur_ref + 1, S_OR, S_CL))
        {
            dis_ref = cur_ref;
            d_ref = i;
            break;
        }
    }

    int ref_gg, conc_gg;
    vec_t * ref_gg_vec, * conc_gg_vec;

    conc_gg_vec = init_vec (sizeof (char *));
    if (!conc_gg_vec)
        return NULL;

    conc_gg = sexpr_get_generalities (conc, S_OR, conc_gg_vec);
    if (conc_gg == AEC_MEM)
        return NULL;

    if (conc_gg == 1)
    {
        destroy_str_vec (conc_gg_vec);
        return _("There must be a disjunction in the conclusion.");
    }

    vec_t * ants, * cons;

    ants = init_vec (sizeof (char *));
    if (!ants)
        return NULL;

    cons = init_vec (sizeof (char *));
    if (!cons)
        return NULL;

    for (i = 0; i < prems->num_stuff; i++)
    {
        if (i == d_ref)
            continue;

        int ftc_chk, ret_chk;
        unsigned char * lsen, * rsen;
        unsigned char * cur_ref;

        cur_ref = vec_str_nth (prems, i);

        ftc_chk = sexpr_find_top_connective (cur_ref, S_CON, &lsen, &rsen);
        if (ftc_chk == AEC_MEM)
            return NULL;

        if (ftc_chk < 0)
        {
            if (lsen) free (lsen);
            if (rsen) free (rsen);

            destroy_str_vec (ants);
            destroy_str_vec (cons);
            destroy_str_vec (conc_gg_vec);

            return _("All of the references except the disjunction reference must contain a conditional.");
        }

        ret_chk = vec_str_add_obj (ants, lsen);
        if (ret_chk < 0)
            return NULL;

        ret_chk = vec_str_add_obj (cons, rsen);
        if (ret_chk < 0)
            return NULL;
    }

    ref_gg_vec = init_vec (sizeof (char *));
    if (!ref_gg_vec)
        return NULL;

    ref_gg = sexpr_get_generalities (dis_ref, S_OR, ref_gg_vec);
    if (ref_gg == AEC_MEM)
        return NULL;

    if (ref_gg == 1)
    {
        destroy_str_vec (ref_gg_vec);
        destroy_str_vec (conc_gg_vec);
        destroy_str_vec (ants);
        destroy_str_vec (cons);
        return _("Constructive Dilemma constructed incorrectly.");
    }

    int ants_ret_chk, cons_ret_chk;

    ants_ret_chk = vec_str_cmp (ref_gg_vec, ants);
    if (ants_ret_chk == AEC_MEM)
        return NULL;

    cons_ret_chk = vec_str_cmp (conc_gg_vec, cons);
    if (cons_ret_chk == AEC_MEM)
        return NULL;

    destroy_str_vec (conc_gg_vec);
    destroy_str_vec (ref_gg_vec);
    destroy_str_vec (ants);
    destroy_str_vec (cons);

    if (ants_ret_chk == 0 && cons_ret_chk == 0)
        return CORRECT;

    switch (ants_ret_chk)
    {
    case 0:
        break;
    case -2:
        return _("One of the reference's disjuncts did not match an antecedent.");
    case -3:
        return _("One of the antecedents did not match a disjunct from the reference.");
    }

    switch (cons_ret_chk)
    {
    case -2:
        return _("One of the conclusion's disjuncts did not match a consequence.");
    case -3:
        return _("One of the consequences did not match a disjunct from the conclusion.");
    }
    return _("Error with Constructive Dilemma.");
}

/* XOR Introduction (xi).
 *  Validates: conc = (P xor Q), prem_0 = P, prem_1 = Q
 *  where one of P, Q must be the negation of the other.
 *  input:
 *    prem_0 - first premise.
 *    prem_1 - second premise.
 *    conc   - conclusion.
 *  output:
 *    CORRECT or an error string.
 */
char *
proc_xi (unsigned char * prem_0, unsigned char * prem_1, unsigned char * conc)
{
    /* The conclusion must have XOR as its top connective. */
    int ftc;
    unsigned char * lsen, * rsen;

    ftc = sexpr_find_top_connective (conc, S_XOR, &lsen, &rsen);
    if (ftc == AEC_MEM)
        return NULL;

    if (ftc < 0)
    {
        if (lsen) free (lsen);
        if (rsen) free (rsen);
        return _("The conclusion must contain an XOR.");
    }
    int match_direct  = (!strcmp (lsen, prem_0) && !strcmp (rsen, prem_1));
    int match_swapped = (!strcmp (lsen, prem_1) && !strcmp (rsen, prem_0));

    free (lsen); free (rsen);

    if (!match_direct && !match_swapped)
        return _("The two references must match the two disjuncts of the XOR conclusion.");

    return CORRECT;
}

/* XOR Elimination (xe).
 *  From (P xor Q) and ~P (or ~Q), derive Q (or P).
 *  From (P xor Q) and P (or Q), derive ~Q (or ~P).
 *  input:
 *    prems - vector of premises (one must be the XOR, the other is the minor premise).
 *    conc  - the conclusion.
 *  output:
 *    CORRECT or an error string.
 */
char *
proc_xe (vec_t * prems, unsigned char * conc)
{
    if (prems->num_stuff != 2)
        return _("XOR Elimination requires exactly two (2) references.");

    unsigned char * prem_0 = vec_str_nth (prems, 0);
    unsigned char * prem_1 = vec_str_nth (prems, 1);

    unsigned char * xor_ref = NULL, * minor_ref = NULL;
    int ftc;
    unsigned char * lsen = NULL, * rsen = NULL;

    /* Try prem_0 as XOR */
    ftc = sexpr_find_top_connective (prem_0, S_XOR, &lsen, &rsen);
    if (ftc == AEC_MEM) return NULL;

    if (ftc >= 0)
    {
        xor_ref = prem_0;
        minor_ref = prem_1;
    }
    else
    {
        if (lsen) free (lsen);
        if (rsen) free (rsen);
        lsen = rsen = NULL;

        /* Try prem_1 as XOR */
        ftc = sexpr_find_top_connective (prem_1, S_XOR, &lsen, &rsen);
        if (ftc == AEC_MEM) return NULL;
        if (ftc < 0)
        {
            if (lsen) free (lsen);
            if (rsen) free (rsen);
            return _("One of the references must contain an XOR as its top connective.");
        }
        xor_ref = prem_1;
        minor_ref = prem_0;
    }

    unsigned char * not_minor_ref;
    if (sexpr_not_check (minor_ref))
        not_minor_ref = sexpr_elim_not (minor_ref);
    else
        not_minor_ref = sexpr_add_not (minor_ref);

    if (!not_minor_ref) { free (lsen); free (rsen); return NULL; }

    unsigned char * not_conc;
    if (sexpr_not_check (conc))
        not_conc = sexpr_elim_not (conc);
    else
        not_conc = sexpr_add_not (conc);

    if (!not_conc) { free (lsen); free (rsen); free (not_minor_ref); return NULL; }

    int valid = 0;

    /* Mode 1: minor_ref is ~disjunct, conc is disjunct */
    if ((!strcmp(not_minor_ref, lsen) && !strcmp(conc, rsen)) ||
        (!strcmp(not_minor_ref, rsen) && !strcmp(conc, lsen)))
    {
        valid = 1;
    }

    /* Mode 2: minor_ref is disjunct, conc is ~disjunct */
    if ((!strcmp(minor_ref, lsen) && !strcmp(not_conc, rsen)) ||
        (!strcmp(minor_ref, rsen) && !strcmp(not_conc, lsen)))
    {
        valid = 1;
    }

    free (lsen); free (rsen); free (not_minor_ref); free (not_conc);

    if (valid)
        return CORRECT;

    return _("The minor premise and conclusion do not match the XOR disjuncts correctly.");
}


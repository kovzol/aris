#ifndef RULES_H
#define RULES_H

#ifdef __cplusplus
extern "C" {
#endif
#ifndef WIN32
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#else
#define _(String) String
#define N_(String) String
#endif

// The rules indices.

enum RULES_INDEX {
    RULE_MP = 0,
    RULE_AD,
    RULE_SM,
    RULE_CN,
    RULE_HS,
    RULE_DS,
    RULE_EX,
    RULE_CD,
    RULE_IM,
    RULE_DM,
    RULE_AS,
    RULE_CO,
    RULE_ID,
    RULE_DT,
    RULE_EQ,
    RULE_DN,
    RULE_EP,
    RULE_SB,
    RULE_UG,
    RULE_UI,
    RULE_EG,
    RULE_EI,
    RULE_BV,
    RULE_NQ,
    RULE_PR,
    RULE_II,
    RULE_FV,
    RULE_LM,
    RULE_SP,
    RULE_SQ,
    RULE_IN,
    RULE_BI,
    RULE_BN,
    RULE_BD,
    RULE_SN,
    NUM_RULES
};

#define END_INFER_RULES RULE_IM
#define END_EQUIV_RULES RULE_UG
#define END_PRED_RULES RULE_LM
#define END_MISC_RULES RULE_BI
#define END_BOOL_RULES NUM_RULES

// The rules list.

static const char rules_list[NUM_RULES][3] = {
    "mp", "ad", "sm", "cn", "hs", "ds", "ex", "cd",
    "im", "dm", "as", "co", "id", "dt", "eq", "dn", "ep", "sb",
    "ug", "ui", "eg", "ei", "bv", "nq", "pr", "ii", "fv",
    "lm", "sp", "sq", "in",
    "bi", "bn", "bd", "sn"
};

static char * rules_names[NUM_RULES] = {
    N_("Modus Ponens"), N_("Addition"), N_("Simplification"), N_("Conjunction"),
    N_("Hypothetical Syllogism"), N_("Disjunctive Syllogism"), N_("Excluded Middle"),
    N_("Constructive Dilemma"),
    N_("Implication"), N_("DeMorgan"), N_("Association"), N_("Commutativity"),
    N_("Idempotence"), N_("Distribution"), N_("Equivalence"), N_("Double Negation"),
    N_("Exportation"), N_("Subsumption"),
    N_("Universal Generalization"), N_("Universal Instantiation"),
    N_("Existential Generalization"), N_("Existential Instantiation"),
    N_("Bound Variable Substitution"), N_("Null Quantifier"), N_("Prenex"), N_("Identity"),
    N_("Free Variable Substitution"),
    N_("Lemma"), N_("Subproof"), N_("Sequence"), N_("Induction"),
    N_("Boolean Identity"), N_("Boolean Negation"), N_("Boolean Dominance"),
    N_("Symbol Negation")
};
#ifdef __cplusplus
}
#endif

#endif // RULES_H

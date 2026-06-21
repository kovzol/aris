/*
  Single source for all rule abbreviations shown at zoom > 150%.
 
  Design rationale

  Logic abbreviations (MP, HS, UG …) are ISO/Latin-derived academic notation
  that is identical across every natural language.  They are locale-invariant
  data, not translatable text, so they live here rather than in any .ts file.
 
  Lookup is index-based (categoryIndex, ruleIndex) to mirror the fixed layout
  of the combo2 property in ProofArea.qml.  This means:
   - No string matching needed — no dependency on the active language.
   - Adding a new UI language requires zero changes to this file.
   - If a future language genuinely needs different abbreviations, add an
     entry to the `overrides` map below.
 
  combo2 group order (must stay in sync with ProofArea.qml):
   0 → Inference
   1 → Equivalence
   2 → Predicate
   3 → Miscellaneous
  4 → Boolean
 */

// Universal abbreviations 

const _groups = [
    // [0] Inference
    //  Modus Ponens, Addition, Simplification, Conjunction,
    //  Hypothetical Syllogism, Disjunctive Syllogism, Excluded middle,
    //  Constructive Dilemma, XOR Introduction, XOR Elimination
    ["MP", "Add", "Simp", "Conj", "HS", "DS", "EM", "CD", "XORI", "XORE"],

    // [1] Equivalence
    //  Implication, DeMorgan, Association, Commutativity, Idempotence,
    //  Distribution, Equivalence, Double Negation, Exportation,
    //  Subsumption, Contrapositive
    ["Impl", "DeM", "Assoc", "Comm", "Idem", "Dist", "Equiv", "DN", "Exp", "Sub", "CP"],

    // [2] Predicate
    //  Universal Generalization, Universal Instantiation,
    //  Existential Generalization, Existential Instantiation,
    //  Bound Variable Substitution, Null Quantifier, Prenex,
    //  Identity, Free Variable Substitution
    ["UG", "UI", "EG", "EI", "BVS", "NQ", "Prenex", "Iden", "FVS"],

    // [3] Miscellaneous
    //  Lemma, Subproof, Sequence, Induction
    ["Lem", "SP", "Seq", "Ind"],

    // [4] Boolean
    //  Identity, Negation, Dominance, Symbol Negation
    ["Iden", "Neg", "Dom", "SymNeg"]
]

// Language-specific overrides
//
// Only populate when a language uses genuinely different abbreviations.
// Key: BCP-47 language code (e.g. "de", "ar", "zh").
// Value: sparse object { groupIndex: { ruleIndex: "ABBR" } }
//
// Example (hypothetical):
//   "zh": { 0: { 0: "假言" } }   // Chinese override for Modus Ponens group 0, rule 0
//
const _overrides = {
    // No overrides needed yet — all languages use universal Latin abbreviations.
}

// Public API 

/**
 * Returns the abbreviation for a rule identified by its position in combo2.
 *
 * @param {int}    categoryIndex  Index of the active category combobox (0–4).
 * @param {int}    ruleIndex      Index of the selected rule within that category.
 * @param {string} [lang]         Optional BCP-47 language code for override lookup.
 * @returns {string|null}  The abbreviation string, or null if indices are out of range.
 */
function get(categoryIndex, ruleIndex, lang) {
    // Check language-specific override first
    if (lang && _overrides[lang]) {
        const ovGroup = _overrides[lang][categoryIndex]
        if (ovGroup?.[ruleIndex] !== undefined)
            return ovGroup[ruleIndex]
    }

    // Fall back to universal table
    const group = _groups[categoryIndex]
    if (!group || ruleIndex < 0 || ruleIndex >= group.length)
        return null

    return group[ruleIndex]
}

/**
 * Convenience: add or update an override entry at runtime.
 * Useful if language detection is done dynamically.
 *
 * @param {string} lang          BCP-47 language code.
 * @param {int}    categoryIndex Group index (0–4).
 * @param {int}    ruleIndex     Rule index within the group.
 * @param {string} abbrev        The abbreviation to use.
 */
function setOverride(lang, categoryIndex, ruleIndex, abbrev) {
    if (!_overrides[lang])
        _overrides[lang] = {}
    if (!_overrides[lang][categoryIndex])
        _overrides[lang][categoryIndex] = {}
    _overrides[lang][categoryIndex][ruleIndex] = abbrev
}

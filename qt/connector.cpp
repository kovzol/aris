/* Connector Class to connect QML with underlying C logic.

   Copyright (C) 2023 Saksham Attri.

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
#include "connector.h"
#include "proofmodel.h"
#include "../src/proof.h"
#include "../src/process.h"
#include "../src/typedef.h"
#include "../src/sen-data.h"
#include "../src/list.h"
#include "../src/vec.h"
#include "../src/aio.h"

#include <QDebug>
#include <iostream>
#include <regex>
#include <algorithm>
#include <QFileDialog>
#include <QSaveFile>
#include <QUrl>
#include <QClipboard>
#include <QGuiApplication>
#include <QRegularExpression>

namespace {

// Frees all proof lines and goals, then the proof shell allocated by proof_init/aio_open.
void releaseCProof(proof_t *&proof)
{
    if (!proof)
        return;

    item_t *itm;
    item_t *next;

    // Free every sen_data value and its item_t wrapper.
    for (itm = proof->everything->head; itm; itm = next) {
        next = itm->next;
        if (itm->value)
            sen_data_destroy((sen_data *) itm->value);
        free(itm);
    }
    
    proof->everything->head = proof->everything->tail = nullptr;
    free(proof->everything);

    for (itm = proof->goals->head; itm; itm = next) {
        next = itm->next;
        if (itm->value)
            free(itm->value);
        free(itm);
    }
    proof->goals->head = proof->goals->tail = nullptr;
    free(proof->goals);

    free(proof);
    proof = nullptr;
}

} 

Connector::Connector(QObject *parent)
    : QObject{parent}, m_evalText{"Evaluate Proof"}, cProof(nullptr), returns(nullptr)
{   
    // Initialize rulesMap

    rulesMap["premise"] = -1;                       rulesMap["Modus Ponens"] = 0;                   rulesMap["Addition"] = 1;
    rulesMap["Simplification"] = 2;                 rulesMap["Conjunction"] = 3;                    rulesMap["Hypothetical Syllogism"] = 4;
    rulesMap["Disjunctive Syllogism"] = 5;          rulesMap["Excluded middle"] = 6;                rulesMap["Constructive Dilemma"] = 7;
    rulesMap["Implication"] = 8;                    rulesMap["DeMorgan"] = 9;                       rulesMap["Association"] = 10;
    rulesMap["Commutativity"] = 11;                 rulesMap["Idempotence"] = 12;                   rulesMap["Distribution"] = 13;
    rulesMap["Equivalence"] = 14;                   rulesMap["Double Negation"] = 15;               rulesMap["Exportation"] = 16;
    rulesMap["Subsumption"] = 17;                   rulesMap["Universal Generalization"] = 18;      rulesMap["Universal Instantiation"] = 19;
    rulesMap["Existential Generalization"] = 20;    rulesMap["Existential Instantiation"] = 21;     rulesMap["Bound Variable Substitution"] = 22;
    rulesMap["Null Quantifier"] = 23;               rulesMap["Prenex"] = 24;                        rulesMap["Identity"] = 25;
    rulesMap["Free Variable Substitution"] = 26;    rulesMap["Lemma"] = 27;                         rulesMap["subproof"] = 28;
    rulesMap["Sequence"] = 29;                      rulesMap["Induction"] = 30;                     rulesMap["Boolean Identity"] = 31;
    rulesMap["Boolean Negation"] = 32;              rulesMap["Boolean Dominance"] = 33;             rulesMap["Symbol Negation"] = 34;
    rulesMap["Excluded Middle"] = 6;                rulesMap["Excluded middle"] = 6;
    rulesMap["sf"] = -2;
    
    reverseMapInit();
}

void Connector::reverseMapInit()
{
    // Initialize reverseRulesMap

    reverseRulesMap[-1] = "premise";                        reverseRulesMap[0] = "Modus Ponens";                    reverseRulesMap[1] = "Addition";
    reverseRulesMap[2] = "Simplification";                  reverseRulesMap[3] = "Conjunction";                     reverseRulesMap[4] = "Hypothetical Syllogism";
    reverseRulesMap[5] = "Disjunctive Syllogism";           reverseRulesMap[6] = "Excluded middle";                 reverseRulesMap[7] = "Constructive Dilemma";
    reverseRulesMap[8] = "Implication";                     reverseRulesMap[9] = "DeMorgan";                        reverseRulesMap[10] = "Association";
    reverseRulesMap[11] = "Commutativity";                  reverseRulesMap[12] = "Idempotence";                    reverseRulesMap[13] = "Distribution";
    reverseRulesMap[14] = "Equivalence";                    reverseRulesMap[15] = "Double Negation";                reverseRulesMap[16] = "Exportation";
    reverseRulesMap[17] = "Subsumption";                    reverseRulesMap[18] = "Universal Generalization";       reverseRulesMap[19] = "Universal Instantiation";
    reverseRulesMap[20] = "Existential Generalization";     reverseRulesMap[21] = "Existential Instantiation";      reverseRulesMap[22] = "Bound Variable Substitution";
    reverseRulesMap[23] = "Null Quantifier";                reverseRulesMap[24] = "Prenex";                         reverseRulesMap[25] = "Identity";
    reverseRulesMap[26] = "Free Variable Substitution";     reverseRulesMap[27] = "Lemma";                          reverseRulesMap[28] = "subproof";
    reverseRulesMap[29] = "Sequence";                       reverseRulesMap[30] = "Induction";                      reverseRulesMap[31] = "Identity ";
    reverseRulesMap[32] = "Negation";                       reverseRulesMap[33] = "Dominance";                      reverseRulesMap[34] = "Symbol Negation";
    reverseRulesMap[-2] = "sf";
}

// Getter for m_evalText
QString Connector::evalText() const
{
    return m_evalText;
}

// Setter for m_evalText
void Connector::setEvalText(const QString &newEvalText)
{
    if (m_evalText == newEvalText)
        return;
    m_evalText = newEvalText;
    emit evalTextChanged();
}

/* Populates m_indices, with array of indices for sen-data corresponding to a line.
 *  input:
 *    toBeEval  - pointer to the ProofData object.
 *  output:
 *    none.
 */
void Connector::genIndices(const ProofData *toBeEval)
{
    //  ProofData::lines() returns by value (a full copy).
    const QVector<ProofLine> snap = toBeEval->lines();

    m_indices.clear();
    QList<int> parent_subproof;
    for (int i = 0; i < snap.size(); i++) {
        m_indices.push_back({});

        if (snap.at(i).pType == QLatin1String("sf"))
            parent_subproof.push_back(i + 1);

        if (i > 0 && snap.at(i).pInd < snap.at(i - 1).pInd)
            if (!parent_subproof.isEmpty())
                parent_subproof.pop_back();

        for (const int subp : parent_subproof)
            m_indices[i].push_back(subp);
    }

    for (int i = 0; i < m_indices.size(); i++)
        m_indices[i].push_back(-1);
}

/* Sets connectives (using regex) and generates proof_t from corresponding ProofData Object
 * This is stored inside cProof
 *  input:
 *    toBeEval  - pointer to the ProofData object.
 *  output:
 *    none.
 */
void Connector::genProof(const ProofData *toBeEval)
{
    main_conns = gui_conns;
    int conn = 1;
    std::regex pat("[&|~$%@#!^:>]");
 
    releaseCProof(cProof);
    cProof = proof_init();
    if (!cProof)
        return;

    genIndices(toBeEval);

    //  ProofData::lines() returns by value (full copy).
    const QVector<ProofLine> snap = toBeEval->lines();

    for (int i = 0; i < snap.size(); i++) {
        const ProofLine &pl = snap.at(i);   // const-ref into the snapshot — zero copies

        int *ind = (int *) calloc(m_indices[i].size(), sizeof(int));
        short *temp_refs = (short *) calloc(pl.pRefs.size(), sizeof(short));


        constexpr int RULE_UNKNOWN = -99;
        int rule     = rulesMap.value(pl.pType, RULE_UNKNOWN);
        int depth    = pl.pInd / 20;
        int premise  = (rule == -1 || pl.pType.isEmpty()) ? 1 : 0;
        int subproof = (rule == -2) ? 1 : 0;

        if (rule == -2)
            rule = -1;

        if (rule == RULE_UNKNOWN)
            rule = -1;


        // Assign Indices
        for (int ii = 0; ii < m_indices[i].size(); ii++)
            ind[ii] = m_indices[i][ii];

        // TODO : Cross-check Unicodes
        std::string str = pl.pText.toStdString();

        if (conn && std::regex_search(str, pat)) {
            main_conns = cli_conns;
            conn = 0;
        }

        // Assign references
        if (pl.pRefs.size() == 1) {
            temp_refs[0] = REF_END;
        } else {
            for (int ii = 1; ii < pl.pRefs.size(); ii++)
                temp_refs[ii - 1] = pl.pRefs.at(ii);
            temp_refs[pl.pRefs.size() - 1] = REF_END;
        }

        sen_data *sd = sen_data_init(i + 1, rule, (unsigned char *) str.c_str(), temp_refs,
                                     premise, pl.fname, subproof, depth, NULL);
        if (!sd) {
            free(ind);
            qDebug() << "proof: could not initialize sen_data";
            continue;
        }

        if (sd->indices)
            free(sd->indices);
        sd->indices = ind;

        item_t *itm = ls_ins_obj(cProof->everything, sd, cProof->everything->tail);
        if (!itm)
            qDebug() << "proof: could not insert sen_data";
    }

    // TODO : Fix boolean
    cProof->boolean = 0;
}


/* Inserts goal text from corresponding GoalData Object into cProof.
 *  input:
 *    toBeEval  - pointer to the GoalData object.
 *  output:
 *    none.
 */
void Connector::genGoals(const GoalData *toBeEval)
{
    // Snapshot once — glines() returns by value (full copy).
    const QVector<GoalLine> snap = toBeEval->glines();

    for (int i = 0; i < snap.size(); i++) {
        std::string str = snap.at(i).gText.toStdString();
        const size_t len = str.size();   // O(1) — avoids two strlen() calls

        unsigned char *temp_text = (unsigned char *) calloc(len + 1, sizeof(unsigned char));
        memcpy(temp_text, str.c_str(), len);

        item_t *itm = ls_ins_obj(cProof->goals, temp_text, cProof->goals->tail);
        if (!itm)
            qDebug() << "proof: could not insert goal data";
    }
}

/* Evaluates the proof and updates the model with per-line error details.
 *  input:
 *    toBeEval  - pointer to the ProofData object.
 *    gls       - pointer to the GoalData object.
 *    pm        - pointer to the ProofModel (receives ErrorRole per line).
 *  output:
 *    1 on success.
 */
int Connector::evalProof(const ProofData *toBeEval, const GoalData *gls, ProofModel *pm)
{
    genProof(toBeEval);
    genGoals(gls);

    if (returns)
        destroy_str_vec(returns);
    returns = init_vec(sizeof(char *));

    if (!proof_eval(cProof, returns, 1))
        qDebug() << "Proof Evaluated Successfully";
    else
        qDebug() << "Memory Error";

    // Clear any stale errors from a previous evaluation.
    pm->clearErrors();

    item_t *ev_itr = cProof->everything->head;
    bool anyError = false;

    for (int i = 0; i < returns->num_stuff; i++) {
        char *cur_ret = (char *) vec_str_nth(returns, i);
        sen_data *sd  = (sen_data *) ev_itr->value;

        if (strcmp(cur_ret, CORRECT)) {
            anyError = true;

            // Build "Line N · RuleName: <message>" for inline display.
            int lineNum     = i + 1;
            QString ruleName = reverseRulesMap.value(sd->rule, QStringLiteral("?"));
            QString message  = QString::fromUtf8(cur_ret);
            QString fullMsg  = QStringLiteral("Line %1 · %2: %3")
                                   .arg(lineNum)
                                   .arg(ruleName)
                                   .arg(message);

            // Write error text directly into the model row (0-based).
            pm->setData(pm->index(i, 0), fullMsg, ProofModel::ErrorRole);

            qDebug() << "[eval] Error line" << lineNum << "(" << ruleName << ") -" << message;
        } else {
            qDebug() << "[eval] Line" << i + 1 << ": Correct!";
        }

        ev_itr = ev_itr->next;
    }

    // evalText stays as the global status,  detail is now in the model.
    setEvalText(anyError ? QStringLiteral("Errors found") : QStringLiteral("Correct!"));

    return 1;
}

/* Generates cProof and saves file (using aio_save).
 *  input:
 *    name          - filename for the file being saved.
 *    toBeSaved     - pointer to the ProofData object.
 *    gls           - pointer to the GoalData object.
 *  output:
 *    none.
 */
void Connector::saveProof(const QString &name, const ProofData *toBeSaved, const GoalData *gls)
{
    genProof(toBeSaved);
    genGoals(gls);

    // selectedFile from QML FileDialog is a file:// URL; convert to a local path.
    QString localName = QUrl(name).toLocalFile();
    if (localName.isEmpty())
        localName = name;

    std::string nameStr = localName.toStdString();
    char *file_name = (char *) calloc((nameStr.size()+1), sizeof(char));
    memcpy(file_name, nameStr.c_str(), nameStr.size());

    if (aio_save(cProof,(const char *) file_name) == 0)
        qDebug() << "File Saved Successfully";
    else
        qDebug() << "File Save Failed for path:" << localName;
    if (file_name)
        free(file_name);
}

/* Opens the specified proof after clearing the previous one.
 *  input:
 *    name      - filename of the opened proof.
 *    openTo    - pointer to the ProofData object.
 *    gls       - pointer to the GoalData object.
 *  output:
 *    none.
 */
void Connector::openProof(const QString &name, ProofData *openTo, GoalData *gls)
{

    // selectedFile from QML FileDialog is a file:// URL; convert to a local path.
    QString localName = QUrl(name).toLocalFile();
    if (localName.isEmpty())
        localName = name;

    std::string nameStr = localName.toStdString();
    char *file_name = (char *) calloc((nameStr.size()+1), sizeof(char)); 
    memcpy(file_name, nameStr.c_str(), nameStr.size());

    releaseCProof(cProof);
    cProof = aio_open((const char *) file_name);
    if (cProof)
        qDebug() << "File Opened Successfully";
    else
        qDebug() << "File Open Failed for path:" << localName;
    if (file_name)
        free(file_name);

    reverseMapInit();

    if (!cProof)
    {
        qDebug() << "Cannot populate UI: proof is null.";
        return;
    }

    // lines() returns by value — cache the count rather than copying the vector
    // on each iteration just to read its size.
    const int s = openTo->lines().size();
    for (int i = 0; i < s; i++)
        openTo->removeLineAt(0);
    //    openTo->lines().clear();

    item_t * pf_itr;
    int d = 0;
    for (pf_itr = cProof->everything->head; pf_itr; pf_itr = pf_itr->next){
        sen_data *sd = (sen_data *) pf_itr->value;
        QList<int> temp_refs = {-1};
        for (int i = 0; sd->refs[i] != REF_END; i++)
            temp_refs.push_back(sd->refs[i]);

        if (sd->depth > d)
            sd->rule = -2;
        openTo->insertLine(sd->line_num-1,sd->line_num,(const char *) sd->text,reverseRulesMap[sd->rule],(sd->depth > 0),
                           (sd->rule == -2),(sd->line_num != 1 && ((sen_data *) pf_itr->prev->value)->depth > sd->depth), sd->depth * 20,temp_refs);
        d = sd->depth;
    }

    int g = gls->glines().size();
    for (int i = 0; i < g; i++)
        gls->removegLineAt(0);

    int i = 0;
    for (pf_itr = cProof->goals->head; pf_itr; pf_itr = pf_itr->next){
        gls->insertgLine(i,-2,false,(const char *) pf_itr->value);
        i++;
    }

    qDebug() << "Model Loaded Successfully";
}

/* Opens the specified proof after clearing the previous one (for WebAssembly).
 *  input:
 *    open      - pointer to the ProofData object.
 *    gls       - pointer to the GoalData object.
 *  output:
 *    none.
 */
void Connector::wasmOpenProof(ProofData *open, GoalData *gls)
{
    auto fileContentReady = [open, this, gls](const QString &fileName, const QByteArray &fileContent) {
        if (fileName.isEmpty()) {
            qDebug() << "No file was selected" ;
        } else {
            QSaveFile file(fileName);
            file.open(QIODevice::WriteOnly);
            file.write(fileContent);
            file.commit();
            openProof(fileName,open,gls);
            file.deleteLater();
        }
    };
    QFileDialog::getOpenFileContent("Aris Proof (*.tle)",  fileContentReady);
}

/* Generates cProof and saves file (using aio_save) (for WebAssembly).
 *  input:
 *    pd         - pointer to the ProofData object.
 *    gls        - pointer to the GoalData object.
 *  output:
 *    none.
 */
void Connector::wasmSaveProof(const ProofData * pd, const GoalData *gls)
{
    saveProof("temp.tle",pd,gls);
    QFile file("temp.tle");
    file.open(QIODevice::ReadOnly);
    QFileDialog::saveFileContent(file.readAll(),"Untitled.tle");
    file.remove("temp.tle");
}

// Getter for cProof
proof_t *Connector::getCProof() const
{
    return cProof;
}

// Getter for post-evaluation return values
vec_t *Connector::getReturns() const
{
    return returns;
}

void Connector::smartPaste(ProofData *pd, ProofModel *pm)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString text = clipboard->text();
    if (text.isEmpty()) return;

    QStringList lines = text.split(QRegularExpression(QStringLiteral("[\\r\\n]")), Qt::SkipEmptyParts);

    // Static data: initialised exactly once across all calls 

    // Rule-name alias table: lowercase user text → canonical Aris rule string.
    static const QMap<QString, QString> ruleAliases = {
        // Inference rules
        {"modus ponens",              "Modus Ponens"},
        {"mp",                        "Modus Ponens"},
        {"premise",                   "premise"},
        {"addition",                  "Addition"},     {"add", "Addition"},
        {"simplification",            "Simplification"},{"simp", "Simplification"}, {"sm", "Simplification"},
        {"conjunction",               "Conjunction"},  {"conj", "Conjunction"},   {"cn", "Conjunction"},
        {"hypothetical syllogism",    "Hypothetical Syllogism"}, {"hs", "Hypothetical Syllogism"},
        {"disjunctive syllogism",     "Disjunctive Syllogism"},  {"ds", "Disjunctive Syllogism"},
        {"excluded middle",           "Excluded Middle"}, {"em", "Excluded Middle"}, {"ex", "Excluded Middle"},
        {"constructive dilemma",      "Constructive Dilemma"}, {"cd", "Constructive Dilemma"},
        // Equivalence rules
        {"implication",               "Implication"},  {"impl", "Implication"}, {"im", "Implication"},
        {"demorgan",                  "DeMorgan"},     {"dem",  "DeMorgan"},    {"dm", "DeMorgan"},
        {"de morgan",                 "DeMorgan"},
        {"association",               "Association"},  {"assoc", "Association"}, {"as", "Association"},
        {"commutativity",             "Commutativity"},{"comm", "Commutativity"},{"co", "Commutativity"},
        {"idempotence",               "Idempotence"},  {"idem", "Idempotence"}, {"id", "Idempotence"},
        {"distribution",              "Distribution"}, {"dist", "Distribution"},{"dt", "Distribution"},
        {"equivalence",               "Equivalence"},  {"equiv", "Equivalence"},{"eq", "Equivalence"},
        {"double negation",           "Double Negation"},{"dn", "Double Negation"},
        {"exportation",               "Exportation"},  {"exp", "Exportation"},  {"ep", "Exportation"},
        {"subsumption",               "Subsumption"},  {"sub", "Subsumption"},  {"sb", "Subsumption"},
        // Predicate / quantifier rules
        {"universal generalization",  "Universal Generalization"},  {"ug", "Universal Generalization"},
        {"universal instantiation",   "Universal Instantiation"},   {"ui", "Universal Instantiation"},
        {"existential generalization","Existential Generalization"},{"eg", "Existential Generalization"},
        {"existential instantiation", "Existential Instantiation"}, {"ei", "Existential Instantiation"},
        {"bound variable substitution","Bound Variable Substitution"},{"bv","Bound Variable Substitution"},
        {"null quantifier",           "Null Quantifier"},  {"nq", "Null Quantifier"},
        {"prenex",                    "Prenex"},            {"pr", "Prenex"},
        // Identity rules 
        {"identity",                  "Identity"},
        {"iden",                      "Identity"},
        // Misc rules
        {"free variable substitution","Free Variable Substitution"},{"fv","Free Variable Substitution"},
        {"lemma",                     "Lemma"},  {"lem", "Lemma"}, {"lm", "Lemma"},
        {"sequence",                  "Sequence"},{"sq",  "Sequence"},
        {"induction",                 "Induction"},{"in", "Induction"},
        //  Boolean algebra rules 
        {"boolean identity",          "Boolean Identity"},   {"bi", "Boolean Identity"},
        {"boolean negation",          "Boolean Negation"},   {"bn", "Boolean Negation"},
        {"boolean dominance",         "Boolean Dominance"},  {"bd", "Boolean Dominance"},
        {"symbol negation",           "Symbol Negation"},    {"sn", "Symbol Negation"},
        {"negation",                  "Symbol Negation"},
        {"dominance",                 "Boolean Dominance"},
    };

    //Pre-compile every per-alias regex ONCE into a parallel map.
    static const QMap<QString, QRegularExpression> aliasRe = []() {
        QMap<QString, QRegularExpression> m;
        for (auto it = ruleAliases.constBegin(); it != ruleAliases.constEnd(); ++it)
            m[it.key()] = QRegularExpression(
                QLatin1String("\\b") + QRegularExpression::escape(it.key())
                + QLatin1String("\\b(?:[^a-z]*)$"));
        return m;
    }();

    // Pre-compiled utility regexes
    static const QRegularExpression numRe(QStringLiteral("^\\s*(?:\\[|\\()?\\d+(?:\\]|\\)|\\.)?\\s*"));
    static const QRegularExpression digitRe(QStringLiteral("\\d+"));
    //trailingSepRe alongside the other utility statics (not inside an if-block).
    static const QRegularExpression trailingSepRe(QStringLiteral("[|,:\\s]+$"));
    // Quantifier normalisation — all pre-compiled statics, zero per-call overhead.
    static const QRegularExpression reForall(QStringLiteral("\\bForall\\s+([a-z])\\b"),  QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reExists(QStringLiteral("\\bExists\\s+([a-z])\\b"),  QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reAxxDbl(QStringLiteral("\\bA([xyz])\\s+\\1\\b"),    QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reExxDbl(QStringLiteral("\\bE([xyz])\\s+\\1\\b"),    QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reAxxCmp(QStringLiteral("\\bA([xyz])\\1\\b"),        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reExxCmp(QStringLiteral("\\bE([xyz])\\1\\b"),        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reAx(QStringLiteral("\\bA\\s*([xyz])\\b"),           QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reEx(QStringLiteral("\\bE\\s*([xyz])\\b"),           QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reQuantParen(QStringLiteral("([\u2200\u2203])([xyz])\\s+([A-Za-z][A-Za-z0-9_]*\\([^)]*\\))"));
    // Fallback: detect unrecognised capitalised rule names followed by ref integers at end of line.
    // Each word must start with an uppercase letter; words are separated by single spaces.
    static const QRegularExpression unknownRuleRe(
        QStringLiteral("\\b([A-Z][a-zA-Z]+(?:\\s[A-Z][a-zA-Z]+)*)\\s+(\\d+(?:[\\s,]+\\d+)*)\\s*$"));

    // ────────────────────────────────────────────────────────────────────────

    emit smartPasteStarted(); // Signal QML to set isExtFile=true before rows are inserted

    // Cache count upfront — pd->lines() returns a full QVector copy,
    // so calling it in a while-condition copies the vector every iteration.
    const int existingCount = pd->lines().size();
    for (int i = 0; i < existingCount; i++)
        pd->removeLineAt(0);

    int insertIdx = 0;

    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) continue;

        // 1. Comments
        int commentIndex = line.indexOf(QStringLiteral(";;"));
        if (commentIndex == -1) commentIndex = line.indexOf(QStringLiteral("//"));
        if (commentIndex == -1) commentIndex = line.indexOf(QStringLiteral("#"));
        if (commentIndex != -1) line = line.left(commentIndex).trimmed();

        // Skip lines that were entirely comments — they were non-empty before stripping
        if (line.isEmpty()) continue;

        // 2. Line numbers, only strip if line actually starts with a number
        QRegularExpressionMatch numMatch = numRe.match(line);
        if (numMatch.hasMatch() && numMatch.capturedLength() > 0 && line.at(0).isDigit()) {
            line.remove(0, numMatch.capturedLength());
            line = line.trimmed();
        }

        // 3. Identify rule and extract reference line numbers.
        QString foundRule;
        QList<int> refs;
        refs.push_back(-1); // Sentinel — pRefs[0] is always -1
        const QString lowerLine = line.toLower();

        int bestRuleIndex = -1;
        QString bestAlias;

        // iterate over pre-compiled aliasRe — no regex construction inside the loop.
        for (auto it = aliasRe.constBegin(); it != aliasRe.constEnd(); ++it) {
            QRegularExpressionMatch match = it.value().match(lowerLine);
            if (match.hasMatch() && match.capturedStart() > bestRuleIndex) {
                bestRuleIndex = match.capturedStart();
                bestAlias     = it.key();
                foundRule     = ruleAliases.value(it.key());
            }
        }

        if (bestRuleIndex != -1) {
            const QString afterRule = lowerLine.mid(bestRuleIndex + bestAlias.length());
            QRegularExpressionMatchIterator refIt = digitRe.globalMatch(afterRule);
            while (refIt.hasNext()) {
                int ref = refIt.next().captured(0).toInt();
                if (ref > 0)
                    refs.push_back(ref);
            }

            line = line.left(bestRuleIndex).trimmed();
            line.remove(trailingSepRe);
        } else {
            // No known rule found — check if it matches the unknownRuleRe pattern.
            // instead of silently treating the line as a premise.
            QRegularExpressionMatch m = unknownRuleRe.match(line);
            if (m.hasMatch()) {
                foundRule = m.captured(1).trimmed();
                QRegularExpressionMatchIterator refIt = digitRe.globalMatch(m.captured(2));
                while (refIt.hasNext()) {
                    int ref = refIt.next().captured(0).toInt();
                    if (ref > 0) refs.push_back(ref);
                }
                line = line.left(m.capturedStart()).trimmed();
                line.remove(trailingSepRe);
            }
        }

        const bool isKnownRule = ruleAliases.values().contains(foundRule);
        if (refs.size() == 1 && !foundRule.isEmpty() && isKnownRule
            && foundRule != QLatin1String("sf")
            && foundRule != QLatin1String("subproof"))
            foundRule = QStringLiteral("premise");

        if (foundRule.isEmpty())
            foundRule = QStringLiteral("premise");

        // 4. Normalise ASCII operators → Unicode
        line.replace(QStringLiteral("->"), QStringLiteral("→"));
        line.replace(QStringLiteral("=>"), QStringLiteral("→"));
        line.replace(QStringLiteral("/\\"), QStringLiteral("∧"));
        line.replace(QStringLiteral("&"),  QStringLiteral("∧"));
        line.replace(QStringLiteral("\\/"), QStringLiteral("∨"));
        line.replace(QStringLiteral("|"),  QStringLiteral("∨"));
        line.replace(QStringLiteral("~"),  QStringLiteral("¬"));
        line.replace(QStringLiteral("!"),  QStringLiteral("¬"));

        // Quantifier normalisation — all pre-compiled statics
        line.replace(reForall,    QStringLiteral("∀\\1"));
        line.replace(reExists,    QStringLiteral("∃\\1"));
        line.replace(reAxxDbl,   QStringLiteral("∀\\1"));
        line.replace(reExxDbl,   QStringLiteral("∃\\1"));
        line.replace(reAxxCmp,   QStringLiteral("∀\\1"));
        line.replace(reExxCmp,   QStringLiteral("∃\\1"));
        line.replace(reAx,       QStringLiteral("∀\\1"));
        line.replace(reEx,       QStringLiteral("∃\\1"));
        // Aris requires parentheses wrapping the predicate after a quantifier.
        line.replace(reQuantParen, QStringLiteral("\\1\\2(\\3)"));

        //  Per-line debug logging only in debug builds.
#ifndef QT_NO_DEBUG
        qDebug() << "[SmartPaste] Inserting at" << insertIdx
                 << "text:" << line << "rule:" << foundRule << "refs:" << refs;
#endif

        //  Pass line (QString) directly — insertLine already takes QString.
        // The old toStdString().c_str() was UB: pointer into a temporary std::string.
        pd->insertLine(insertIdx, insertIdx + 1, line, foundRule, false, false, false, 0, refs);
        insertIdx++;
    }

    // Batch-update line numbers after the full insertion loop
    if (insertIdx > 0)
        pm->updateLines();

    emit smartPasteDone();
}

void Connector::smartCopy(const ProofData *pd, const QVariantList &selectedIndices)
{
    if (!pd) return;

    QList<int> sortedIndices;
    for (const QVariant &v : selectedIndices) {
        sortedIndices.append(v.toInt());
    }

    QString copyText;
    const auto &lines = pd->lines();

    for (int idx : sortedIndices) {
        if (idx < 0 || idx >= lines.size()) continue;
        const ProofLine &pl = lines.at(idx);

        // Reconstruct indentation (pInd is in pixels, ~20 per indent, 4 spaces per indent)
        QString indentStr;
        int spaces = (pl.pInd / 20) * 4;
        indentStr.fill(' ', spaces);

        // Reconstruct references
        QString refsStr;
        if (pl.pRefs.size() > 1) { 
            for (int j = 1; j < pl.pRefs.size(); j++) {
                refsStr += QString::number(pl.pRefs[j]);
                if (j < pl.pRefs.size() - 1) refsStr += ", ";
            }
        }

        // Format: [indent][text] [rule] [refs]
        QString lineStr = indentStr + pl.pText;
        if (!pl.pType.isEmpty() && pl.pType != "undefined") {
            if (!lineStr.isEmpty() && !lineStr.endsWith(' ')) lineStr += " ";
            lineStr += pl.pType;
        }
        if (!refsStr.isEmpty()) {
            if (!lineStr.isEmpty() && !lineStr.endsWith(' ')) lineStr += " ";
            lineStr += refsStr;
        }

        copyText += lineStr + "\n";
    }

    if (QClipboard *clipboard = QGuiApplication::clipboard()) {
        clipboard->setText(copyText);
    }
}

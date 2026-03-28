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
#include <QFileDialog>
#include <QSaveFile>
#include <QUrl>
#include <QMessageBox>
#include <exception>

static void connector_free_proof (proof_t * proof)
{
    if (!proof)
        return;

    if (proof->everything)
    {
        item_t * itm = proof->everything->head;
        while (itm)
        {
            item_t * next = itm->next;
            if (itm->value)
                sen_data_destroy ((sen_data *) itm->value);
            free (itm);
            itm = next;
        }
        free (proof->everything);
    }

    if (proof->goals)
    {
        item_t * itm = proof->goals->head;
        while (itm)
        {
            item_t * next = itm->next;
            if (itm->value)
                free (itm->value);
            free (itm);
            itm = next;
        }
        free (proof->goals);
    }

    free (proof);
}

static int connector_show_eval_dialog (Connector * connector,
                                       QMessageBox::Icon icon,
                                       const QString & title,
                                       const QString & message,
                                       bool reset_eval_state)
{
    if (connector && reset_eval_state)
        connector->setEvalText ("Evaluate Proof");

    QMessageBox msg (icon, title, message, QMessageBox::Ok, nullptr);
    msg.exec ();
    return 0;
}

static bool connector_has_nonempty_goal (const GoalData * goals)
{
    if (!goals)
        return false;

    const QVector<GoalLine> goal_lines = goals->glines ();
    for (const GoalLine & line : goal_lines)
    {
        if (!line.gText.trimmed ().isEmpty ())
            return true;
    }

    return false;
}

Connector::Connector(QObject *parent)
    : QObject{parent}, cProof(nullptr), returns(nullptr), m_evalText{"Evaluate Proof"}
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
    rulesMap["Sequence"] = 29;                      rulesMap["Induction"] = 30;                     rulesMap["Identity "] = 31;
    rulesMap["Negation"] = 32;                      rulesMap["Dominance"] = 33;                     rulesMap["Symbol Negation"] = 34;
    rulesMap["sf"] = -2;
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
    m_indices.clear();
    QList<int> parent_subproof;
    for (int i = 0; i < toBeEval->lines().size(); i++){
        m_indices.push_back({});

        if (toBeEval->lines().at(i).pType == "sf")
            parent_subproof.push_back(i + 1);

        if ( i > 0 && toBeEval->lines().at(i).pInd < toBeEval->lines().at(i-1).pInd)
            if (parent_subproof.size() > 0)
                parent_subproof.pop_back();

        for (const int subp: parent_subproof)
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
    if (!toBeEval)
        return;

    if (cProof)
    {
        connector_free_proof(cProof);
        cProof = nullptr;
    }

    main_conns_init_gui();
    int conn = 1;
    std::regex pat("[&|~$%@#!^:>]");
    cProof = proof_init();
    if (!cProof)
        return;
    genIndices(toBeEval);
    for (int i = 0; i < toBeEval->lines().size(); i++){
        sen_data *sd;
        item_t *itm;
        unsigned char *temp_text;
        sd = (sen_data *) calloc (1, sizeof(sen_data));
        size_t ind_count = m_indices[i].size();
        size_t ind_alloc = ind_count > 0 ? ind_count : 1;
        int *ind = (int *) calloc(ind_alloc, sizeof(int));
        int ref_count = toBeEval->lines().at(i).pRefs.size();
        int actual_refs = 0;
        for (int ii = 0; ii < ref_count; ii++)
            if (toBeEval->lines().at(i).pRefs.at(ii) > 0)
                actual_refs++;

        short *temp_refs = (short *) calloc(actual_refs + 1, sizeof(short));

        if (!sd || !ind || !temp_refs)
        {
            if (sd)
                free(sd);
            if (ind)
                free(ind);
            if (temp_refs)
                free(temp_refs);
            continue;
        }

        sd->line_num = i+1;
        sd->rule = rulesMap[toBeEval->lines().at(i).pType];
        sd->depth = toBeEval->lines().at(i).pInd/20;
        sd->premise = (sd->rule == -1)?1:0;
        sd->subproof = (sd->rule == -2)?1:0;
        sd->file = toBeEval->lines().at(i).fname
                   ? (unsigned char *) strdup((const char *) toBeEval->lines().at(i).fname)
                   : NULL;

        if (sd->rule == -2)
            sd->rule = -1;

        // Assign Indices
        for (int ii = 0; ii < (int) ind_count; ii++)
            ind[ii] = m_indices[i][ii];

        sd->indices = ind;

        // TODO : Cross-check Unicodes
        // Assign Text
        std::string str = toBeEval->lines().at(i).pText.toStdString();

        if (conn && std::regex_search(str,pat)){
            main_conns_init_cli();
            conn = 0;
        }

        temp_text = (unsigned char *) calloc((strlen(str.c_str()))+1, sizeof(unsigned char));
        if (!temp_text)
        {
            free(sd);
            free(ind);
            free(temp_refs);
            continue;
        }
        memcpy(temp_text, str.c_str(), strlen(str.c_str()));
        sd->text = temp_text;


        // Assign references (accept only positive line numbers).
        int ref_pos = 0;
        for (int ii = 0; ii < ref_count; ii++)
        {
            int ref_val = toBeEval->lines().at(i).pRefs.at(ii);
            if (ref_val > 0)
                temp_refs[ref_pos++] = (short) ref_val;
        }
        temp_refs[ref_pos] = REF_END;
        sd->refs = temp_refs;

        // Insert into proof object
        itm = ls_ins_obj (cProof->everything, sd, cProof->everything->tail);
        if (!itm)
        {
            free(sd->text);
            free(sd->refs);
            free(sd->indices);
            free(sd);
            qDebug() << "proof: could not insert sen_data ";
        }
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
    if (!toBeEval || !cProof || !cProof->goals)
        return;

    for (int i = 0; i < toBeEval->glines().size(); i++){

        unsigned char *temp_text;
        item_t *itm;

        std::string str = toBeEval->glines().at(i).gText.toStdString();
        temp_text = (unsigned char *) calloc((strlen(str.c_str()))+1, sizeof(unsigned char));
        if (!temp_text)
            continue;
        memcpy(temp_text, str.c_str(), strlen(str.c_str()));

        itm = ls_ins_obj (cProof->goals, temp_text, cProof->goals->tail);
        if (!itm)
        {
            free(temp_text);
            qDebug() << "proof: could not insert goal data ";
        }

    }
}

/* Evaluates the proof and updates m_evalText accordingly.
 *  input:
 *    toBeEval  - pointer to the ProofData object.
 *    gls       - pointer to the GoalData object.
 *  output:
 *    1 on success.
 */
int Connector::evalProof(const ProofData *toBeEval, const GoalData *gls)
{
    try
    {
        if (!toBeEval || !gls)
            return connector_show_eval_dialog(this, QMessageBox::Critical,
                                              "Evaluation Error",
                                              "Invalid evaluation input.",
                                              true);

        if (!connector_has_nonempty_goal(gls))
            return connector_show_eval_dialog(this, QMessageBox::Warning,
                                              "Error",
                                              "Please set a goal before evaluating the proof.",
                                              true);

        if (returns)
        {
            destroy_str_vec(returns);
            returns = nullptr;
        }

        genProof(toBeEval);
        if (!cProof || !cProof->everything)
            return connector_show_eval_dialog(this, QMessageBox::Critical,
                                              "Evaluation Error",
                                              "Evaluation failed (proof generation error).",
                                              true);

        genGoals(gls);
        returns = init_vec(sizeof(char *));
        if (!returns)
            return connector_show_eval_dialog(this, QMessageBox::Critical,
                                              "Evaluation Error",
                                              "Evaluation failed (internal initialization error).",
                                              true);

        int eval_rc = proof_eval(cProof, returns, 0);
        if (!eval_rc)
            qDebug() << "Proof Evaluated Successfully";
        else
        {
            qDebug() << "Memory Error";
            return connector_show_eval_dialog(this, QMessageBox::Critical,
                                              "Evaluation Error",
                                              "Evaluation failed (internal memory error).",
                                              true);
        }

        item_t * ev_itr;
        int f = 0;
        ev_itr = cProof->everything->head;
        for (int i = 0; i < returns->num_stuff; i++){
            char * cur_ret;
            cur_ret = (char *) vec_str_nth (returns, i);
            if (!ev_itr || !ev_itr->value || !cur_ret)
                break;

            char * cur_line;
            cur_line =(char *) ((sen_data *) ev_itr->value)->text;
            if (!cur_line)
                cur_line = (char *) "";

            if (strcmp (cur_ret, CORRECT)){
                qDebug() << "Error in line " << i + 1 << "- " << cur_line;
                setEvalText(((f)?m_evalText:"") + QString("Error in line %1 - \n      ").arg(i+1) + cur_ret + "\n");
                f = 1;
                qDebug() << "  "<< cur_ret;
            }
            else
                qDebug() <<"Line " << i + 1 << ": " << CORRECT;

            ev_itr = ev_itr->next;
        }
        if (!f)
        {
            setEvalText("Correct!");
            return 1;
        }

        return connector_show_eval_dialog(this, QMessageBox::Critical,
                                          "Evaluation Error",
                                          m_evalText,
                                          false);
    }
    catch (const std::exception & ex)
    {
        return connector_show_eval_dialog(this, QMessageBox::Critical,
                                          "Evaluation Error",
                                          QString("An error occurred during evaluation:\n%1").arg(ex.what()),
                                          true);
    }
    catch (...)
    {
        return connector_show_eval_dialog(this, QMessageBox::Critical,
                                          "Evaluation Error",
                                          "An error occurred during evaluation.",
                                          true);
    }
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
    if (!openTo || !gls)
        return;

    if (cProof)
    {
        connector_free_proof(cProof);
        cProof = nullptr;
    }

    // selectedFile from QML FileDialog is a file:// URL; convert to a local path.
    QString localName = QUrl(name).toLocalFile();
    if (localName.isEmpty())
        localName = name;

    std::string nameStr = localName.toStdString();
    char *file_name = (char *) calloc((nameStr.size()+1), sizeof(char)); 
    memcpy(file_name, nameStr.c_str(), nameStr.size());

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
    if (!cProof->everything || !cProof->goals)
    {
        qDebug() << "Cannot populate UI: proof lists are null.";
        return;
    }

    int s = openTo->lines().size();
    for (int i = 0; i < s; i++)
        openTo->removeLineAt(0);
    //    openTo->lines().clear();

    item_t * pf_itr;
    int d = 0;
    for (pf_itr = cProof->everything->head; pf_itr; pf_itr = pf_itr->next){
        sen_data *sd = (sen_data *) pf_itr->value;
        if (!sd)
            continue;
        QList<int> temp_refs = {-1};
        for (int i = 0; sd->refs && i < 5000 && sd->refs[i] != REF_END; i++)
            temp_refs.push_back(sd->refs[i]);

        if (sd->depth > d)
            sd->rule = -2;
        bool is_end_sub = false;
        if (sd->line_num != 1 && pf_itr->prev && pf_itr->prev->value)
            is_end_sub = ((sen_data *) pf_itr->prev->value)->depth > sd->depth;
        const char *line_text = (const char *) (sd->text ? sd->text : (unsigned char *) "");
        openTo->insertLine(sd->line_num-1,sd->line_num,line_text,reverseRulesMap[sd->rule],(sd->depth > 0),
                           (sd->rule == -2),is_end_sub, sd->depth * 20,temp_refs);
        d = sd->depth;
    }

    int g = gls->glines().size();
    for (int i = 0; i < g; i++)
        gls->removegLineAt(0);

    int i = 0;
    for (pf_itr = cProof->goals->head; pf_itr; pf_itr = pf_itr->next){
        const char *goal_text = (const char *) (pf_itr->value ? pf_itr->value : (unsigned char *) "");
        gls->insertgLine(i,-2,false,goal_text);
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
    auto fileContentReady = [&open, this, &gls](const QString &fileName, const QByteArray &fileContent) {
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

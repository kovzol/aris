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

Connector::Connector(QObject *parent)
    : QObject{parent}, m_evalText{"Evaluate Proof"}
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

QString Connector::evalText() const
{
    return m_evalText;
}

void Connector::setEvalText(const QString &newEvalText)
{
    if (m_evalText == newEvalText)
        return;
    m_evalText = newEvalText;
    emit evalTextChanged();
}

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

void Connector::genProof(const ProofData *toBeEval)
{
    main_conns = gui_conns;
    int conn = 1;
    std::regex pat("[&|~$%@#!^:>]");
    cProof = proof_init();
    genIndices(toBeEval);
    for (int i = 0; i < toBeEval->lines().size(); i++){
        sen_data *sd;
        item_t *itm;
        unsigned char *temp_text;
        sd = (sen_data *) calloc (1, sizeof(sen_data));
        int *ind = (int *) calloc(m_indices[i].size(), sizeof(int));
        short *temp_refs = (short *) calloc(toBeEval->lines().at(i).pRefs.size(), sizeof(short));

        sd->line_num = i+1;
        sd->rule = rulesMap[toBeEval->lines().at(i).pType];
        sd->depth = toBeEval->lines().at(i).pInd/20;
        sd->premise = (sd->rule == -1)?1:0;
        sd->subproof = (sd->rule == -2)?1:0;
        sd->file = toBeEval->lines().at(i).fname;

        if (sd->rule == -2)
            sd->rule = -1;

        // Assign Indices
        for (int ii = 0; ii < m_indices[i].size(); ii++)
            ind[ii] = m_indices[i][ii];

        sd->indices = ind;

        // TODO : Cross-check Unicodes
        // Assign Text
        std::string str = toBeEval->lines().at(i).pText.toStdString();

        if (conn && std::regex_search(str,pat)){
            main_conns = cli_conns;
            conn = 0;
        }

        temp_text = (unsigned char *) calloc((strlen(str.c_str()))+1, sizeof(unsigned char));
        memcpy(temp_text, str.c_str(), strlen(str.c_str()));
        sd->text = temp_text;


        // Assign references
        if (toBeEval->lines().at(i).pRefs.size() == 1)
            temp_refs[0] = REF_END;
        else{
            for (int ii = 1; ii < toBeEval->lines().at(i).pRefs.size(); ii++)
                temp_refs[ii-1] = toBeEval->lines().at(i).pRefs.at(ii);
            temp_refs[toBeEval->lines().at(i).pRefs.size()-1] = REF_END;
        }
        sd->refs = temp_refs;

        // Insert into proof object
        itm = ls_ins_obj (cProof->everything, sd, cProof->everything->tail);
        if (!itm)
            qDebug() << "proof: could not insert sen_data ";
    }

    // TODO : Fix boolean
    cProof->boolean = 0;
}

void Connector::genGoals(const GoalData *toBeEval)
{

    for (int i = 0; i < toBeEval->glines().size(); i++){

        unsigned char *temp_text;
        item_t *itm;

        std::string str = toBeEval->glines().at(i).gText.toStdString();
        temp_text = (unsigned char *) calloc((strlen(str.c_str()))+1, sizeof(unsigned char));
        memcpy(temp_text, str.c_str(), strlen(str.c_str()));

        itm = ls_ins_obj (cProof->goals, temp_text, cProof->goals->tail);
        if (!itm)
            qDebug() << "proof: could not insert goal data ";

    }
}

int Connector::evalProof(const ProofData *toBeEval, const GoalData *gls)
{
    genProof(toBeEval);
    genGoals(gls);
//    vec_t *rets;
    returns = init_vec(sizeof(char *));

    if (!proof_eval(cProof,returns,1))
        qDebug() << "Proof Evaluated Successfully";
    else
        qDebug() << "Memory Error";

    item_t * ev_itr;
    int f = 0;
    ev_itr = cProof->everything->head;
    for (int i = 0; i < returns->num_stuff; i++){
        char * cur_ret, * cur_line;
        cur_ret = (char *) vec_str_nth (returns, i);
        cur_line =(char *) ((sen_data *) ev_itr->value)->text;

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
    if (!f) setEvalText("Correct!");

    return 1;
}

void Connector::saveProof(const QString &name, const ProofData *toBeSaved, const GoalData *gls)
{
    genProof(toBeSaved);
    genGoals(gls);

    char *file_name = (char *) calloc((name.size()+1), sizeof(char));
    memcpy(file_name, name.toStdString().c_str(), name.size());

    if (aio_save(cProof,(const char *) file_name) == 0)
        qDebug() << "File Saved Successfully";
    if (file_name)
        free(file_name);
}

void Connector::openProof(const QString &name, ProofData *openTo, GoalData *gls)
{

    char *file_name = (char *) calloc((name.size()+1), sizeof(char));
    memcpy(file_name, name.toStdString().c_str(), name.size());

    cProof = aio_open((const char *) file_name);
    if (cProof)
        qDebug() << "File Opened Successfully";
    if (file_name)
        free(file_name);

    reverseMapInit();
    int s = openTo->lines().size();
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


void Connector::wasmSaveProof(const ProofData * pd, const GoalData *gls)
{
    saveProof("temp.tle",pd,gls);
    QFile file("temp.tle");
    file.open(QIODevice::ReadOnly);
    QFileDialog::saveFileContent(file.readAll(),"Untitled.tle");
    file.remove("temp.tle");
}

proof_t *Connector::getCProof() const
{
    return cProof;
}

vec_t *Connector::getReturns() const
{
    return returns;
}

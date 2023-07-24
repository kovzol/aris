#include "connector.h"
#include "proof.h"
#include "process.h"
#include "typedef.h"
#include "sen-data.h"
#include "list.h"
#include "vec.h"
#include "aio.h"

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
    rulesMap["Sequence"] = 29;                      rulesMap["Induction"] = 30;                     rulesMap["Identity"] = 31;
    rulesMap["Negation"] = 32;                      rulesMap["Dominance"] = 33;                     rulesMap["Symbol Negation"] = 34;
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
    reverseRulesMap[29] = "Sequence";                       reverseRulesMap[30] = "Induction";                      reverseRulesMap[31] = "Identity";
    reverseRulesMap[32] = "Negation";                       reverseRulesMap[33] = "Dominance";                      reverseRulesMap[34] = "Symbol Negation";
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
        for (const int subp: parent_subproof)
            m_indices[i].push_back(subp);

        if (toBeEval->lines().at(i).pType == "subproof")
            parent_subproof.push_back(i + 1);
        if ( i < toBeEval->lines().size() -1 && toBeEval->lines().at(i + 1).pInd < toBeEval->lines().at(i).pInd)
            parent_subproof.pop_back();
    }
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
        int *ind = (int *) calloc(m_indices.size(), sizeof(int));
        short *temp_refs = (short *) calloc(toBeEval->lines().at(i).pRefs.size(), sizeof(short));

        sd->line_num = i+1;
        sd->rule = rulesMap[toBeEval->lines().at(i).pType];
        sd->depth = toBeEval->lines().at(i).pInd/20;
        sd->premise = (sd->rule == -1)?1:0;
        sd->subproof = (sd->rule == 28)?1:0;

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

//        temp_text = (unsigned char *) calloc((toBeEval->lines().at(i).pText.size()+1), sizeof(unsigned char));
//        memcpy(temp_text, str.c_str(), toBeEval->lines().at(i).pText.size());
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

int Connector::evalProof(const ProofData *toBeEval)
{
    genProof(toBeEval);
    vec_t *rets;
    rets = init_vec(sizeof(char *));

    if (!proof_eval(cProof,rets,1))
        qDebug() << "Proof Evaluated Successfully";
    else
        qDebug() << "Memory Error";

    item_t * ev_itr;
    int f = 0;
    ev_itr = cProof->everything->head;
    for (int i = 0; i < rets->num_stuff; i++){
        char * cur_ret, * cur_line;
        cur_ret = (char *) vec_str_nth (rets, i);
        cur_line =(char *) ((sen_data *) ev_itr->value)->text;

        if (strcmp (cur_ret, CORRECT)){
            qDebug() << "Error in line " << i + 1 << "- " << cur_line;
            setEvalText(QString("Error in line %1 - %2").arg(i+1).arg(cur_line));
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

void Connector::saveProof(const QString &name, const ProofData *toBeSaved)
{
    genProof(toBeSaved);

    char *file_name = (char *) calloc((name.size()+1), sizeof(char));
    memcpy(file_name, name.toStdString().c_str(), name.size());

    if (aio_save(cProof,(const char *) file_name) == 0)
        qDebug() << "File Saved Successfully";
    if (file_name)
        free(file_name);
}

void Connector::openProof(const QString &name, ProofData *openTo)
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

    for (pf_itr = cProof->everything->head; pf_itr; pf_itr = pf_itr->next){
        sen_data *sd = (sen_data *) pf_itr->value;
        QList<int> temp_refs = {-1};
        for (int i = 0; sd->refs[i] != REF_END; i++)
            temp_refs.push_back(sd->refs[i]);

        openTo->insertLine(sd->line_num-1,sd->line_num,(const char *) sd->text,reverseRulesMap[sd->rule],(sd->depth > 0),
                           (sd->subproof == 1),(sd->line_num != 1 && ((sen_data *) pf_itr->prev->value)->depth > sd->depth), sd->depth * 20,temp_refs);
    }

    qDebug() << "Model Loaded Successfully";
}

void Connector::wasmOpenProof(ProofData *open)
{
    auto fileContentReady = [&open, this](const QString &fileName, const QByteArray &fileContent) {
        if (fileName.isEmpty()) {
            // No file was selected
        } else {
            QSaveFile file(fileName);
            file.open(QIODevice::WriteOnly);
            file.write(fileContent);
            file.commit();
//            QFileDialog::saveFileContent(fileContent,"idk");
            openProof(fileName,open);
        }
    };
    QFileDialog::getOpenFileContent("Aris Proof (*.tle)",  fileContentReady);
}

bool Connector::isWasm()
{
    #ifdef Q_OS_WASM
    return true;
    #endif
    return false;
}

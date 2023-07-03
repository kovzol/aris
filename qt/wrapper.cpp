#include "wrapper.h"

#include "test.h"
#include "proof.h"
//#include "process.h"
#include "typedef.h"
#include "sen-data.h"
#include "list.h"
#include "vec.h"

#include <QDebug>
#include <iostream>

Wrapper::Wrapper(QObject *parent)
    : QObject{parent}
{
    qDebug() << f();
//    std::cout << "hi";
//    qDebug() << test_proof_t();
}

void Wrapper::displayData()
{
    for (int i = 0; i < m_body.size(); i++){
        qDebug()<< i+ 1 <<" : text is  " << m_body[i] << " and rule is " << m_rules[i] << " and depth is "<< m_depth[i]<< " and subproof starter: " << m_subproof[i] << " and refs are: ";
        if (m_refs.size() > i){
        for (int ii = 0; ii < m_refs[i].size(); ii++)
            qDebug() << m_refs[i][ii] ;
        }
        qDebug() << " and indices are : " ;
        if (m_indices.size() > i){
        for (int ii = 0; ii < m_indices[i].size(); ii++)
            qDebug() << m_indices[i][ii] ;
        }
    }
    checkProof();
}

void Wrapper::clearData()
{
    m_body.clear();
    m_rules.clear();
    m_premise.clear();
    m_subproof.clear();
    m_indices.clear();
    m_depth.clear();
    m_refs.clear();
}

void Wrapper::textAppend(const QString &text)
{
    m_body << text;
}

void Wrapper::ruleAppend(const QString &rule)
{
    m_rules << rule;

    m_premise.append(rule == "premise");

    m_subproof.append(rule == "subproof");
}

void Wrapper::refsAppend(const int i, const int &ref)
{
    if (i == m_refs.size())
        m_refs.push_back({});
    m_refs[i].push_back(ref);
}

void Wrapper::depthAppend(const int &indent)
{
    m_depth.append(indent/20);
}

void Wrapper::computeIndices()
{
    QList<int> parent_subproof;
    for (int i = 0; i < m_rules.size(); i++){
        m_indices.push_back({});
        for (int ii = 0; ii < parent_subproof.size(); ii++){
        m_indices[i].push_back(parent_subproof[ii]);
        }
        if (m_rules[i] == "subproof")
            parent_subproof.push_back(i + 1);
        if ( i < m_rules.size() -1 && m_depth[i + 1] < m_depth[i])
            parent_subproof.pop_back();
    }
}

void Wrapper::computeRules()
{
    for (int i = 0; i < m_rules.size(); i++){
        QString temp = m_rules[i];
        int val = -1;

        // TODO : Fix this ugly if-else later

        if (temp == "Modus Ponens")
            val = 0;
        else if (temp == "Addition")
            val = 1;
        else if (temp == "Simplification")
            val = 2;
        else if (temp == "Conjunction")
            val = 3;
        else if (temp == "Hypothetical Syllogism")
            val = 4;
        else if (temp == "Disjunctive Syllogism")
            val = 5;
        else if (temp == "Excluded middle")
            val = 6;
        else if (temp == "Constructive Dilemma")
            val = 7;
        else if (temp == "Implication")
            val = 8;
        else if (temp == "DeMorgan")
            val = 9;
        else if (temp == "Association")
            val = 10;
        else if (temp == "Commutativity")
            val = 11;
        else if (temp == "Idempotence")
            val = 12;
        else if (temp == "Distribution")
            val = 13;
        else if (temp == "Equivalence")
            val = 14;
        else if (temp == "Double Negation")
            val = 15;
        else if (temp == "Exportation")
            val = 16;
        else if (temp == "Subsumption")
            val = 17;
        else if (temp == "Universal Generalization")
            val = 18;
        else if (temp == "Universal Instantiation")
            val = 19;
        else if (temp == "Existential Generalization")
            val = 20;
        else if (temp == "Existential Instantiation")
            val = 21;
        else if (temp == "Bound Variable Substitution")
            val = 22;
        else if (temp == "Null Quantifier")
            val = 23;
        else if (temp == "Prenex")
            val = 24;
        else if (temp == "Identity")
            val = 25;
        else if (temp == "Free Variable Substitution")
            val = 26;
        else if (temp == "Lemma")
            val = 27;
        else if (temp == "Subproof")
            val = 28;
        else if (temp == "Sequence")
            val = 29;
        else if (temp == "Induction")
            val = 30;
        else if (temp == "Identity")
            val = 31;
        else if (temp == "Negation")
            val = 32;
        else if (temp == "Dominance")
            val = 33;
        else if (temp == "Symbol Negation")
            val = 34;
        else
            val = -1;

        rules.push_back(val);
    }
}

int Wrapper::test_proof_t()
{
    proof_t * proof;
    proof = proof_init();

    if (!proof)
        return 0;
    return 1;
}

int Wrapper::checkProof()
{
    proof_t * proof;

    proof = proof_init ();
    if (!proof){
//        free(sd);
        return -10;
    }

    for (int i = 0; i < rules.size(); i++){
        sen_data * sd;
        item_t * itm;
        sd = (sen_data *) calloc (1, sizeof (sen_data));

        sd->line_num = i+1;
//        qDebug() << "rules";
        sd->rule = rules[i];
//        qDebug() << "depth" ;
        sd->depth = m_depth[i];
//        qDebug() << "indices";
        int ind[m_indices[i].size()];
        for (int ii = 0; i < m_indices[i].size();ii++){
            ind[ii] = m_indices[i][ii];
        }
//        int hmm[3] = {-1,0,0};
        sd->indices = ind;
//        qDebug() << "premise";
        sd->premise = (m_premise[i])?1:0;
//        qDebug() << "subproof";
        sd->subproof = (m_subproof[i])?1:0;
//        qDebug() << "text";
//        unsigned char * lol = ;
//        qDebug() << m_body[i];

        std::string buff = m_body[i].toStdString();
//        qDebug() << buff;
//        char * temp =(char *) malloc(sizeof(char *));
        char *temp = buff.data();

//        const char * temp = buff.c_str();
//        for (int ii = 0; ii < m_body[i].size(); ii++){
//            lol[ii] = temp[ii];
//        }
//        qDebug() << lol;
        sd->text = (unsigned char *) temp;
//        qDebug() << sd->text;
//        qDebug() << "refs";
//        short r[m_refs[i].size()];
//        qDebug() << "ok";
//        for (int ii = 0; i < m_refs[i].size();ii++){
//            r[ii] = m_refs[i][ii];
//        }
//        qDebug() << "hmm";
        short k[1] = {-1};
        sd->refs = k;
        itm = ls_ins_obj (proof->everything, sd, proof->everything->tail);
        if (!itm)
            qDebug() << "oh no";
    }
    qDebug() << "SD initialized succesfully" ;
    proof->boolean = 0;
    vec_t * rets;
    rets = init_vec(m_body.size());

    qDebug() << "proof evals to : " << proof_eval(proof,rets,1);

//    free(sd);
    return 1;
}


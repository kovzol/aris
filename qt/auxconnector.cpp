#include "auxconnector.h"
#include "aio.h"
#include "connector.h"
#include "list.h"
#include "proof.h"
#include "rules.h"
#include "sen-data.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QSaveFile>

auxConnector::auxConnector(QObject *parent)
    : QObject{parent}
{

}

void auxConnector::latex(const QString &name, const ProofData *toBeEval, Connector *c)
{
    c->genProof(toBeEval);
    QString newName = name.contains("file://")? name.mid(7): name;
    char *file_name = (char *) calloc((newName.size()+1), sizeof(char));
    memcpy(file_name, newName.toStdString().c_str(), newName.size());
    qDebug() << file_name;
    if (convert_proof_latex(c->getCProof(),file_name) == 0){
        qDebug() << "Latex conversion successful";
    }
    else
        qDebug() << "Memory Error";
    if (file_name)
        free(file_name);

}

void auxConnector::wasmLatex(const ProofData *pd, Connector *c)
{
    latex("temp.tex",pd,c);
    QFile file("temp.tex");
    file.open(QIODevice::ReadOnly);
    QFileDialog::saveFileContent(file.readAll(),"Untitled.tex");
    file.remove("temp.tex");

}

void auxConnector::importProof(const QString &name, ProofData *pd, Connector *c, ProofModel *pm)
{
    QString newName = name.contains("file://")? name.mid(7): name;
    char *file_name = (char *) calloc((newName.size()+1), sizeof(char));
    memcpy(file_name, newName.toStdString().c_str(), newName.size());

    proof_t *proof = aio_open(file_name);

    item_t *pf_itr;
    int ref_num = 0, ev_conc = -1, ev_itr;
    short *refs;

    refs = (short *) calloc (proof->everything->num_stuff, sizeof(int));

    int num_ins = 0;
    for (pf_itr =(item_t *) proof->everything->head; pf_itr != NULL; pf_itr = pf_itr->next){
        sen_data *sd;
        char *pf_text;

        sd = (sen_data *) pf_itr->value;
        if (!sd->premise)
            break;

        pf_text = (char *) sd->text;

        for (ev_itr = 0; ev_itr < pd->lines().size(); ev_itr++){
            char *ev_text;
            int ln = ev_itr + 1;

            if (pd->lines().at(ev_itr).pType != "premise"){
                if (ev_conc == -1)
                    ev_conc = ev_itr;
                break;
            }

            std::string str = pd->lines().at(ev_itr).pText.toStdString();
            ev_text = (char *) calloc((strlen(str.c_str()))+1, sizeof(char));
            memcpy(ev_text, str.c_str(), strlen(str.c_str()));

            if (!strcmp(ev_text, pf_text)){
                refs[ref_num++] = (short) ln;
                free(ev_text);
                break;
            }
            if (ev_text)
                free(ev_text);
        }

        if (ev_itr >= pd->lines().size() || pd->lines().at(ev_itr).pType != "premise"){
            QList<int> temp_refs = {-1};
            for (int i = 0; sd->refs[i] != REF_END; i++)
                temp_refs.push_back(sd->refs[i]);

            if (sd->depth > 0)
                sd->rule = -2;
            pd->insertLine(num_ins,num_ins+1,(const char *) sd->text,c->reverseRulesMap[sd->rule],false,
                               false,false, sd->depth * 20,temp_refs);
            pm->updateLines();
            pm->updateRefs(num_ins,true);
            num_ins++;

            refs[ref_num++] = (short) num_ins;
        }

    }

    refs[ref_num] = REF_END;

    if (ev_conc == -1){
        for (ev_itr = 0; ev_itr < pd->lines().size(); ev_itr++){
            if (pd->lines().at(ev_itr).pType == "premise"){
                if (ev_conc == -1)
                    ev_conc = ev_itr;
                break;
            }
        }
    }

    for (pf_itr = proof->goals->head; pf_itr != NULL; pf_itr = pf_itr->next){

        unsigned char *pf_text;
        pf_text = (unsigned char *) pf_itr->value;

        for (ev_itr = ev_conc; ev_itr < pd->lines().size(); ev_itr++){

            unsigned char *ev_text;
            std::string str = pd->lines().at(ev_itr).pText.toStdString();
            ev_text = (unsigned char *) calloc((strlen(str.c_str()))+1, sizeof(unsigned char));
            memcpy(ev_text, str.c_str(), strlen(str.c_str()));


            if (!strcmp((char *)pf_text,(char *)ev_text)){
                free(ev_text);
                break;
            }
            if (ev_text)
                free(ev_text);
        }

        if (ev_itr >= pd->lines().size()){
            sen_data *sd;
            sd = sen_data_init(-1,RULE_LM,(unsigned char *)pf_text,refs,0,(unsigned char *)file_name,0,0,NULL);
            qDebug()<< file_name;

            QList<int> temp_refs = {-1};
            for (int i = 0; sd->refs[i] != REF_END; i++)
                temp_refs.push_back(sd->refs[i]);

            int l;
            for (l = 0; l < pd->lines().size(); l++){
                if (pd->lines().at(l).pType != "premise")
                    break;
            }

            pd->insertLine(l,l+1,(const char *) sd->text,c->reverseRulesMap[sd->rule],false,
                           false,false, sd->depth * 20,temp_refs);
            pd->setFile(l,newName);
            pm->updateLines();
            pm->updateRefs(l,true);

        }

    }

    free(refs);
    proof_destroy(proof);
    if (file_name)
        free(file_name);
}

void auxConnector::wasmImportProof(ProofData *pd, Connector *c, ProofModel *pm)
{
    auto fileContentReady = [this, &c, &pd, &pm](const QString &fileName, const QByteArray &fileContent) {
        if (fileName.isEmpty()) {
            qDebug() << "No file was selected" ;
        } else {
            QSaveFile file(fileName);
            file.open(QIODevice::WriteOnly);
            file.write(fileContent);
            file.commit();
            importProof(fileName,pd,c,pm);
//            file.deleteLater();
        }
    };
    QFileDialog::getOpenFileContent("Aris Proof (*.tle)",  fileContentReady);
}

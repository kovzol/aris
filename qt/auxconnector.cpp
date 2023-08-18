#include "auxconnector.h"
#include "aio.h"
#include "list.h"
#include "proof.h"
#include "sen-data.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>

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

void auxConnector::importProof(const QString &name, ProofData *pd)
{
//    QString newName = name.contains("file://")? name.mid(7): name;
//    char *file_name = (char *) calloc((newName.size()+1), sizeof(char));
//    memcpy(file_name, newName.toStdString().c_str(), newName.size());

//    proof_t *proof = aio_open(file_name);

//    item_t *pf_itr;
//    int ref_num = 0, ev_conc = -1, ev_itr;
//    short *refs;

//    refs = (short *) calloc (proof->everything->num_stuff, sizeof(int));

//    for (pf_itr =(item_t *) proof->everything->num_stuff; pf_itr; pf_itr = pf_itr->next){
//        sen_data *sd;
//        char *pf_text;

//        sd = (sen_data *) pf_itr->value;
//        if (!sd->premise)
//            break;

//        pf_text = (char *) sd->text;

//        for (ev_itr = 0; ev_itr < pd->lines().size(); ev_itr++){
//            char *ev_text;
//            int ln = ev_itr + 1;

//            if (pd->lines().at(ev_itr).pType != "premise"){
//                if (ev_conc == -1)
//                    ev_conc = ev_itr;
//                break;
//            }

//            std::string str = pd->lines().at(ev_itr).pText.toStdString();
//            ev_text = (char *) calloc((strlen(str.c_str()))+1, sizeof(char));
//            memcpy(ev_text, str.c_str(), strlen(str.c_str()));

//            if (!strcmp(ev_text, pf_text)){
//                refs[ref_num++] = (short) ln;
//                break;
//            }
//        }

//        if (ev_itr >= pd->lines().size() || pd->lines().at(ev_itr).pType != "premise"){
//            // insert this as premise
//        }

//    }

//    if (file_name)
    //        free(file_name);
}

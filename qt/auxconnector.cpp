/* Auxiliary Connector Class for operations not included in Connector.

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
#include "auxconnector.h"
#include "../src/aio.h"
#include "connector.h"
#include "../src/list.h"
#include "../src/proof.h"
#include "../src/rules.h"
#include "../src/sen-data.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QSaveFile>

#ifndef Q_OS_WASM
#include <QProcess>
#endif

auxConnector::auxConnector(QObject *parent)
    : QObject{parent}
{

}

/* Exports the proof to a latex file.
 *  input:
 *    name      - filename for the to be saved file.
 *    toBeEval  - pointer to the ProofData object.
 *    c         - pointer to the Connector object.
 *  output:
 *    none.
 */
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

/* Exports the proof to a latex file (for WebAssembly).
 *  input:
 *    pd - pointer to the ProofData object.
 *    c  - pointer to the Connector object.
 *  output:
 *    none.
 */
void auxConnector::wasmLatex(const ProofData *pd, Connector *c)
{
    latex("temp.tex",pd,c);
    QFile file("temp.tex");
    file.open(QIODevice::ReadOnly);
    QFileDialog::saveFileContent(file.readAll(),"Untitled.tex");
    file.remove("temp.tex");

}

/* Imports a proof into the current proof.
 *  input:
 *    name  - filename of the imported proof file.
 *    pd    - pointer to the ProofData object.
 *    c     - pointer to the Connector object.
 *    pm    - pointer to the ProofModel object.
 *  output:
 *    none.
 */
void auxConnector::importProof(const QString &name, ProofData *pd, const Connector *c, ProofModel *pm)
{
    QString newName = name.contains("file://")? name.mid(7): name;
    char *file_name = (char *) calloc((newName.size()+1), sizeof(char));
    memcpy(file_name, newName.toStdString().c_str(), newName.size());

    proof_t *proof = aio_open(file_name);

    item_t *pf_itr;
    int ref_num = 0, ev_conc = -1, ev_itr;
    short *refs;

    refs = (short *) calloc (proof->everything->num_stuff, sizeof(int));

    int num_ins = 1;
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
//            for (int i = 0; sd->refs[i] != REF_END; i++)
//                temp_refs.push_back(sd->refs[i]);

            if (sd->depth > 0)
                sd->rule = -2;
            pd->insertLine(num_ins,num_ins+1,(const char *) sd->text,c->reverseRulesMap[sd->rule],false,
                               false,false, sd->depth * 20,temp_refs);
            pd->setFile(num_ins,newName);
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

/* Imports a proof into the current proof (for WebAssembly).
 *  input:
 *    pd    - pointer to the ProofData object.
 *    c     - pointer to the Connector object.
 *    pm    - pointer to the ProofModel object.
 *  output:
 *    none.
 */
void auxConnector::wasmImportProof(ProofData *pd, const Connector *c, ProofModel *pm)
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

#ifndef Q_OS_WASM

/* Starts Aris as a new detached process (for Desktop).
 *  input:
 *    none.
 *  output:
 *    none.
 */
void auxConnector::newWindow()
{
    QString program = "./aris-qt";
    QStringList arguments;

    QProcess *myProcess = new QProcess(this);
    myProcess->startDetached(program, arguments);
    delete myProcess;
}
#endif

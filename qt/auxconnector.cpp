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
#include <QUrl>
#include <QDir>
#include <QStandardPaths>

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
void auxConnector::importProof(const QString &name, const QString &mode, ProofData *pd, Connector *c, ProofModel *pm)
{
    c->reverseMapInit();

    QString newName = QUrl(name).toLocalFile();
    if (newName.isEmpty())
        newName = name;
    char *file_name = (char *) calloc((newName.size()+1), sizeof(char));
    memcpy(file_name, newName.toStdString().c_str(), newName.size());

    qDebug() << "Import proof path:" << newName;
    proof_t *proof = aio_open(file_name);

    if (!proof) {
        qDebug() << "Failed to import proof for path:" << newName;
        c->setEvalText("Import failed: could not open file");
        free(file_name);
        return;
    }

    const QString normalized = mode.trimmed().toLower();
    const bool isAppend = (normalized == "append");
    const bool isPrepend = (normalized == "prepend");
    const bool isOverwrite = (!isAppend && !isPrepend);

    if (isOverwrite) {
        while (pd->lines().size() > 0) {
            pd->removeLineAt(0);
        }
        pm->updateLines();
    }

    int insertIndex = isAppend ? pd->lines().size() : 0;
    const int refOffset = isAppend ? insertIndex : 0;
    int insertedCount = 0;

    item_t *pf_itr;
    int prevDepth = 0;
    for (pf_itr = (item_t *) proof->everything->head; pf_itr; pf_itr = pf_itr->next) {
        sen_data *sd = (sen_data *) pf_itr->value;

        if (sd->depth > prevDepth)
            sd->rule = -2;

        QList<int> temp_refs = {-1};
        for (int i = 0; sd->refs[i] != REF_END; i++) {
            int ref = sd->refs[i];
            if (refOffset > 0 && ref > 0)
                ref += refOffset;
            temp_refs.push_back(ref);
        }

        bool subEnd = (sd->line_num != 1 && ((sen_data *) pf_itr->prev->value)->depth > sd->depth);

        pd->insertLine(insertIndex, insertIndex + 1, (const char *) sd->text,
                       c->reverseRulesMap[sd->rule], (sd->depth > 0),
                       (sd->rule == -2), subEnd, sd->depth * 20, temp_refs);
        pd->setFile(insertIndex, newName);

        if (isPrepend)
            pm->updateRefs(insertIndex, true);

        insertIndex++;
        insertedCount++;
        prevDepth = sd->depth;
    }

    if (insertedCount > 0)
        pm->updateLines();
    c->setEvalText("Import complete");
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
void auxConnector::wasmImportProof(const QString &mode, ProofData *pd, Connector *c, ProofModel *pm)
{
    auto fileContentReady = [this, &c, &pd, &pm, mode](const QString &fileName, const QByteArray &fileContent) {
        if (fileName.isEmpty()) {
            qDebug() << "No file was selected" ;
            c->setEvalText("Import failed: no file selected");
        } else {
            QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            if (tempDir.isEmpty())
                tempDir = ".";
            const QString tempName = QDir(tempDir).filePath("import-temp.tle");
            QFile file(tempName);
            if (!file.open(QIODevice::WriteOnly)) {
                qDebug() << "Failed to write temp import file:" << tempName;
                c->setEvalText("Import failed: could not write temp file");
                return;
            }
            file.write(fileContent);
            file.flush();
            file.close();
            if (!QFile::exists(tempName)) {
                qDebug() << "Temp import file missing after write:" << tempName;
                c->setEvalText("Import failed: temp file missing");
                return;
            }
            importProof(tempName, mode, pd, c, pm);
        }
    };
    QFileDialog::getOpenFileContent("Aris Proof (*.tle)",  fileContentReady);
}

void auxConnector::resetProof(ProofData *pd, GoalData *gd, ProofModel *pm)
{
    while (pd->lines().size() > 0) {
        pd->removeLineAt(0);
    }
    pd->insertLine(0, 1, "", "premise", false, false, false, 0, {-1});
    pm->updateLines();

    while (gd->glines().size() > 0) {
        gd->removegLineAt(0);
    }
    gd->insertgLine(0, -2, false, "");
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

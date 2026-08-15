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
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextDocumentWriter>
#ifndef Q_OS_WASM
#include <QPrinter>
#endif

#ifndef Q_OS_WASM
#include <QProcess>
#endif

namespace {
enum ImportMode {
    ImportOverwrite = 0,
    ImportAppendEnd = 1,
    ImportPrepend = 2
};

bool hasOnlyBlankProof(const ProofData *pd)
{
    const QVector<ProofLine> lines = pd->lines();

    if (lines.size() != 1)
        return false;

    const ProofLine &line = lines.at(0);
    return line.pType == "premise" && line.pText.isEmpty() && line.pInd == 0;
}

void clearProofLines(ProofData *pd)
{
    while (!pd->lines().isEmpty())
        pd->removeLineAt(0);
}

void shiftReferences(ProofData *pd, ProofModel *pm, int startIndex, int delta)
{
    for (int i = startIndex; i < pd->lines().size(); ++i) {
        QList<int> refs = pd->lines().at(i).pRefs;

        for (int refIndex = 1; refIndex < refs.size(); ++refIndex) {
            if (refs[refIndex] >= 0)
                refs[refIndex] += delta;
        }

        QList<QVariant> updatedRefs;
        for (int ref: refs)
            updatedRefs.append(ref);

        pm->setData(pm->index(i, 0), updatedRefs, ProofModel::RefsRole);
    }
}

QList<int> shiftedRefs(const QList<int> &refs, int delta)
{
    QList<int> adjusted = refs;

    for (int i = 1; i < adjusted.size(); ++i) {
        if (adjusted[i] >= 0)
            adjusted[i] += delta;
    }

    return adjusted;
}
}

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
    else {
        qDebug() << "Memory Error";
        emit errorOccurred(tr("LaTeX export failed: could not convert proof to LaTeX format."));
    }
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

    if (!proof) {
        qDebug() << "Failed to import proof";
        emit errorOccurred(tr("Import failed: the selected file could not be opened or is not a valid Aris proof."));
        free(file_name);
        emit importFinished(false);
        return;
    }

    item_t *pf_itr;
    int ref_num = 0, ev_conc = -1, ev_itr;
    short *refs;

    refs = (short *) calloc (proof->everything->num_stuff, sizeof(int));

    while (pd->lines().size() > 0) {
        pd->removeLineAt(0);
    }
    
    // 2. TELL THE UI THE LIST IS NOW EMPTY 
    pm->updateLines(); 

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
//            for (int i = 0; sd->refs[i] != REF_END; i++)
//                temp_refs.push_back(sd->refs[i]);

            if (sd->depth > 0)
                sd->rule = -2;
            {
                auto ci = Connector::getCategoryAndIndex(sd->rule);
                pd->insertLine(num_ins,num_ins+1,(const char *) sd->text,c->reverseRulesMap[sd->rule],false,
                                   false,false, sd->depth * 20,temp_refs, ci.first, ci.second);
            }
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

    int l;
    for (l = 0; l < pd->lines().size(); l++){
        if (pd->lines().at(l).pType != "premise")
            break;
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

            {
                auto ci = Connector::getCategoryAndIndex(sd->rule);
                pd->insertLine(l,l+1,(const char *) sd->text,c->reverseRulesMap[sd->rule],false,
                               false,false, sd->depth * 20,temp_refs, ci.first, ci.second);
            }
            pd->setFile(l,newName);
            pm->updateLines();
            pm->updateRefs(l,true);
            l++;
        }

    }

    free(refs);
    proof_destroy(proof);
    if (file_name)
        free(file_name);

    emit importFinished(true);
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
    auto fileContentReady = [this, c, pd, pm](const QString &fileName, const QByteArray &fileContent) {
        if (fileName.isEmpty()) {
            qDebug() << "No file was selected" ;
            emit importFinished(false);
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

void auxConnector::importProofWithMode(const QString &name, ProofData *pd, const Connector *c, ProofModel *pm, int mode)
{
    if (mode < ImportOverwrite || mode > ImportPrepend)
        mode = ImportOverwrite;

    const bool blankProof = hasOnlyBlankProof(pd);

    if (mode == ImportOverwrite || blankProof) {
        importProof(name, pd, c, pm);
        return;
    }

    ProofData importedData;
    ProofModel importedModel;
    importedModel.setlines(&importedData);
    importProof(name, &importedData, c, &importedModel);

    const QVector<ProofLine> importedLines = importedData.lines();
    const int existingCount = pd->lines().size();
    const int insertIndex = (mode == ImportAppendEnd) ? existingCount : 0;
    const int refDelta = (mode == ImportAppendEnd) ? existingCount : 0;

    for (int i = 0; i < importedLines.size(); ++i) {
        const ProofLine &line = importedLines.at(i);
        const int targetIndex = insertIndex + i;
        const QList<int> refs = shiftedRefs(line.pRefs, refDelta);

        pd->insertLine(targetIndex, targetIndex + 1, line.pText, line.pType,
                       line.pSub, line.pSubStart, line.pSubEnd, line.pInd, refs,
                       line.pRuleCategory, line.pRuleIndex);

        if (line.fname)
            pd->setFile(targetIndex, QString::fromUtf8((const char *) line.fname));
    }

    pm->updateLines();

    if (mode == ImportPrepend && existingCount > 0)
        shiftReferences(pd, pm, importedLines.size(), importedLines.size());
}

void auxConnector::wasmImportProofWithMode(ProofData *pd, const Connector *c, ProofModel *pm, int mode)
{
    auto fileContentReady = [this, c, pd, pm, mode](const QString &fileName, const QByteArray &fileContent) {
        if (fileName.isEmpty()) {
            qDebug() << "No file was selected" ;
            emit importFinished(false);
        } else {
            QSaveFile file(fileName);
            file.open(QIODevice::WriteOnly);
            file.write(fileContent);
            file.commit();
            importProofWithMode(fileName, pd, c, pm, mode);
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

// Plain-text export helper

/* Builds a plain-text representation of one ProofLine.
 *  depth   = pInd / 20 (0 = top-level, 1 = one subproof deep, …)
 *  prefix  = box-drawing hint: "┌─" for sf rows, "└─" for the last row
 *            before a subproof end, "" for everything else.
 *  Returns a single formatted line with trailing newline.
 */
static QString proofLineToText(const ProofLine &pl, int depth, const QString &prefix)
{
    // Skip the closing subproof sentinel row — the └─ line already closes it.
    if (pl.pSubEnd)
        return QString();

    // Indent: 2 spaces per depth level, then the box-drawing prefix.
    QString indent = QString("  ").repeated(depth);
    if (!prefix.isEmpty())
        indent += prefix + " ";
    else if (depth > 0)
        indent += "  ";          // align content rows with the box interior

    // Line number, right-aligned in a 3-char field.
    QString lineNum = QString::number(pl.pLine).rightJustified(3);

    // Build refs string: "1, 2, 3" (skip sentinel -1).
    QStringList refStrs;
    for (int r : pl.pRefs)
        if (r != -1) refStrs << QString::number(r);
    QString refs = refStrs.join(", ");

    // Rule / type label (omit for sf rows — the box-drawing says enough).
    QString rule;
    if (pl.pSubStart)
        rule = "(subproof start)";
    else if (pl.pType == QLatin1String("comment"))
        rule = "";
    else if (!pl.pType.isEmpty() && pl.pType != "choose")
        rule = pl.pType + (refs.isEmpty() ? "" : ": " + refs);

    // Compose: "  ┌─  3.  P → Q          [→ Elim: 1, 2]"
    QString formula = pl.pText.isEmpty() ? "(empty)" : pl.pText;
    if (pl.pType == QLatin1String("comment")) formula = "// " + pl.pText;
    
    QString left = (pl.pType == QLatin1String("comment")) ? 
                   (indent + "      " + formula) : 
                   (indent + lineNum + ".  " + formula);
    QString ruleTag = rule.isEmpty() ? "" : "  [" + rule + "]";

    // Pad the formula area to a fixed column so rule tags align.
    const int targetCol = 60;
    if (left.length() < targetCol)
        left = left.leftJustified(targetCol);

    return left + ruleTag + "\n";
}

/* Exports the proof to a plain-text file.
 *  Reads directly from ProofData — no C engine call needed.
 *  input:
 *    name - absolute file path (may have "file://" prefix).
 *    pd   - pointer to the ProofData object.
 *  output:
 *    none (emits errorOccurred on failure).
 */
void auxConnector::exportText(const QString &name, const ProofData *pd)
{
    QString path = name.startsWith("file://") ? name.mid(7) : name;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred(tr("Text export failed: could not open '%1' for writing.").arg(path));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    const QVector<ProofLine> &lines = pd->lines();
    const int n = lines.size();

    // Pre-compute which rows are "last before a subproof end" so we can
    // add the └─ prefix to them.
    QSet<int> closingRows;
    for (int i = 1; i < n; ++i)
        if (lines.at(i).pSubEnd)
            closingRows.insert(i - 1);

    out << "Proof\n" << QString("=").repeated(72) << "\n\n";

    for (int i = 0; i < n; ++i) {
        const ProofLine &pl = lines.at(i);
        int depth = pl.pInd / 20;

        QString prefix;
        if (pl.pSubStart)
            prefix = "\u250c\u2500";                   // ┌─
        else if (closingRows.contains(i))
            prefix = "\u2514\u2500";                   // └─

        QString row = proofLineToText(pl, depth, prefix);
        if (!row.isEmpty())
            out << row;
    }

    out << "\n" << QString("=").repeated(72) << "\n";
    file.close();
    qDebug() << "[ARIS] exportText: wrote" << path;
}

/* Exports the proof to a plain-text file (WebAssembly).
 *  Writes to a temp file then triggers browser download.
 */
void auxConnector::wasmExportText(const ProofData *pd)
{
    exportText("aris_export_tmp.txt", pd);
    QFile file("aris_export_tmp.txt");
    if (file.open(QIODevice::ReadOnly)) {
        QFileDialog::saveFileContent(file.readAll(), "proof.txt");
        file.close();
    }
    QFile::remove("aris_export_tmp.txt");
}

//  Markdown export helper

/* Exports the proof to a Markdown file (.md).
 *  Produces a GitHub-renderable table where the Formula column is
 *  padded with non-breaking spaces (&#160;) for nesting depth.
 *  Subproof start/end rows become visual separator rows in the table.
 *  input:
 *    name - absolute file path.
 *    pd   - pointer to the ProofData object.
 *  output:
 *    none (emits errorOccurred on failure).
 */
void auxConnector::exportMarkdown(const QString &name, const ProofData *pd)
{
    QString path = name.startsWith("file://") ? name.mid(7) : name;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred(tr("Markdown export failed: could not open '%1' for writing.").arg(path));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    const QVector<ProofLine> &lines = pd->lines();
    const int n = lines.size();

    out << "# Proof\n\n";
    out << "| # | Formula | Rule | Refs |\n";
    out << "|---|---------|------|------|\n";

    for (int i = 0; i < n; ++i) {
        const ProofLine &pl = lines.at(i);
        int depth = pl.pInd / 20;

        // Non-breaking space indent (4 per level) for the formula column.
        QString nbsp  = QString("&nbsp;").repeated(depth * 4);

        if (pl.pSubStart) {
            // Subproof opening row — show as a light separator with label.
            out << "| | " << nbsp << "*subproof start* | | |\n";
            continue;
        }
        if (pl.pSubEnd) {
            // Subproof closing row — show as a light separator.
            out << "| | " << nbsp << "*subproof end* | | |\n";
            continue;
        }
        if (pl.pType == QLatin1String("comment")) {
            QString commentText = pl.pText;
            commentText.replace("|", "\\|");
            out << "| | " << nbsp << "*// " << commentText << "* | | |\n";
            continue;
        }

        // Regular line.
        QString lineNum = QString::number(pl.pLine);
        QString formula = pl.pText.isEmpty() ? "*(empty)*" : pl.pText;

        // Rule label (blank for premises displayed as "premise").
        QString rule = (pl.pType == "premise") ? "premise" : pl.pType;
        if (rule == "choose") rule = "";

        // Refs: "1, 2" (skip -1 sentinel).
        QStringList refStrs;
        for (int r : pl.pRefs)
            if (r != -1) refStrs << QString::number(r);
        QString refs = refStrs.join(", ");

        // Escape pipe characters in formula so the table doesn't break.
        formula.replace("|", "\\|");
        rule.replace("|", "\\|");

        out << "| " << lineNum
            << " | " << nbsp << formula
            << " | " << rule
            << " | " << refs
            << " |\n";
    }

    out << "\n*Generated by GNU Aris*\n";
    file.close();
    qDebug() << "[ARIS] exportMarkdown: wrote" << path;
}

/* Exports the proof to a Markdown file (WebAssembly).
 *  Writes to a temp file then triggers browser download.
 */
void auxConnector::wasmExportMarkdown(const ProofData *pd)
{
    exportMarkdown("aris_export_tmp.md", pd);
    QFile file("aris_export_tmp.md");
    if (file.open(QIODevice::ReadOnly)) {
        QFileDialog::saveFileContent(file.readAll(), "proof.md");
        file.close();
    }
    QFile::remove("aris_export_tmp.md");
}

// ---------------------------------------------------------------------------
// Shared helper: build a QTextDocument representation of the proof.
// Used by both exportDocx and exportPdf so formatting is identical.
// ---------------------------------------------------------------------------
static QTextDocument *buildProofDocument(const ProofData *pd)
{
    QTextDocument *doc = new QTextDocument();
    QTextCursor    cur(doc);

    // ---- Title ----
    QTextCharFormat titleFmt;
    titleFmt.setFontWeight(QFont::Bold);
    titleFmt.setFontPointSize(16);
    cur.insertText(QStringLiteral("Proof"), titleFmt);

    QTextCharFormat normalFmt;
    normalFmt.setFontPointSize(11);
    cur.insertBlock();
    cur.insertText(QString(), normalFmt);

    const QVector<ProofLine> &lines = pd->lines();
    const int n = lines.size();

    // ---- Table ----
    // Columns: #  |  Formula  |  Rule  |  Refs
    QTextTableFormat tableFmt;
    tableFmt.setHeaderRowCount(1);
    tableFmt.setCellPadding(4);
    tableFmt.setCellSpacing(0);
    tableFmt.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFmt.setBorder(1);
    tableFmt.setWidth(QTextLength(QTextLength::PercentageLength, 100));

    // Column widths: 5% | 55% | 25% | 15%
    QVector<QTextLength> colWidths;
    colWidths << QTextLength(QTextLength::PercentageLength,  5)
              << QTextLength(QTextLength::PercentageLength, 55)
              << QTextLength(QTextLength::PercentageLength, 25)
              << QTextLength(QTextLength::PercentageLength, 15);
    tableFmt.setColumnWidthConstraints(colWidths);

    QTextTable *table = cur.insertTable(n + 1, 4, tableFmt);

    // Header row
    QTextCharFormat hdrFmt;
    hdrFmt.setFontWeight(QFont::Bold);
    const QStringList headers = { QStringLiteral("#"),
                                   QStringLiteral("Formula"),
                                   QStringLiteral("Rule"),
                                   QStringLiteral("Refs") };
    for (int c = 0; c < 4; ++c) {
        QTextCursor cell = table->cellAt(0, c).firstCursorPosition();
        cell.insertText(headers.at(c), hdrFmt);
    }

    // Data rows
    for (int i = 0; i < n; ++i) {
        const ProofLine &pl = lines.at(i);
        int depth = pl.pInd / 20;

        // Indentation prefix (em-space per depth level)
        QString indent = QString(QChar(0x2003)).repeated(depth);  // EM SPACE

        // Formula cell
        QString formula;
        if (pl.pSubStart)
            formula = indent + QStringLiteral("\u250c\u2500 subproof");  // ┌─
        else if (pl.pSubEnd)
            formula = indent + QStringLiteral("\u2514\u2500");            // └─
        else if (pl.pType == QLatin1String("comment"))
            formula = indent + QStringLiteral("// ") + pl.pText;
        else
            formula = indent + (pl.pText.isEmpty() ? QStringLiteral("(empty)") : pl.pText);

        // Rule
        QString rule;
        if (!pl.pSubStart && !pl.pSubEnd && pl.pType != QLatin1String("comment")) {
            rule = (pl.pType == QLatin1String("premise")) ? QStringLiteral("premise") : pl.pType;
            if (rule == QLatin1String("choose")) rule = QString();
        }

        // Refs
        QStringList refStrs;
        for (int r : pl.pRefs)
            if (r != -1) refStrs << QString::number(r);
        QString refs = refStrs.join(QStringLiteral(", "));

        auto insertCell = [&](int col, const QString &txt, bool italic = false) {
            QTextCursor c = table->cellAt(i + 1, col).firstCursorPosition();
            QTextCharFormat f = normalFmt;
            if (italic) f.setFontItalic(true);
            if (pl.pType == QLatin1String("comment")) f.setForeground(QColor(128, 128, 128));
            c.insertText(txt, f);
        };

        insertCell(0, pl.pSubStart || pl.pSubEnd || pl.pType == QLatin1String("comment") ? QString() : QString::number(pl.pLine));
        insertCell(1, formula, pl.pType == QLatin1String("comment"));
        insertCell(2, rule);
        insertCell(3, refs);
    }

    // Footer note
    cur = doc->rootFrame()->lastCursorPosition();
    cur.insertBlock();
    QTextCharFormat footerFmt;
    footerFmt.setFontItalic(true);
    footerFmt.setFontPointSize(9);
    cur.insertText(QStringLiteral("Generated by GNU Aris"), footerFmt);

    return doc;
}

// ---------------------------------------------------------------------------
// ODT export
// ---------------------------------------------------------------------------

/* Exports the proof to an OpenDocument Text (.odt) file.
 *  Uses QTextDocumentWriter with the "odf" format, which is built into Qt
 *  on all platforms — no extra plugin is required.
 *  input:
 *    name - absolute file path (may have "file://" prefix).
 *    pd   - pointer to the ProofData object.
 */
void auxConnector::exportOdt(const QString &name, const ProofData *pd)
{
    QString path = name.startsWith(QLatin1String("file://")) ? name.mid(7) : name;

    // Ensure the path ends with .odt.
    if (!path.endsWith(QLatin1String(".odt"), Qt::CaseInsensitive))
        path += QStringLiteral(".odt");

    QScopedPointer<QTextDocument> doc(buildProofDocument(pd));

    QTextDocumentWriter writer(path);
    writer.setFormat("odf");   // built-in Qt format, always available

    if (!writer.write(doc.data())) {
        emit errorOccurred(tr("ODT export failed: could not write to '%1'.").arg(path));
        qDebug() << "[ARIS] exportOdt: write failed for" << path;
        return;
    }

    qDebug() << "[ARIS] exportOdt: wrote" << path;
}

/* Exports the proof to an ODT file (WebAssembly).
 *  Writes to a temp file then triggers a browser download.
 */
void auxConnector::wasmExportOdt(const ProofData *pd)
{
    const QString tmp = QStringLiteral("aris_export_tmp.odt");
    exportOdt(tmp, pd);
    QFile file(tmp);
    if (file.open(QIODevice::ReadOnly)) {
        QFileDialog::saveFileContent(file.readAll(), QStringLiteral("proof.odt"));
        file.close();
    }
    QFile::remove(tmp);
}


// ---------------------------------------------------------------------------
// PDF export
// ---------------------------------------------------------------------------

/* Exports the proof to a PDF file.
 *  Uses QPrinter in PdfFormat mode together with QTextDocument::print().
 *  Not available in WASM builds (QPrinter / PrintSupport is absent from
 *  the Qt WASM SDK); on WASM a clear error is emitted instead.
 *  input:
 *    name - absolute file path (may have "file://" prefix).
 *    pd   - pointer to the ProofData object.
 */
void auxConnector::exportPdf(const QString &name, const ProofData *pd)
{
#ifdef Q_OS_WASM
    Q_UNUSED(name)
    Q_UNUSED(pd)
    emit errorOccurred(tr("PDF export is not available on your device."));
#else
    QString path = name.startsWith(QLatin1String("file://")) ? name.mid(7) : name;

    if (!path.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive))
        path += QStringLiteral(".pdf");

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);

    QScopedPointer<QTextDocument> doc(buildProofDocument(pd));
    doc->print(&printer);

    // QPrinter::PdfFormat always succeeds unless the path is unwritable.
    // Verify the file was actually created.
    if (!QFile::exists(path)) {
        emit errorOccurred(tr("PDF export failed: could not write to '%1'.").arg(path));
        qDebug() << "[ARIS] exportPdf: file not found after print for" << path;
        return;
    }

    qDebug() << "[ARIS] exportPdf: wrote" << path;
#endif
}

/* Exports the proof to a PDF file (WebAssembly).
 *  QPrinter is unavailable in WASM; emit a user-facing error.
 *  On desktop this path is never reached (wasmExportPdf is only
 *  called from the WASM branch of the QML format picker).
 */
void auxConnector::wasmExportPdf(const ProofData *pd)
{
#ifdef Q_OS_WASM
    Q_UNUSED(pd)
    emit errorOccurred(tr("PDF export is not available in the browser version."));
#else
    const QString tmp = QStringLiteral("aris_export_tmp.pdf");
    exportPdf(tmp, pd);
    QFile file(tmp);
    if (file.open(QIODevice::ReadOnly)) {
        QFileDialog::saveFileContent(file.readAll(), QStringLiteral("proof.pdf"));
        file.close();
    }
    QFile::remove(tmp);
#endif
}

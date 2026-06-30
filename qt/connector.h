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
#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QVariantList>
#include <QObject>
#include <QHash>
#include "../src/typedef.h"
#include "proofdata.h"
#include "goaldata.h"
#include "proofmodel.h"

class Connector : public QObject
{
    Q_OBJECT
public:
    explicit Connector(QObject *parent = nullptr);

    void reverseMapInit();

    Q_PROPERTY(QString evalText READ evalText WRITE setEvalText NOTIFY evalTextChanged)
    Q_PROPERTY(QString autoSaveStatus READ autoSaveStatus NOTIFY autoSaveStatusChanged)

    QString evalText() const;
    void setEvalText(const QString &newEvalText);

    QString autoSaveStatus() const;
    void setAutoSaveStatus(const QString &status);

    void genIndices(const ProofData * toBeEval);
    void genProof(const ProofData * toBeEval);
    void genGoals(const GoalData * toBeEval);

    Q_INVOKABLE int evalProof(const ProofData *toBeEval, const GoalData *gls, ProofModel *pm);
    Q_INVOKABLE void saveProof(const QString &name,  const ProofData *toBeSaved, const GoalData *gls);
    Q_INVOKABLE void openProof(const QString &name, ProofData *openTo, GoalData *gls);
    Q_INVOKABLE void wasmOpenProof(ProofData *open, GoalData *gls);
    Q_INVOKABLE void wasmSaveProof(const ProofData *pd, const GoalData *gls);
    Q_INVOKABLE void smartPaste(ProofData *pd, ProofModel *pm);
    Q_INVOKABLE void smartCopy(const ProofData *pd, const QVariantList &selectedIndices);

    // IDBFS persistence — WASM only, no-ops on desktop
    Q_INVOKABLE void autoSave(const ProofData *pd, const GoalData *gls);
    Q_INVOKABLE void autoLoad(ProofData *openTo, GoalData *gls);
    Q_INVOKABLE bool isIdbfsReady() const;

    // Persistence warning banner — shown once, then dismissed to localStorage
    Q_INVOKABLE bool shouldShowPersistenceWarning() const;
    Q_INVOKABLE void dismissPersistenceWarning();

    proof_t *getCProof() const;
    vec_t *getReturns() const;

    QHash<QString,int> rulesMap;
    QHash<int,QString> reverseRulesMap;

signals:

    void evalTextChanged();
    void autoSaveStatusChanged();
    void smartPasteStarted();
    void smartPasteDone();


private:
    proof_t * cProof;
    vec_t * returns;
//    QHash<QString,int> rulesMap;
//    QHash<int,QString> reverseRulesMap;
    QString m_evalText;
    QString m_autoSaveStatus;
    QList<QList<int>> m_indices;
};

#endif // CONNECTOR_H

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

#include <QObject>
#include <QHash>
#include "../src/typedef.h"
#include "proofdata.h"
#include "goaldata.h"

class Connector : public QObject
{
    Q_OBJECT
public:
    explicit Connector(QObject *parent = nullptr);

    void reverseMapInit();

    Q_PROPERTY(QString evalText READ evalText WRITE setEvalText NOTIFY evalTextChanged)

    QString evalText() const;
    void setEvalText(const QString &newEvalText);

    void genIndices(const ProofData * toBeEval);
    void genProof(const ProofData * toBeEval);
    void genGoals(const GoalData * toBeEval);

    Q_INVOKABLE int evalProof(const ProofData * toBeEval, const GoalData * gls);
    Q_INVOKABLE void saveProof(const QString &name,  const ProofData *toBeSaved, const GoalData *gls);
    Q_INVOKABLE void openProof(const QString &name, ProofData *openTo, GoalData *gls);
    Q_INVOKABLE void wasmOpenProof(ProofData *open, GoalData *gls);
    Q_INVOKABLE void wasmSaveProof(const ProofData *pd, const GoalData *gls);

    proof_t *getCProof() const;
    vec_t *getReturns() const;

    QHash<QString,int> rulesMap;
    QHash<int,QString> reverseRulesMap;

signals:

    void evalTextChanged();


private:
    proof_t * cProof;
    vec_t * returns;
//    QHash<QString,int> rulesMap;
//    QHash<int,QString> reverseRulesMap;
    QString m_evalText;
    QList<QList<int>> m_indices;
};

#endif // CONNECTOR_H

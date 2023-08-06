#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QObject>
#include <QHash>
#include "typedef.h"
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

signals:

    void evalTextChanged();


private:
    proof_t * cProof;
    vec_t * returns;
    QHash<QString,int> rulesMap;
    QHash<int,QString> reverseRulesMap;
    QString m_evalText;
    QList<QList<int>> m_indices;
};

#endif // CONNECTOR_H

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QObject>
#include <QHash>
#include "typedef.h"
#include "proofdata.h"

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
    Q_INVOKABLE int evalProof(const ProofData * toBeEval);
    Q_INVOKABLE void saveProof(const QString &name,  const ProofData *toBeSaved);
    Q_INVOKABLE void openProof(const QString &name, ProofData *openTo);
    Q_INVOKABLE void wasmOpenProof(ProofData *open);

signals:

    void evalTextChanged();

private:
    proof_t * cProof;
    QHash<QString,int> rulesMap;
    QHash<int,QString> reverseRulesMap;
    QString m_evalText;
    QList<QList<int>> m_indices;
};

#endif // CONNECTOR_H

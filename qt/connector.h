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

    Q_PROPERTY(QString evalText READ evalText WRITE setEvalText NOTIFY evalTextChanged)

    QString evalText() const;
    void setEvalText(const QString &newEvalText);

    void genIndices(const ProofData * toBeEval);
    void genProof(const ProofData * toBeEval);
    Q_INVOKABLE int evalProof(const ProofData * toBeEval);
    Q_INVOKABLE void saveProof(const QString &name,  const ProofData *toBeSaved);

signals:

    void evalTextChanged();

private:
    proof_t * cProof;
    QHash<QString,int> rulesMap;
    QString m_evalText;
    QList<QList<int>> m_indices;
};

#endif // CONNECTOR_H

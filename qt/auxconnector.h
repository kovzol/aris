#ifndef AUXCONNECTOR_H
#define AUXCONNECTOR_H

#include <QObject>
#include "connector.h"
#include "proofmodel.h"

class auxConnector : public QObject
{
    Q_OBJECT
public:
    explicit auxConnector(QObject *parent = nullptr);

    Q_INVOKABLE void latex(const QString &name, const ProofData *toBeEval , Connector *c);
    Q_INVOKABLE void wasmLatex(const ProofData *pd, Connector *c);
    Q_INVOKABLE void importProof(const QString &name, ProofData *pd);

private:

};

#endif // AUXCONNECTOR_H

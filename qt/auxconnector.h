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
    Q_INVOKABLE void importProof(const QString &name, ProofData *pd, Connector *c, ProofModel *pm);
    Q_INVOKABLE void wasmImportProof(ProofData *pd, Connector *c, ProofModel *pm);

#ifndef Q_OS_WASM
    Q_INVOKABLE void newWindow();
#endif

signals:

private:

    QString m_textFieldColor;
};

#endif // AUXCONNECTOR_H

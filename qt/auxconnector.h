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
    Q_INVOKABLE void importProof(const QString &name, ProofData *pd, const Connector *c, ProofModel *pm);
    Q_INVOKABLE void wasmImportProof(ProofData *pd, const Connector *c, ProofModel *pm);

#ifndef Q_OS_WASM
    Q_INVOKABLE void newWindow();
#endif

signals:

private:

    QString m_textFieldColor;
};

#endif // AUXCONNECTOR_H

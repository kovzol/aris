/* Class to store Proof Data (minus goals).

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
#ifndef PROOFDATA_H
#define PROOFDATA_H

#include <QObject>

struct ProofLine{
    int pLine;
    QString pText;
    QString pType;
    bool pSub;
    bool pSubStart;
    bool pSubEnd;
    int pInd;
    QList<int> pRefs;
    unsigned char *fname;
};

class ProofData : public QObject
{
    Q_OBJECT
public:
    explicit ProofData(QObject *parent = nullptr);

    QVector<ProofLine> lines() const;

    bool setLineAt(int index, const ProofLine &proofLine);
    void setFile(int index, const QString& name);

signals:
    void preLineInsert(int index);
    void postLineInsert();

    void preLineRemove(int index);
    void postLineRemove();

public slots:
    void insertLine(int index, int a, QString b, QString c, bool d, bool e, bool f, int g, QList<int> h);
    void removeLineAt(int index);

private:
    QVector<ProofLine> m_proofLines;

};

#endif // PROOFDATA_H

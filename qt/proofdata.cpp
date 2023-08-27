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
#include "proofdata.h"

ProofData::ProofData(QObject *parent)
    : QObject{parent}
{
    m_proofLines.append({1,"","premise",false,false,false,0,{-1},NULL});
}

QVector<ProofLine> ProofData::lines() const
{
    return m_proofLines;
}

// Edit/Change an existing proof line (in m_proofLines) for some valid index
bool ProofData::setLineAt(int index, const ProofLine &proofLine)
{
    if (index < 0 || index > m_proofLines.size())
        return false;

    const ProofLine &oldLine = m_proofLines.at(index);
    if (oldLine.pLine == proofLine.pLine && oldLine.pText == proofLine.pText \
        && oldLine.pSub == proofLine.pSub && oldLine.pSubEnd == proofLine.pSubEnd \
        && oldLine.pSubStart == proofLine.pSubStart && oldLine.pInd == proofLine.pInd \
        && oldLine.pType == proofLine.pType && oldLine.pRefs == proofLine.pRefs)
    {
        return false;
    }

    m_proofLines[index] = proofLine;
    return true;
}

// Set sd->file for a line (required for rule lemma i.e. import proof)
void ProofData::setFile(int index, const QString &name)
{
    m_proofLines[index].fname = (unsigned char *) calloc(name.size()+1, sizeof(unsigned char));
    memcpy(m_proofLines[index].fname, name.toStdString().c_str(), name.size());
}

// Insert a proof line (in m_proofLines) at a valid index
void ProofData::insertLine(int index,int a, QString b, QString c, bool d, bool e, bool f, int g, QList<int> h)
{
    emit preLineInsert(index);

    ProofLine aLine;
    aLine.pLine = a;
    aLine.pText = b;
    aLine.pType = c;
    aLine.pSub = d;
    aLine.pSubStart = e;
    aLine.pSubEnd = f;
    aLine.pInd = g;
    aLine.pRefs = h;
    aLine.fname = NULL;
    m_proofLines.insert(index,aLine);

    emit postLineInsert();
}

// Proof a proof line (in m_proofLines) at a valid index
void ProofData::removeLineAt(int index)
{
    emit preLineRemove(index);

    m_proofLines.removeAt(index);

    emit postLineRemove();
}

#include "proofdata.h"

ProofData::ProofData(QObject *parent)
    : QObject{parent}
{
    m_proofLines.append({1,"","premise",false,false,false,0,{-1}});
}

QVector<ProofLine> ProofData::lines() const
{
    return m_proofLines;
}

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
    m_proofLines.insert(index,aLine);

    emit postLineInsert();
}

void ProofData::removeLineAt(int index)
{
    emit preLineRemove(index);

    m_proofLines.removeAt(index);

    emit postLineRemove();
}

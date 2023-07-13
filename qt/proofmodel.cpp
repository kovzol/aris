#include "proofmodel.h"

ProofModel::ProofModel(QObject *parent)
    : QObject{parent}
{

}

int ProofModel::pmResult() const
{
    return m_pmResult;
}

void ProofModel::setPmResult(int newPmResult)
{
    if (m_pmResult == newPmResult)
        return;
    m_pmResult = newPmResult;
    emit pmResultChanged();
}

QQmlListProperty<ProofLine> ProofModel::pmLines()
{
    return QQmlListProperty<ProofLine>(this,this,&ProofModel::appendLine,&ProofModel::lineCount,&ProofModel::lines,&ProofModel::clearLines);
}

// Helper Definitions

void ProofModel::appendLineCustom(ProofLine * line)
{
    m_pmLines.append(line);
}

long long int ProofModel::lineCountCustom()
{
    return m_pmLines.count();
}

ProofLine *ProofModel::linesCustom(long long int index)
{
    return m_pmLines.at(index);
}

void ProofModel::clearLinesCustom()
{
    m_pmLines.clear();
}


// Callback Definitions

void ProofModel::appendLine(QQmlListProperty<ProofLine> * list, ProofLine * line)
{
    return reinterpret_cast<ProofModel*>(list->data)->appendLineCustom(line);
}

long long int ProofModel::lineCount(QQmlListProperty<ProofLine>* list)
{
    return reinterpret_cast<ProofModel*>(list->data)->lineCountCustom();
}

ProofLine *ProofModel::lines(QQmlListProperty<ProofLine> * list, long long int index)
{
    return reinterpret_cast<ProofModel*>(list->data)->linesCustom(index);
}

void ProofModel::clearLines(QQmlListProperty<ProofLine>* list)
{
    return reinterpret_cast<ProofModel*>(list->data)->clearLinesCustom();
}


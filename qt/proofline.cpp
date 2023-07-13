#include "proofline.h"

ProofLine::ProofLine(QObject *parent)
    : QObject{parent}
{

}

QString ProofLine::pText() const
{
    return m_pText;
}

void ProofLine::setPText(const QString &newPText)
{
    if (m_pText == newPText)
        return;
    m_pText = newPText;
    emit pTextChanged();
}

int ProofLine::pLine() const
{
    return m_pLine;
}

void ProofLine::setPLine(int newPLine)
{
    if (m_pLine == newPLine)
        return;
    m_pLine = newPLine;
    emit pLineChanged();
}

int ProofLine::pDepth() const
{
    return m_pDepth;
}

void ProofLine::setPDepth(int newPDepth)
{
    if (m_pDepth == newPDepth)
        return;
    m_pDepth = newPDepth;
    emit pDepthChanged();
}

QString ProofLine::pType() const
{
    return m_pType;
}

void ProofLine::setPType(const QString &newPType)
{
    if (m_pType == newPType)
        return;
    m_pType = newPType;
    emit pTypeChanged();
}

bool ProofLine::pSub() const
{
    return m_pSub;
}

void ProofLine::setPSub(bool newPSub)
{
    if (m_pSub == newPSub)
        return;
    m_pSub = newPSub;
    emit pSubChanged();
}

bool ProofLine::pSubStart() const
{
    return m_pSubStart;
}

void ProofLine::setPSubStart(bool newPSubStart)
{
    if (m_pSubStart == newPSubStart)
        return;
    m_pSubStart = newPSubStart;
    emit pSubStartChanged();
}

bool ProofLine::pSubEnd() const
{
    return m_pSubEnd;
}

void ProofLine::setPSubEnd(bool newPSubEnd)
{
    if (m_pSubEnd == newPSubEnd)
        return;
    m_pSubEnd = newPSubEnd;
    emit pSubEndChanged();
}

int ProofLine::pIndent() const
{
    return m_pIndent;
}

void ProofLine::setPIndent(int newPIndent)
{
    if (m_pIndent == newPIndent)
        return;
    m_pIndent = newPIndent;
    emit pIndentChanged();
}

QVector<int> ProofLine::pRefs() const
{
    return m_pRefs;
}

void ProofLine::setPRefs(const QVector<int> &newPRefs)
{
    if (m_pRefs == newPRefs)
        return;
    m_pRefs = newPRefs;
    emit pRefsChanged();
}

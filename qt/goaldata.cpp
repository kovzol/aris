#include "goaldata.h"

GoalData::GoalData(QObject *parent)
    : QObject{parent}
{
    m_goalLines.append({-2,""});
}

QVector<GoalLine> GoalData::glines() const
{
    return m_goalLines;
}

bool GoalData::setgLineAt(int index, const GoalLine &goalLine)
{
    if (index < 0 || index > m_goalLines.size())
        return false;

    const GoalLine &oldLine = m_goalLines.at(index);
    if (oldLine.gLine == goalLine.gLine && oldLine.gText == goalLine.gText)
    {
        return false;
    }

    m_goalLines[index] = goalLine;
    return true;
}

void GoalData::insertgLine(int index, int gLine, QString gText)
{
    emit pregLineInsert(index);

    GoalLine aLine;
    aLine.gLine = gLine;
    aLine.gText = gText;

    m_goalLines.insert(index,aLine);

    emit postgLineInsert();
}

void GoalData::removegLineAt(int index)
{
    emit pregLineRemove(index);

    m_goalLines.removeAt(index);

    emit postgLineRemove();
}

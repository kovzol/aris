/* Class to hold Goal Data.

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
#include "goaldata.h"

GoalData::GoalData(QObject *parent)
    : QObject{parent}
{
    m_goalLines.append({-2,false,""});
}

QVector<GoalLine> GoalData::glines() const
{
    return m_goalLines;
}

// Edit/Change an existing goal line (in m_goalLines) for some valid index
bool GoalData::setgLineAt(int index, const GoalLine &goalLine)
{
    if (index < 0 || index > m_goalLines.size())
        return false;

    const GoalLine &oldLine = m_goalLines.at(index);
    if (oldLine.gLine == goalLine.gLine && oldLine.gText == goalLine.gText && oldLine.gValid == goalLine.gValid)
    {
        return false;
    }

    m_goalLines[index] = goalLine;
    return true;
}

// Insert a goal line (in m_goalLines) at a valid index
void GoalData::insertgLine(int index, int gLine, bool gValid, QString gText)
{
    emit pregLineInsert(index);

    GoalLine aLine;
    aLine.gLine = gLine;
    aLine.gValid = gValid;
    aLine.gText = gText;

    m_goalLines.insert(index,aLine);

    emit postgLineInsert();
}

// Remove a goal line (in m_goalLines) at a valid index
void GoalData::removegLineAt(int index)
{
    emit pregLineRemove(index);

    m_goalLines.removeAt(index);

    emit postgLineRemove();
}

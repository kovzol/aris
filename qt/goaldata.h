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
#ifndef GOALDATA_H
#define GOALDATA_H

#include <QObject>

struct GoalLine{
    int gLine;
    bool gValid;
    QString gText;
};

class GoalData : public QObject
{
    Q_OBJECT
public:
    explicit GoalData(QObject *parent = nullptr);

    QVector<GoalLine> glines() const;

    bool setgLineAt(int index, const GoalLine &goalLine);

signals:
    void pregLineInsert(int index);
    void postgLineInsert();

    void pregLineRemove(int index);
    void postgLineRemove();

public slots:
    void insertgLine(int index, int gLine, bool gValid, QString gText);
    void removegLineAt(int index);

private:
    QVector<GoalLine> m_goalLines;

};

#endif // GOALDATA_H

/* Class to model Goal Data to be viewed/interacted with in QML.

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
#ifndef GOALMODEL_H
#define GOALMODEL_H

#include <QAbstractListModel>
#include "goaldata.h"
#include "connector.h"

class GoalModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(GoalData* glines READ glines WRITE setGlines)

public:
    explicit GoalModel(QObject *parent = nullptr);

    enum {
        LineRole = Qt::UserRole,
        TextRole,
        ValidRole
    };

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual QHash<int, QByteArray> roleNames() const override;

    GoalData *glines() const;
    void setGlines(GoalData *newGlines);

    Q_INVOKABLE void evalGoals(GoalData *gls, Connector *c);


private:
    GoalData *m_glines = nullptr;
};

#endif // GOALMODEL_H

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
#include "goalmodel.h"
#include "goals-qt.h"
#include <QApplication>

GoalModel::GoalModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int GoalModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_glines->glines().size();
}

QVariant GoalModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const GoalLine someLine = m_glines->glines().at(index.row());

    switch(role){
        case LineRole:
            return QVariant(someLine.gLine);
        case TextRole:
            return QVariant(someLine.gText);
        case ValidRole:
            return QVariant(someLine.gValid);
    }

    return QVariant();
}

bool GoalModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!m_glines)
            return false;

    GoalLine someLine = m_glines->glines().at(index.row());

    switch (role){
        case LineRole:
            someLine.gLine = value.toInt();
            break;
        case TextRole:
            someLine.gText = value.toString();
            break;
        case ValidRole:
            someLine.gValid = value.toBool();
            break;
    }

    if (m_glines->setgLineAt(index.row(),someLine)) {
            emit dataChanged(index, index, {role});
            return true;
    }
    return false;
}

Qt::ItemFlags GoalModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable; // FIXME: Implement me!
}

QHash<int, QByteArray> GoalModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[LineRole] = "line";
    names[TextRole] = "text";
    names[ValidRole] = "valid";

    return names;
}

GoalData *GoalModel::glines() const
{
    return m_glines;
}

void GoalModel::setGlines(GoalData *newGlines)
{
    beginResetModel();

    if (m_glines)
        m_glines->disconnect(this);

    m_glines = newGlines;

    if (m_glines){
        connect(m_glines,&GoalData::pregLineInsert,this,[=](int index){
            beginInsertRows(QModelIndex(),index,index);
        });
        connect(m_glines,&GoalData::postgLineInsert,this,[=](){
            endInsertRows();
        });
        connect(m_glines,&GoalData::pregLineRemove,this,[=](int index){
            beginRemoveRows(QModelIndex(),index,index);
        });
        connect(m_glines,&GoalData::postgLineRemove,this,[=](){
            endRemoveRows();
        });
    }

    endResetModel();
}

void GoalModel::evalGoals(GoalData *gls, Connector *c)
{
    for (int i = 0; i < gls->glines().size(); i++){
        int ln, is_valid;
        unsigned char *temp_text;

        std::string str = gls->glines().at(i).gText.toStdString();
        temp_text = (unsigned char *) calloc((strlen(str.c_str()))+1, sizeof(unsigned char));
        memcpy(temp_text, str.c_str(), strlen(str.c_str()));


        if (qtgoal_check_line(c->getCProof(),temp_text,c->getReturns(),&ln,&is_valid) == 0){
            setData(index(i,0),ln,256);
            setData(index(i,0),(is_valid == 1),258);
        }
        else
            qDebug() << "Error in checking goal " << i + 1 << ":\n\t Goal was probably empty";
    }
}

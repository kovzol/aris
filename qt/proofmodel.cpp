/* Class to model Proof Data (minus goals) to be viewed/interacted with in QML.

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
#include "proofmodel.h"

ProofModel::ProofModel(QObject *parent)
    : QAbstractListModel(parent), mLines(nullptr)
{
}

int ProofModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid() || !mLines)
        return 0;

    return mLines->lines().size();
}

QVariant ProofModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const ProofLine someLine = mLines->lines().at(index.row());

    switch (role){
    case LineRole:
        return QVariant(someLine.pLine);
    case TextRole:
        return QVariant(someLine.pText);
    case TypeRole:
        return QVariant(someLine.pType);
    case SubRole:
        return QVariant(someLine.pSub);
    case SubStartRole:
        return QVariant(someLine.pSubStart);
    case SubEndRole:
        return QVariant(someLine.pSubEnd);
    case IndentRole:
        return QVariant(someLine.pInd);
    case RefsRole:
        QList<QVariant> ret;
        for (int x: someLine.pRefs)
            ret.append(x);
        return ret;

    }
    return QVariant();
}

bool ProofModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!mLines)
        return false;

    ProofLine someLine = mLines->lines().at(index.row());

    switch (role){
    case LineRole:
        someLine.pLine = value.toInt();
        break;
    case TextRole:
        someLine.pText = value.toString();
        break;
    case TypeRole:
        someLine.pType = value.toString();
        break;
    case SubRole:
        someLine.pSub = value.toBool();
        break;
    case SubStartRole:
        someLine.pSubStart = value.toBool();
        break;
    case SubEndRole:
        someLine.pSubEnd = value.toBool();
        break;
    case IndentRole:
        someLine.pInd = value.toInt();
        break;
    case RefsRole:
        someLine.pRefs.clear();
        QVariantList temp = value.toList();
        for (const QVariant &x: qAsConst(temp))
            someLine.pRefs.append(x.toInt());
        break;

    }

    if (mLines->setLineAt(index.row(),someLine)) {
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

Qt::ItemFlags ProofModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable;
}

QHash<int, QByteArray> ProofModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[LineRole] = "line";
    names[TextRole] = "lText";
    names[TypeRole] = "type";
    names[SubRole] = "sub";
    names[SubStartRole] = "subSt";
    names[SubEndRole] = "subEnd";
    names[IndentRole] = "ind";
    names[RefsRole] = "refs";
    return names;
}

ProofData *ProofModel::lines() const
{
    return mLines;
}

void ProofModel::setlines(ProofData *newLines)
{
    beginResetModel();

    if (mLines)
        mLines->disconnect(this);

    mLines = newLines;

    if (mLines){
        connect(mLines,&ProofData::preLineInsert,this,[=](int index){
            beginInsertRows(QModelIndex(),index,index);
        });
        connect(mLines,&ProofData::postLineInsert,this,[=](){
            endInsertRows();
        });
        connect(mLines,&ProofData::preLineRemove,this,[=](int index){
            beginRemoveRows(QModelIndex(),index,index);
        });
        connect(mLines,&ProofData::postLineRemove,this,[=](){
            endRemoveRows();
        });
    }

    endResetModel();
}

// TODO: Use model indices directly in QML, no need to update lines that way
void ProofModel::updateLines()
{
    for (int i = 0; i < mLines->lines().size(); ++i) {
        setData(index(i,0),i+1,LineRole);
    }
}

void ProofModel::updateRefs(int ln, bool op)
{
    for (int i = ln+1; i < mLines->lines().size(); i++){
        QList<int> refs = mLines->lines().at(i).pRefs;

        for (int ii = 1; ii < refs.size(); ii++){

            if (op){
                if (refs[ii] >= (ln+1))
                    refs[ii]++;

            }
            else{
                if (refs[ii] == (ln + 1))
                    refs.removeAt(ii);
                else if (refs[ii] > (ln + 1))
                    refs[ii]--;

            }
        }
        QList<QVariant> ret;
        for (int x: refs)
            ret.append(x);
        setData(index(i,0),ret,RefsRole);
    }

}

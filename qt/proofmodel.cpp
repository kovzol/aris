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

void ProofModel::updateLines()
{
    for (int i = 0; i < mLines->lines().size(); ++i) {
        setData(index(i,0),i+1,LineRole);
    }
}

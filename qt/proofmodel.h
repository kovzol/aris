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
#ifndef PROOFMODEL_H
#define PROOFMODEL_H

#include <QAbstractListModel>
#include "proofdata.h"

class ProofModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(ProofData* lines READ lines WRITE setlines)


public:
    explicit ProofModel(QObject *parent = nullptr);

    enum {
        LineRole = Qt::UserRole,
        TextRole,
        TypeRole,
        SubRole,
        SubStartRole,
        SubEndRole,
        IndentRole,
        RefsRole
    };

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual QHash<int, QByteArray> roleNames() const override;

    ProofData *lines() const;
    void setlines(ProofData *newLines);
    Q_INVOKABLE void updateLines();
    Q_INVOKABLE void updateRefs(int ln, bool op);

private:
    ProofData *mLines;
};

#endif // PROOFMODEL_H

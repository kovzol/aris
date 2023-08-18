#ifndef PROOFMODEL_H
#define PROOFMODEL_H

#include <QAbstractListModel>
#include "proofdata.h"

//class ProofData;

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

#ifndef GOALMODEL_H
#define GOALMODEL_H

#include <QAbstractListModel>
#include "goaldata.h"
//#include "typedef.h"
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

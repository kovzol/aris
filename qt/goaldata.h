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

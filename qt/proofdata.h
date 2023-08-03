#ifndef PROOFDATA_H
#define PROOFDATA_H

#include <QObject>

struct ProofLine{
    int pLine;
    QString pText;
    QString pType;
    bool pSub;
    bool pSubStart;
    bool pSubEnd;
    int pInd;
    QList<int> pRefs;
};

class ProofData : public QObject
{
    Q_OBJECT
public:
    explicit ProofData(QObject *parent = nullptr);

    QVector<ProofLine> lines() const;

    bool setLineAt(int index, const ProofLine &proofLine);

signals:
    void preLineInsert(int index);
    void postLineInsert();

    void preLineRemove(int index);
    void postLineRemove();

public slots:
    void insertLine(int index, int a, QString b, QString c, bool d, bool e, bool f, int g, QList<int> h);
    void removeLineAt(int index);

private:
    QVector<ProofLine> m_proofLines;

};

#endif // PROOFDATA_H

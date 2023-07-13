#ifndef PROOFLINE_H
#define PROOFLINE_H

#include <QObject>

class ProofLine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString pText READ pText WRITE setPText NOTIFY pTextChanged)
    Q_PROPERTY(int pLine READ pLine WRITE setPLine NOTIFY pLineChanged)
    Q_PROPERTY(int pDepth READ pDepth WRITE setPDepth NOTIFY pDepthChanged)
    Q_PROPERTY(QString pType READ pType WRITE setPType NOTIFY pTypeChanged)
    Q_PROPERTY(bool pSub READ pSub WRITE setPSub NOTIFY pSubChanged)
    Q_PROPERTY(bool pSubStart READ pSubStart WRITE setPSubStart NOTIFY pSubStartChanged)
    Q_PROPERTY(bool pSubEnd READ pSubEnd WRITE setPSubEnd NOTIFY pSubEndChanged)
    Q_PROPERTY(int pIndent READ pIndent WRITE setPIndent NOTIFY pIndentChanged)
    Q_PROPERTY(QVector<int> pRefs READ pRefs WRITE setPRefs NOTIFY pRefsChanged)

public:
    explicit ProofLine(QObject *parent = nullptr);

    QString pText() const;
    void setPText(const QString &newPText);

    int pLine() const;
    void setPLine(int newPLine);

    int pDepth() const;
    void setPDepth(int newPDepth);

    QString pType() const;
    void setPType(const QString &newPType);

    bool pSub() const;
    void setPSub(bool newPSub);

    bool pSubStart() const;
    void setPSubStart(bool newPSubStart);

    bool pSubEnd() const;
    void setPSubEnd(bool newPSubEnd);

    int pIndent() const;
    void setPIndent(int newPIndent);

    QVector<int> pRefs() const;
    void setPRefs(const QVector<int> &newPRefs);

signals:
    void pTextChanged();
    void pLineChanged();
    void pDepthChanged();
    void pTypeChanged();
    void pSubChanged();
    void pSubStartChanged();
    void pSubEndChanged();
    void pIndentChanged();
    void pRefsChanged();

private:
    QString m_pText;
    int m_pLine;
    int m_pDepth;
    QString m_pType;
    bool m_pSub;
    bool m_pSubStart;
    bool m_pSubEnd;
    int m_pIndent;
    QVector<int> m_pRefs;
};

#endif // PROOFLINE_H

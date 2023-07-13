#ifndef PROOFMODEL_H
#define PROOFMODEL_H

#include <QObject>
#include <QQmlListProperty>
#include "proofline.h"

class ProofModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int pmResult READ pmResult WRITE setPmResult NOTIFY pmResultChanged)
    Q_PROPERTY(QQmlListProperty<ProofLine> pmLines READ pmLines NOTIFY pmLinesChanged)
public:
    explicit ProofModel(QObject *parent = nullptr);

    int pmResult() const;
    void setPmResult(int newPmResult);

    QQmlListProperty<ProofLine> pmLines();

    // Helper Methods
    void appendLineCustom(ProofLine*);
    long long int lineCountCustom();
    ProofLine * linesCustom(long long int);
    void clearLinesCustom();

signals:

    void pmResultChanged();
    void pmLinesChanged();

private:
    // CallBack Methods
    static void appendLine(QQmlListProperty<ProofLine>*, ProofLine*);
    static long long int lineCount(QQmlListProperty<ProofLine>*);
    static ProofLine * lines(QQmlListProperty<ProofLine>*, long long int);
    static void clearLines(QQmlListProperty<ProofLine>*);



    int m_pmResult;
//    QQmlListProperty<ProofLine> m_pmLines;
    QVector<ProofLine *> m_pmLines;
};

#endif // PROOFMODEL_H

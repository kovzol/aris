#ifndef WRAPPER_H
#define WRAPPER_H

#include <QObject>

class Wrapper : public QObject
{
    Q_OBJECT


public:
    explicit Wrapper(QObject *parent = nullptr);
    Q_INVOKABLE void displayData();
    Q_INVOKABLE void clearData();
    Q_INVOKABLE void textAppend(const QString &text);
    Q_INVOKABLE void ruleAppend(const QString &rule);
    Q_INVOKABLE void refsAppend(const int i, const int &ref);
    Q_INVOKABLE void depthAppend(const int &indent);
    Q_INVOKABLE void computeIndices();
    Q_INVOKABLE void computeRules();
    Q_INVOKABLE int test_proof_t();
    Q_INVOKABLE int checkProof();

    Q_PROPERTY(QString evalText READ evalText WRITE setEvalText NOTIFY evalTextChanged);
    QString evalText() const;
    void setEvalText(const QString &newEvalText);

signals:
    void evalTextChanged();

private:
    QStringList m_body;
    QStringList m_rules;
    QList<bool> m_premise;
    QList<bool> m_subproof;
    QList<QList<int>> m_indices;
    QList<QList<int>> m_refs;
    QList<int> m_depth;
    QList<int> rules;
    QString m_evalText;
};

#endif // WRAPPER_H

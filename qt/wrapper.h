#ifndef WRAPPER_H
#define WRAPPER_H

#include <QObject>

class Wrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString bodyText READ bodyText WRITE setBodyText NOTIFY bodyTextChanged)
    Q_PROPERTY(bool evalResult READ evalResult WRITE setEvalResult NOTIFY evalResultChanged)
public:
    explicit Wrapper(QObject *parent = nullptr);

    QString bodyText() const;
    void setBodyText(const QString &newBodyText);

    bool evalResult() const;
    void setEvalResult(bool newEvalResult);

signals:

    void bodyTextChanged();
    void evalResultChanged();

private:
    QString m_bodyText;
    QStringList m_body;
    bool m_evalResult;
};

#endif // WRAPPER_H

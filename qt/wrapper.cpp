#include "wrapper.h"
#include <QDebug>

Wrapper::Wrapper(QObject *parent)
    : QObject{parent}
{

}

QString Wrapper::bodyText() const
{
    return m_bodyText;
}

void Wrapper::setBodyText(const QString &newBodyText)
{
//    if (m_bodyText == newBodyText)
//        return;
    m_bodyText = newBodyText;
    emit bodyTextChanged();
    m_body << m_bodyText;

    qDebug() << "the text : " << m_bodyText << " and size of m_body = " << m_body.size();
}

bool Wrapper::evalResult() const
{
    return m_evalResult;
}

void Wrapper::setEvalResult(bool newEvalResult)
{
    if (m_evalResult == newEvalResult)
        return;
    m_evalResult = newEvalResult;
    emit evalResultChanged();
}

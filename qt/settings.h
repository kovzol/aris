#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QObject>
#include <QTranslator>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QLibraryInfo>

class Settings : public QObject {
    Q_OBJECT

public:
    explicit Settings(QGuiApplication *app);

    void setEngine(QQmlEngine *engine);

    Q_INVOKABLE void setLanguage(const QString& languageCode);

private:
    QGuiApplication *m_app;
    QQmlEngine *m_engine;
    QTranslator m_translator;
};

#endif // LANGUAGEMANAGER_H

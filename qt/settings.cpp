#include <QString>
#include "settings.h"


Settings::Settings(QGuiApplication *app)
    : m_app(app)
{
    // Add localization
    QTranslator translator;
    if (translator.load(QLocale::system(), u"aris-qt"_qs,u"_"_qs, u":/i18n"_qs)) {
        app->installTranslator(&translator);
    }
}

void Settings::setEngine(QQmlEngine *engine)
{
    m_engine = engine;
}

void Settings::setLanguage(const QString& languageCode)
{
    m_app->removeTranslator(&m_translator);

    if (m_translator.load(u"aris-qt_"_qs+languageCode, u":/i18n"_qs)) {
        m_app->installTranslator(&m_translator);
        QString layoutDir = m_app->translate("main", "QT_LAYOUT_DIRECTION");
        if (layoutDir == "RTL") {
            m_app->setLayoutDirection(Qt::RightToLeft);
        } else {
            m_app->setLayoutDirection(Qt::LeftToRight);
        }
    }

    m_engine->retranslate();
}

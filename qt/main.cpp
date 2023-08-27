#include <QApplication>
#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QQuickStyle>
#include "proofmodel.h"
#include "proofdata.h"
#include "goaldata.h"
#include "goalmodel.h"
#include "connector.h"
#include "auxconnector.h"


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);

    // preprocessor macros because url/link resolution works differently on qmake vis a vis cmake
#ifndef WITH_CMAKE
    app.setWindowIcon(QIcon(":/assets/icon_simple.svg"));
    const QUrl url(QStringLiteral("qrc:/main.qml"));
#else
    app.setWindowIcon(QIcon(u"arisqt/assets/icon_simple.svg"_qs));
    const QUrl url(u"arisqt/main.qml"_qs);
#endif

    // Create custom objects for interfacing with QML
    ProofData theData;
    GoalData theGoals;
    Connector cConnector;
    auxConnector auxConnector;

    // Register models with QML
    qmlRegisterType<ProofModel>("proof.model",1,0,"ProofModel");
    qmlRegisterUncreatableType<ProofData>("proof.model",1,0,"ProofData","Should not be created inside QML");
    qmlRegisterType<GoalModel>("goal.model",1,0,"GoalModel");
    qmlRegisterUncreatableType<GoalData>("goal.model",1,0,"GoalData","Should not be created inside QML");

    QQmlApplicationEngine engine;

    // Set Application Style
    QQuickStyle::setStyle("Fusion");

    // Provide context to QML engine to use custom objects as properties
    engine.rootContext()->setContextProperty("theData",&theData);
    engine.rootContext()->setContextProperty("theGoals",&theGoals);
    engine.rootContext()->setContextProperty("cConnector",&cConnector);
    engine.rootContext()->setContextProperty("auxConnector",&auxConnector);


//    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}

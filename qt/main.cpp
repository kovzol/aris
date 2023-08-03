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


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/assets/icon_simple.svg"));

    ProofData theData;
    GoalData theGoals;
    Connector cConnector;

    qmlRegisterType<ProofModel>("proof.model",1,0,"ProofModel");
    qmlRegisterUncreatableType<ProofData>("proof.model",1,0,"ProofData","Should not be created inside QML");
    qmlRegisterType<GoalModel>("goal.model",1,0,"GoalModel");
    qmlRegisterUncreatableType<GoalData>("goal.model",1,0,"GoalData","Should not be created inside QML");

    QQmlApplicationEngine engine;
    QQuickStyle::setStyle("Fusion");
    engine.rootContext()->setContextProperty("theData",&theData);
    engine.rootContext()->setContextProperty("theGoals",&theGoals);
    engine.rootContext()->setContextProperty("cConnector",&cConnector);



    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}

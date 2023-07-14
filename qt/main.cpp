#include <QApplication>
#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QQuickStyle>
#include "wrapper.h"
#include "proofmodel.h"
#include "proofdata.h"


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/assets/icon_simple.svg"));

    Wrapper theWrap;
    ProofData theData;

    qmlRegisterType<ProofModel>("proof.model",1,0,"ProofModel");
    qmlRegisterUncreatableType<ProofData>("proof.model",1,0,"ProofData","Should not be created inside QML");

    QQmlApplicationEngine engine;
    QQuickStyle::setStyle("Fusion");
    engine.rootContext()->setContextProperty("Wrapper",&theWrap);
    engine.rootContext()->setContextProperty("theData",&theData);



    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}

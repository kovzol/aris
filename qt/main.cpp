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
#include "settings.h"

#ifdef Q_OS_WASM
#include <emscripten.h>
#endif


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);

    // preprocessor macros because url/link resolution works differently on qmake vis a vis cmake
#ifndef WITH_CMAKE
    QIcon::setThemeSearchPaths({ u":/assets/icons"_qs });
    app.setWindowIcon(QIcon(":/assets/icon_simple.svg"));
    const QUrl url(QStringLiteral("qrc:/main.qml"));
#else
    QIcon::setThemeSearchPaths({ u"arisqt/assets/icons"_qs });
    app.setWindowIcon(QIcon(u"arisqt/assets/icon_simple.svg"_qs));
    const QUrl url(u"arisqt/main.qml"_qs);
#endif

    QIcon::setThemeName("arisqt");

    // Create custom objects for interfacing with QML
    Settings settings(&app);
    ProofData theData;
    GoalData theGoals;
    Connector cConnector;
    auxConnector auxConnector;

    // Register models with QML
    qmlRegisterType<Settings>("settings.model",1,0,"Settings");
    qmlRegisterType<ProofModel>("proof.model",1,0,"ProofModel");
    qmlRegisterUncreatableType<ProofData>("proof.model",1,0,"ProofData","Should not be created inside QML");
    qmlRegisterType<GoalModel>("goal.model",1,0,"GoalModel");
    qmlRegisterUncreatableType<GoalData>("goal.model",1,0,"GoalData","Should not be created inside QML");

    QQmlApplicationEngine engine;
    settings.setEngine(&engine);

    // Set Application Style
    QQuickStyle::setStyle("Fusion");

    // Provide context to QML engine to use custom objects as properties
    engine.rootContext()->setContextProperty("settings",&settings);
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

#ifdef Q_OS_WASM
    // Mount a persistent virtual directory backed by the browser's IndexedDB.
    // On first load: /persistent is empty and autoLoad() will find no file.
    // On subsequent loads: /persistent already contains autosave.tle in IndexedDB.
    EM_ASM({
        // Flag polled by Connector::isIdbfsReady() every 100ms from QML.
        window._aris_idbfs_ready = false;

        try { FS.mkdir('/persistent'); } catch(e) {}

        // Mount the IDBFS backend to /persistent
        FS.mount(IDBFS, {}, '/persistent');

        // Sync FROM IndexedDB INTO the virtual FS (populate = true).
        FS.syncfs(true, function(err) {
            if (err)
                console.error('[ARIS] IDBFS startup sync failed:', err);
            else
                console.log('[ARIS] IDBFS mounted and synced successfully.');
            window._aris_idbfs_ready = true;
        });

        // Best-effort sync on tab close / reload.
        window.addEventListener('beforeunload', function() {
            try { FS.syncfs(false, function(){}); } catch(e) {}
        });
    });
#endif

    return app.exec();
}

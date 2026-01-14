#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuickControls2/QQuickStyle>

#include "models/timelinemodel.h"
#include "models/hotspotmodel.h"
#include "profilercontroller.h"
#include "common/datamodels.h"

int main(int argc, char *argv[]) {
    QQuickStyle::setStyle("Fusion");
    QGuiApplication app(argc, argv);

    qRegisterMetaType<CounterFrame>();
    qRegisterMetaType<CpuSample>();
    qRegisterMetaType<HotspotEntry>();
    qRegisterMetaType<HotspotFrame>();

    TimelineModel timeline;
    HotspotModel hotspots;
    ProfilerController controller(&timeline, &hotspots);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("timelineModel", &timeline);
    engine.rootContext()->setContextProperty("hotspotModel", &hotspots);
    engine.rootContext()->setContextProperty("profilerController", &controller);

    const QUrl url(u"qrc:/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}

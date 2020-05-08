#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QtQuick>
#include <QtLocation>
#include <QtPositioning>

#include "triangulation_component.h"
#include "calculationresult_component.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    qmlRegisterType<TriangulationComponent>("lnu.oles.Triangulation", 1, 0, "Triangulation");
    qmlRegisterType<CalculationResultComponent>("lnu.oles.CalculationResult", 1, 0, "CalculationResult");

    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/main.qml"));

    QObject::connect(&engine,
                     &QQmlApplicationEngine::objectCreated,
                     &app,
                     [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl) {
                             QCoreApplication::exit(-1);
                         }
                    },
                    Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}

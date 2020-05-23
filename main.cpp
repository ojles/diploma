#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtMessageHandler>
#include <QDateTime>
#include <QDebug>

#include "calculationresult_component.h"

QObject* messageBoxTextEdit;

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    Q_UNUSED(context)

    QString dateTime = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    QString logType;
    QString baseFormat = "<code style=\"%4\">%1 [%2] %3</code>\n";
    QString finalMessage;

    switch (type) {
    case QtDebugMsg:
        logType = "DEBUG";
        finalMessage = QString(baseFormat).arg(dateTime, logType, message, "");
        break;
    case QtInfoMsg:
        logType = "INFO";
        finalMessage = QString(baseFormat).arg(dateTime, logType, message, "");
        break;
    case QtWarningMsg:
        logType = "WARNING";
        finalMessage = QString(baseFormat).arg(dateTime, logType, message, "color: orange");
        break;
    case QtCriticalMsg:
        logType = "CRITICAL";
        finalMessage = QString(baseFormat).arg(dateTime, logType, message, "color: red");
        break;
    case QtFatalMsg:
        logType = "FATAL";
        finalMessage = QString(baseFormat).arg(dateTime, logType, message, "color: red");
        break;
    }
    QMetaObject::invokeMethod(messageBoxTextEdit, "append", Q_ARG(QString, finalMessage));
}


int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

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

    messageBoxTextEdit = engine.rootObjects().first()->findChild<QObject*>("messageBoxTextEdit");
    qInstallMessageHandler(myMessageOutput);

    return app.exec();
}

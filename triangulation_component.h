#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <QObject>
#include <QDebug>
#include <QGeoCoordinate>
#include <triangulate.h>

class TriangulationComponent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *polyline READ polyline WRITE setPolyline NOTIFY polylineChanged)
    Q_PROPERTY(QString switches READ switches WRITE setSwitches NOTIFY switchesChanged)

public:
    explicit TriangulationComponent(QObject *parent = nullptr);

    QObject* polyline();

    QString switches();

    QList<QObject*> children();

    void setPolyline(QObject *polyline);

    void setSwitches(QString &switches);

    Q_INVOKABLE void doTriangulate();

    Q_INVOKABLE QGeoCoordinate vertexAt(int index);

    Q_INVOKABLE int verticesCount();

signals:
    void polylineChanged();

    void switchesChanged();

    void triangulationFinished();

private:
    void update();

    QObject *_polyline;
    QString _switches;
    QList<QGeoCoordinate> vertices;
};

#endif // TRIANGULATION_H

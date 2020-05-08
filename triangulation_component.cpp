#include "triangulation_component.h"

TriangulationComponent::TriangulationComponent(QObject *parent) : QObject(parent)
{
}

QObject* TriangulationComponent::polyline()
{
    return _polyline;
}

QString TriangulationComponent::switches()
{
    return _switches;
}

void TriangulationComponent::setPolyline(QObject *polyline)
{
    this->_polyline = polyline;
    emit polylineChanged();
}

void TriangulationComponent::setSwitches(QString &switches)
{
    this->_switches = switches;
    emit switchesChanged();
}

void TriangulationComponent::update()
{
    if (_polyline == nullptr) {
        return;
    }

    QList<QVariant> path = _polyline->property("path").toList();
    char* triangulationSwitches = new char[_switches.length()];
    memcpy(triangulationSwitches, _switches.toStdString().c_str(), _switches.length());

    // remove last point that is the same as the first one
    int pathSize = path.size() - 1;
    if (pathSize < 3) {
        return;
    }

    vector<TriangulatePoint> inputPoints;
    for (int i = 0; i < pathSize; i++) {
        QGeoCoordinate coordinate = path.at(i).value<QGeoCoordinate>();
        TriangulatePoint point;
        point.x = coordinate.longitude();
        point.y = coordinate.latitude();
        inputPoints.push_back(point);
    }

    Triangulate triangulate;
    vector<TriangulatePoint> outputPoints = triangulate.triangulateSimplified(triangulationSwitches, inputPoints);
    delete[] triangulationSwitches;

    this->vertices.clear();
    for (int i = 0; i < outputPoints.size(); i++) {
        TriangulatePoint point = outputPoints.at(i);
        this->vertices.append(QGeoCoordinate(point.y, point.x));
    }

    emit TriangulationComponent::triangulationFinished();
}

void TriangulationComponent::doTriangulate()
{
    if (this->_polyline == nullptr)
    {
        return;
    }
    update();
}

QGeoCoordinate TriangulationComponent::vertexAt(int index)
{
    return this->vertices.at(index);
}

int TriangulationComponent::verticesCount()
{
    return this->vertices.size();
}

#include "calculationresult_component.h"
#include "triangle_gradient_renderer.h"

#ifdef INTCALC_DEBUG
#include <iostream>
using std::cout;
using std::endl;
using std::cerr;
#endif


CalculationResultComponent::CalculationResultComponent()
    : vertices(new QVector<QVector2D>()),
      colors(new QVector<QVector4D>()),
      indices(new QVector<unsigned int>()),
      dataChanged(new bool(false)),
      _showTriangulation(new bool(false)),
      _regionOfStudy(nullptr),
      _gamma2(nullptr) {
}

CalculationResultComponent::~CalculationResultComponent() {
    delete vertices;
    delete colors;
}

QQuickFramebufferObject::Renderer* CalculationResultComponent::createRenderer() const {
    return new TriangleGradientRenderer(vertices, colors, indices, _showTriangulation, dataChanged);
}

QString CalculationResultComponent::triangulationSwitches() {
    return _triangulationSwitches;
}

bool CalculationResultComponent::showTriangulation() {
    return *_showTriangulation;
}

QObject* CalculationResultComponent::regionOfStudy() {
    return _regionOfStudy;
}

QObject* CalculationResultComponent::gamma2() {
    return _gamma2;
}

QJSValue CalculationResultComponent::conservacyAreas() {
    return _conservacyAreas;
}

bool CalculationResultComponent::calculateGamma2() {
    return _calculateGamma2;
}

double CalculationResultComponent::mu() {
    return _mu;
}

double CalculationResultComponent::sigma() {
    return _sigma;
}

double CalculationResultComponent::alpha() {
    return _alpha;
}

bool CalculationResultComponent::hasData() {
    return _hasData;
}

void CalculationResultComponent::setTriangulationSwitches(QString triangulationSwitches) {
    _triangulationSwitches = triangulationSwitches;
    emit triangulationSwitchesChanged();
}

void CalculationResultComponent::setShowTriangulation(bool showTriangulation) {
    *_showTriangulation = showTriangulation;
    emit showTriangulationChanged();
}

void CalculationResultComponent::setRegionOfStudy(QObject* regionOfStudy) {
    _regionOfStudy = regionOfStudy;
    emit regionOfStudyChanged();
}

void CalculationResultComponent::setGamma2(QObject* gamma2) {
    _gamma2 = gamma2;
    emit gamma2Changed();
}

void CalculationResultComponent::setConservacyAreas(QJSValue conservacyAreas) {
    _conservacyAreas = conservacyAreas;
    emit conservacyAreasChanged();
}

void CalculationResultComponent::setCalculateGamma2(bool calculateGamma2) {
    _calculateGamma2 = calculateGamma2;
    emit calculateGamma2Changed();
}

void CalculationResultComponent::setMu(double mu) {
    _mu = mu;
    emit muChanged();
}

void CalculationResultComponent::setSigma(double sigma) {
    _sigma = sigma;
    emit sigmaChanged();
}

void CalculationResultComponent::setAlpha(double alpha) {
    _alpha = alpha;
    emit alphaChanged();
}

QVector<intcalc::Vector2d>* retrievePointsFromMapPolyline(QVariant path, int minPoints) {
    QList<QVariant> pathElements = path.toList();
    if (pathElements.size() < minPoints) {
        throw "Invalid path size";
    }

    QVector<intcalc::Vector2d>* points = new QVector<intcalc::Vector2d>();
    for (auto pathElement : pathElements) {
        QGeoCoordinate coordinate = pathElement.value<QGeoCoordinate>();
        points->push_back(intcalc::Vector2d(coordinate.longitude(), coordinate.latitude()));
    }
    return points;
}

QVariant CalculationResultComponent::doCalculate() {
    QVector<intcalc::Vector2d>* inputPoints = retrievePointsFromMapPolyline(_regionOfStudy->property("path"), 3);
    QVector<intcalc::Vector2d>* gamma2 = retrievePointsFromMapPolyline(_gamma2->property("path"), 2);
    QVector<QVector<intcalc::Vector2d>*> conservacyAreas;
    int areaCount = _conservacyAreas.property("length").toInt() - 1;
    for (int i = 0; i < areaCount; i++) {
        QVariant path = _conservacyAreas.property(i).property("path").toVariant();
        conservacyAreas.push_back(retrievePointsFromMapPolyline(path, 3));
    }

    intcalc::FEMCalculator femCalculator;
    femCalculator.setRegionOfStudy(inputPoints);
    if (_calculateGamma2) {
        femCalculator.setGamma2(gamma2);
    }
    femCalculator.setConservacyAreas(conservacyAreas);
    femCalculator.setTriangulationSwitches(_triangulationSwitches);
    femCalculator.setMu(_mu);
    femCalculator.setSigma(_sigma);
    femCalculator.setAlpha(_alpha);
    femCalculator.setBeta([](const intcalc::Vector2d& vertex) -> intcalc::Vector2d {
        Q_UNUSED(vertex);
        intcalc::Vector2d result;
        result.x = -10;
        result.y = 0;
        return result;
    });

     QList<QGeoCoordinate> resCoords;
    try {
        intcalc::CalcSolution solution = femCalculator.solve();

        intcalc::Point2DValue minPoint;
        for (int i = 0; i < solution.vertices.size(); i++) {
            if (!solution.vertices[i].isOnContour) {
                minPoint.value = solution.vertices[i].value;
                break;
            }
        }
        for (const intcalc::Point2DValue& vertex : solution.vertices) {
            if (vertex.value < minPoint.value && !vertex.isOnContour) {
                minPoint.value = vertex.value;
            }
        }
        for (const intcalc::Point2DValue& vertex : solution.vertices) {
            if (minPoint.value == vertex.value && !vertex.isOnContour) {
                double longitude = vertex.x;
                double latitude = vertex.y;
                resCoords.push_back(QGeoCoordinate(latitude, longitude));
            }
        }

        acceptFEMSolution(solution);

        qInfo() << "Min points: " << resCoords.size();
    } catch(const char* e) {
        qCritical() << e;
        return QVariant::fromValue(nullptr);
    }

    // clean resources
    delete inputPoints;
    delete gamma2;
    for (auto area : conservacyAreas) {
        delete area;
    }

    return QVariant::fromValue(resCoords);
}

void CalculationResultComponent::clear() {
    vertices->clear();
    colors->clear();
    indices->clear();
    *dataChanged = true;
    _hasData = false;
    emit hasDataChanged();
}

void CalculationResultComponent::acceptFEMSolution(intcalc::CalcSolution& solution) {
    if (solution.vertices.size() <= 0) {
        return;
    }

    // TODO: instead of goint through all triangulation points
    // it would be better to go though ROS points
    intcalc::Point2DValue bottomLeft = solution.vertices[0];
    intcalc::Point2DValue topRight = solution.vertices[0];
    for (long i = 0; i < solution.vertices.size(); i++) {
        if (bottomLeft.x > solution.vertices[i].x) {
            bottomLeft.x = solution.vertices[i].x;
        }
        if (bottomLeft.y > solution.vertices[i].y) {
            bottomLeft.y = solution.vertices[i].y;
        }
        if (topRight.x < solution.vertices[i].x) {
            topRight.x = solution.vertices[i].x;
        }
        if (topRight.y < solution.vertices[i].y) {
            topRight.y = solution.vertices[i].y;
        }
        if (bottomLeft.value > solution.vertices[i].value) {
            bottomLeft.value = solution.vertices[i].value;
        }
        if (topRight.value < solution.vertices[i].value) {
            topRight.value = solution.vertices[i].value;
        }
    }

    qInfo() << "---Min value: " << bottomLeft.value;
    qInfo() << "---Max value: " << topRight.value;

    double xScaler = 2 / (topRight.x - bottomLeft.x);
    double yScaler = 2 / (topRight.y - bottomLeft.y);
    double colorScaler = 1 / (topRight.value - bottomLeft.value);

    vertices->clear();
    colors->clear();
    indices->clear();
    for (auto vertex : solution.vertices) {
        float x = static_cast<float>((vertex.x - bottomLeft.x) * xScaler - 1);
        float y = static_cast<float>((vertex.y - bottomLeft.y) * yScaler - 1);
        float color = static_cast<float>((vertex.value - bottomLeft.value) * colorScaler);
        *vertices << QVector2D(x, y);
        *colors << QVector4D(color, color, color, 0.8f);
    }
    for (int i = 0; i < solution.triangleIndices.size(); i++) {
        *indices << solution.triangleIndices[i];
    }
    *dataChanged = true;
    _hasData = true;
    emit hasDataChanged();
}

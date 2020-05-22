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
      _boundaryCondition(nullptr) {
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

QObject* CalculationResultComponent::boundaryCondition() {
    return _boundaryCondition;
}

bool CalculationResultComponent::calculateBoundaryCondition() {
    return _calculateBoundaryContition;
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

void CalculationResultComponent::setBoundaryCondition(QObject* boundaryCondition) {
    _boundaryCondition = boundaryCondition;
    emit boundaryConditionChanged();
}

void CalculationResultComponent::setCalculateBoundaryCondition(bool calculateBoundaryCondition) {
    _calculateBoundaryContition = calculateBoundaryCondition;
    emit calculateBoundaryConditionChanged();
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

QVector<intcalc::Vector2d> retrieveROSPoints(QObject* ros) {
    if (ros == nullptr) {
        return QVector<intcalc::Vector2d>();
    }

    QList<QVariant> path = ros->property("path").toList();

    // remove last point that is the same as the first one
    int pathSize = path.size() - 1;
    if (pathSize < 3) {
#ifdef INTCALC_DEBUG
        cerr << "For some reason pathSize in retrieveROSPoints() is less then 3!" << endl;
#endif
        return QVector<intcalc::Vector2d>();
    }

    QVector<intcalc::Vector2d> points;
    for (int i = 0; i < pathSize; i++) {
        QGeoCoordinate coordinate = path.at(i).value<QGeoCoordinate>();
        intcalc::Vector2d point;
        point.x = coordinate.longitude();
        point.y = coordinate.latitude();
        points.push_back(point);
    }

    return points;
}

QVector<intcalc::Vector2d> retrieveBoundaryCondition(QObject* boundaryCondition) {
    if (boundaryCondition == nullptr) {
        return QVector<intcalc::Vector2d>();
    }

    QList<QVariant> path = boundaryCondition->property("path").toList();

    int pathSize = path.size();
    if (pathSize < 2) {
#ifdef INTCALC_DEBUG
        cerr << "For some reason pathSize in retrieveBoundaryCondition() is less then 2!" << endl;
#endif
        return QVector<intcalc::Vector2d>();
    }

    QVector<intcalc::Vector2d> points;
    for (int i = 0; i < pathSize; i++) {
        QGeoCoordinate coordinate = path.at(i).value<QGeoCoordinate>();
        intcalc::Vector2d point;
        point.x = coordinate.longitude();
        point.y = coordinate.latitude();
        points.push_back(point);
    }

    return points;
}

void CalculationResultComponent::acceptFEMSolution(intcalc::CalcSolution& solution) {
    if (solution.vertices.size() <= 0) {
        return;
    }

    // TODO: instead of goint through all triangulation points
    // it would be better to go though ROS points
    intcalc::Point2DValue bottomLeft = solution.vertices[0];
    intcalc::Point2DValue topRight = solution.vertices[0];
    for (unsigned long i = 0; i < solution.vertices.size(); i++) {
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

    double xScaler = 2 / (topRight.x - bottomLeft.x);
    double yScaler = 2 / (topRight.y - bottomLeft.y);
    double colorScaler = 1 / (topRight.value - bottomLeft.value);

    vertices->clear();
    colors->clear();
    indices->clear();
    for (unsigned long i = 0; i < solution.vertices.size(); i++) {
        float x = static_cast<float>((solution.vertices[i].x - bottomLeft.x) * xScaler - 1);
        float y = static_cast<float>((solution.vertices[i].y - bottomLeft.y) * yScaler - 1);
        float color = static_cast<float>((solution.vertices[i].value - bottomLeft.value) * colorScaler);
        *vertices << QVector2D(x, y);
        *colors << QVector4D(color, color, color, 0.8f);
    }
    for (unsigned int i = 0; i < solution.triangleIndices.size(); i++) {
        *indices << solution.triangleIndices[i];
    }
    *dataChanged = true;
    _hasData = true;
    emit hasDataChanged();
}

void CalculationResultComponent::doCalculate() {
    QVector<intcalc::Vector2d> inputPoints = retrieveROSPoints(_regionOfStudy);
    QVector<intcalc::Vector2d> boundaryCondition = retrieveBoundaryCondition(_boundaryCondition);

    if (inputPoints.size() == 0) {
        return;
    }

    // TODO: check if boundaryCondition is null then don't calculate it
    // or maybe just add a checkbox if to account boundary condition

    intcalc::FEMCalculator femCalculator;
    femCalculator.setRegionOfStudy(&inputPoints);
    if (_calculateBoundaryContition) {
        femCalculator.setGamma2(&boundaryCondition);
    }
    femCalculator.setTriangulationSwitches(_triangulationSwitches);
    femCalculator.setMu(_mu);
    femCalculator.setSigma(_sigma);
    femCalculator.setAlpha(_alpha);
    femCalculator.setBeta([](const intcalc::Vector2d& vertex) -> intcalc::Vector2d {
        intcalc::Vector2d result;
        result.x = 10;
        result.y = 0;
        return result;
    });
    femCalculator.setF([](const intcalc::VertexInfo& vertex) -> double {
        return 0.5;
    });

    intcalc::CalcSolution solution = femCalculator.solve();
    acceptFEMSolution(solution);
}

void CalculationResultComponent::clear() {
    vertices->clear();
    colors->clear();
    indices->clear();
    *dataChanged = true;
    _hasData = false;
    emit hasDataChanged();
}

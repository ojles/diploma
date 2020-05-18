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
      _regionOfStudy(nullptr),
      _boundaryCondition(nullptr)
{
}

CalculationResultComponent::~CalculationResultComponent()
{
    delete vertices;
    delete colors;
}

QQuickFramebufferObject::Renderer* CalculationResultComponent::createRenderer() const
{
    return new TriangleGradientRenderer(vertices, colors);
}

QString CalculationResultComponent::triangulationSwitches()
{
    return _triangulationSwitches;
}

QObject* CalculationResultComponent::regionOfStudy()
{
    return _regionOfStudy;
}

QObject* CalculationResultComponent::boundaryCondition()
{
    return _boundaryCondition;
}

bool CalculationResultComponent::calculateBoundaryCondition()
{
    return _calculateBoundaryContition;
}

double CalculationResultComponent::mu()
{
    return _mu;
}

double CalculationResultComponent::sigma()
{
    return _sigma;
}

double CalculationResultComponent::alpha()
{
    return _alpha;
}

void CalculationResultComponent::setTriangulationSwitches(QString triangulationSwitches)
{
    _triangulationSwitches = triangulationSwitches;
    emit triangulationSwitchesChanged();
}

void CalculationResultComponent::setRegionOfStudy(QObject* regionOfStudy)
{
    _regionOfStudy = regionOfStudy;
    emit regionOfStudyChanged();
}

void CalculationResultComponent::setBoundaryCondition(QObject* boundaryCondition)
{
    _boundaryCondition = boundaryCondition;
    emit boundaryConditionChanged();
}

void CalculationResultComponent::setCalculateBoundaryCondition(bool calculateBoundaryCondition)
{
    _calculateBoundaryContition = calculateBoundaryCondition;
    emit calculateBoundaryConditionChanged();
}

void CalculationResultComponent::setMu(double mu)
{
    _mu = mu;
    emit muChanged();
}

void CalculationResultComponent::setSigma(double sigma)
{
    _sigma = sigma;
    emit sigmaChanged();
}

void CalculationResultComponent::setAlpha(double alpha)
{
    _alpha = alpha;
    emit alphaChanged();
}

vector<intcalc::Vector2d> retrieveROSPoints(QObject* ros)
{
    if (ros == nullptr) {
        return vector<intcalc::Vector2d>();
    }

    QList<QVariant> path = ros->property("path").toList();

    // remove last point that is the same as the first one
    int pathSize = path.size() - 1;
    if (pathSize < 3) {
#ifdef INTCALC_DEBUG
        cerr << "For some reason pathSize in retrieveROSPoints() is less then 3!" << endl;
#endif
        return vector<intcalc::Vector2d>();
    }

    vector<intcalc::Vector2d> points;
    for (int i = 0; i < pathSize; i++) {
        QGeoCoordinate coordinate = path.at(i).value<QGeoCoordinate>();
        intcalc::Vector2d point;
        point.x = coordinate.longitude();
        point.y = coordinate.latitude();
        points.push_back(point);
    }

    return points;
}

vector<intcalc::Vector2d> retrieveBoundaryCondition(QObject* boundaryCondition)
{
    if (boundaryCondition == nullptr) {
        return vector<intcalc::Vector2d>();
    }

    QList<QVariant> path = boundaryCondition->property("path").toList();

    int pathSize = path.size();
    if (pathSize < 2) {
#ifdef INTCALC_DEBUG
        cerr << "For some reason pathSize in retrieveBoundaryCondition() is less then 2!" << endl;
#endif
        return vector<intcalc::Vector2d>();
    }

    vector<intcalc::Vector2d> points;
    for (int i = 0; i < pathSize; i++) {
        QGeoCoordinate coordinate = path.at(i).value<QGeoCoordinate>();
        intcalc::Vector2d point;
        point.x = coordinate.longitude();
        point.y = coordinate.latitude();
        points.push_back(point);
    }

    return points;
}

void CalculationResultComponent::acceptFEMSolution(intcalc::CalcSolution& solution)
{
    if (solution.triangles.size() <= 0) {
        return;
    }

    intcalc::Point2DValue bottomLeft = solution.triangles[0];
    intcalc::Point2DValue topRight = solution.triangles[0];
    for (unsigned long i = 0; i < solution.triangles.size(); i++) {
        if (bottomLeft.x > solution.triangles[i].x) {
            bottomLeft.x = solution.triangles[i].x;
        }
        if (bottomLeft.y > solution.triangles[i].y) {
            bottomLeft.y = solution.triangles[i].y;
        }
        if (topRight.x < solution.triangles[i].x) {
            topRight.x = solution.triangles[i].x;
        }
        if (topRight.y < solution.triangles[i].y) {
            topRight.y = solution.triangles[i].y;
        }
        if (bottomLeft.value > solution.triangles[i].value) {
            bottomLeft.value = solution.triangles[i].value;
        }
        if (topRight.value < solution.triangles[i].value) {
            topRight.value = solution.triangles[i].value;
        }
    }

    double xScaler = 2 / (topRight.x - bottomLeft.x);
    double yScaler = 2 / (topRight.y - bottomLeft.y);
    double colorScaler = 1 / (topRight.value - bottomLeft.value);

    vertices->clear();
    colors->clear();
    for (unsigned long i = 0; i < solution.triangles.size(); i++) {
        float x = static_cast<float>((solution.triangles[i].x - bottomLeft.x) * xScaler - 1);
        float y = static_cast<float>((solution.triangles[i].y - bottomLeft.y) * yScaler - 1);
        float color = static_cast<float>((solution.triangles[i].value - bottomLeft.value) * colorScaler);
        *vertices << QVector2D(x, y);
        *colors << QVector4D(color, color, color, 1.0f);
    }
}

void CalculationResultComponent::doCalculate()
{
    vector<intcalc::Vector2d> inputPoints = retrieveROSPoints(_regionOfStudy);
    vector<intcalc::Vector2d> boundaryCondition = retrieveBoundaryCondition(_boundaryCondition);

    if (inputPoints.size() == 0) {
        return;
    }

    // TODO: check if boundaryCondition is null then don't calculate it
    // or maybe just add a checkbox if to account boundary condition

    intcalc::FEMCalculator femCalculator;
    femCalculator.setPoints(&inputPoints);
    if (_calculateBoundaryContition)
    {
        femCalculator.setBoundaryCondition(&boundaryCondition);
    }
    femCalculator.setTriangulationSwitches(_triangulationSwitches);
    femCalculator.setMu(_mu);
    femCalculator.setSigma(_sigma);
    femCalculator.setAlpha(_alpha);
    femCalculator.setBeta([](double x, double y) -> intcalc::Vector2d {
        Q_UNUSED(x)
        Q_UNUSED(y)
        intcalc::Vector2d result;
        result.x = 15;
        result.y = 0;
        return result;
    });
    femCalculator.setF([](double x, double y) -> double {
        Q_UNUSED(x)
        Q_UNUSED(y)
        return 0.5;
    });

    intcalc::CalcSolution solution = femCalculator.solve();
    acceptFEMSolution(solution);
}

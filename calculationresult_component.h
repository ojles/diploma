#ifndef CALCULATIONRESULTCOMPONENT_H
#define CALCULATIONRESULTCOMPONENT_H

#include <QQuickFramebufferObject>
#include <QGeoCoordinate>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>

#include "integral_calculation.h"

class CalculationResultComponent : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(QString triangulationSwitches READ triangulationSwitches WRITE setTriangulationSwitches NOTIFY triangulationSwitchesChanged)
    Q_PROPERTY(QObject* regionOfStudy READ regionOfStudy WRITE setRegionOfStudy NOTIFY regionOfStudyChanged)
    Q_PROPERTY(QObject* boundaryCondition READ boundaryCondition WRITE setBoundaryCondition NOTIFY boundaryConditionChanged)
    Q_PROPERTY(bool calculateBoundaryCondition READ calculateBoundaryCondition WRITE setCalculateBoundaryCondition NOTIFY calculateBoundaryConditionChanged)
    Q_PROPERTY(double mu READ mu WRITE setMu NOTIFY muChanged)
    Q_PROPERTY(double sigma READ sigma WRITE setSigma NOTIFY sigmaChanged)
    Q_PROPERTY(double alpha READ alpha WRITE setAlpha NOTIFY alphaChanged)

public:
    CalculationResultComponent();
    ~CalculationResultComponent();

    Renderer *createRenderer() const override;

    QString triangulationSwitches();
    QObject* regionOfStudy();
    QObject* boundaryCondition();
    bool calculateBoundaryCondition();
    double mu();
    double sigma();
    double alpha();

    void setTriangulationSwitches(QString triangulationSwitches);
    void setRegionOfStudy(QObject* regionOfStudy);
    void setBoundaryCondition(QObject* boundaryCondition);
    void setCalculateBoundaryCondition(bool calculateBoundaryCondition);
    void setMu(double mu);
    void setSigma(double sigma);
    void setAlpha(double alpha);

    Q_INVOKABLE void doCalculate();

signals:
    void triangulationSwitchesChanged();
    void regionOfStudyChanged();
    void boundaryConditionChanged();
    void calculateBoundaryConditionChanged();
    void muChanged();
    void sigmaChanged();
    void alphaChanged();

private:
    void acceptFEMSolution(intcalc::CalcSolution& solution);

    QVector<QVector2D>* vertices;
    QVector<QVector4D>* colors;
    QObject* _regionOfStudy;
    QObject* _boundaryCondition;
    QString _triangulationSwitches;
    bool _calculateBoundaryContition;
    double _mu;
    double _sigma;
    double _alpha;
};

#endif // CALCULATIONRESULTCOMPONENT_H

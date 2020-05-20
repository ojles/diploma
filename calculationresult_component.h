#ifndef CALCULATIONRESULTCOMPONENT_H
#define CALCULATIONRESULTCOMPONENT_H

#include <QQuickFramebufferObject>
#include <QGeoCoordinate>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>

#include "integral_calculation.h"

class CalculationResultComponent : public QQuickFramebufferObject {
    Q_OBJECT
    Q_PROPERTY(QString triangulationSwitches READ triangulationSwitches WRITE setTriangulationSwitches NOTIFY triangulationSwitchesChanged)
    Q_PROPERTY(bool showTriangulation READ showTriangulation WRITE setShowTriangulation NOTIFY showTriangulationChanged)
    Q_PROPERTY(QObject* regionOfStudy READ regionOfStudy WRITE setRegionOfStudy NOTIFY regionOfStudyChanged)
    Q_PROPERTY(QObject* boundaryCondition READ boundaryCondition WRITE setBoundaryCondition NOTIFY boundaryConditionChanged)
    Q_PROPERTY(bool calculateBoundaryCondition READ calculateBoundaryCondition WRITE setCalculateBoundaryCondition NOTIFY calculateBoundaryConditionChanged)
    Q_PROPERTY(double mu READ mu WRITE setMu NOTIFY muChanged)
    Q_PROPERTY(double sigma READ sigma WRITE setSigma NOTIFY sigmaChanged)
    Q_PROPERTY(double alpha READ alpha WRITE setAlpha NOTIFY alphaChanged)
    Q_PROPERTY(bool hasData READ hasData NOTIFY hasDataChanged)

public:
    CalculationResultComponent();
    ~CalculationResultComponent();

    Renderer *createRenderer() const override;

    QString triangulationSwitches();
    bool showTriangulation();
    QObject* regionOfStudy();
    QObject* boundaryCondition();
    bool calculateBoundaryCondition();
    double mu();
    double sigma();
    double alpha();
    bool hasData();

    void setTriangulationSwitches(QString triangulationSwitches);
    void setShowTriangulation(bool showTriangulation);
    void setRegionOfStudy(QObject* regionOfStudy);
    void setBoundaryCondition(QObject* boundaryCondition);
    void setCalculateBoundaryCondition(bool calculateBoundaryCondition);
    void setMu(double mu);
    void setSigma(double sigma);
    void setAlpha(double alpha);

    Q_INVOKABLE void doCalculate();

    Q_INVOKABLE void clear();

signals:
    void triangulationSwitchesChanged();
    void showTriangulationChanged();
    void regionOfStudyChanged();
    void boundaryConditionChanged();
    void calculateBoundaryConditionChanged();
    void muChanged();
    void sigmaChanged();
    void alphaChanged();
    void hasDataChanged();

private:
    void acceptFEMSolution(intcalc::CalcSolution& solution);

    QVector<QVector2D>* vertices;
    QVector<QVector4D>* colors;
    QVector<unsigned int>* indices;
    bool* dataChanged;
    bool* _showTriangulation;

    QString _triangulationSwitches;
    QObject* _regionOfStudy;
    QObject* _boundaryCondition;
    bool _calculateBoundaryContition;
    double _mu;
    double _sigma;
    double _alpha;
    bool _hasData;
};

#endif // CALCULATIONRESULTCOMPONENT_H

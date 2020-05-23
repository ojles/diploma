#ifndef CALCULATIONRESULTCOMPONENT_H
#define CALCULATIONRESULTCOMPONENT_H

#include <QQuickFramebufferObject>
#include <QGeoCoordinate>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include <QDebug>

#include "integral_calculation.h"

class CalculationResultComponent : public QQuickFramebufferObject {
    Q_OBJECT
    Q_PROPERTY(QString triangulationSwitches READ triangulationSwitches WRITE setTriangulationSwitches NOTIFY triangulationSwitchesChanged)
    Q_PROPERTY(bool showTriangulation READ showTriangulation WRITE setShowTriangulation NOTIFY showTriangulationChanged)
    Q_PROPERTY(QObject* regionOfStudy READ regionOfStudy WRITE setRegionOfStudy NOTIFY regionOfStudyChanged)
    Q_PROPERTY(QObject* gamma2 READ gamma2 WRITE setGamma2 NOTIFY gamma2Changed)
    Q_PROPERTY(QJSValue conservacyAreas READ conservacyAreas WRITE setConservacyAreas NOTIFY conservacyAreasChanged)
    Q_PROPERTY(bool calculateGamma2 READ calculateGamma2 WRITE setCalculateGamma2 NOTIFY calculateGamma2Changed)
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
    QObject* gamma2();
    QJSValue conservacyAreas();
    bool calculateGamma2();
    double mu();
    double sigma();
    double alpha();
    bool hasData();

    void setTriangulationSwitches(QString triangulationSwitches);
    void setShowTriangulation(bool showTriangulation);
    void setRegionOfStudy(QObject* regionOfStudy);
    void setGamma2(QObject* gamma2);
    void setConservacyAreas(QJSValue conservacyAreas);
    void setCalculateGamma2(bool calculateGamma2);
    void setMu(double mu);
    void setSigma(double sigma);
    void setAlpha(double alpha);

    Q_INVOKABLE void doCalculate();
    Q_INVOKABLE void clear();

signals:
    void triangulationSwitchesChanged();
    void showTriangulationChanged();
    void regionOfStudyChanged();
    void gamma2Changed();
    void conservacyAreasChanged();
    void calculateGamma2Changed();
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
    QObject* _gamma2;
    QJSValue _conservacyAreas;
    bool _calculateGamma2;
    double _mu;
    double _sigma;
    double _alpha;
    bool _hasData;
};

#endif // CALCULATIONRESULTCOMPONENT_H

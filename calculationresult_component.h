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
    Q_PROPERTY(double windX READ windX WRITE setWindX NOTIFY windXChanged)
    Q_PROPERTY(double windY READ windY WRITE setWindY NOTIFY windYChanged)
    Q_PROPERTY(double mu READ mu WRITE setMu NOTIFY muChanged)
    Q_PROPERTY(double sigma READ sigma WRITE setSigma NOTIFY sigmaChanged)
    Q_PROPERTY(double alpha READ alpha WRITE setAlpha NOTIFY alphaChanged)
    Q_PROPERTY(bool hasData READ hasData NOTIFY hasDataChanged)

public:
    CalculationResultComponent()
        : vertices(new QVector<QVector2D>()),
          colors(new QVector<QVector4D>()),
          indices(new QVector<unsigned int>()),
          dataChanged(new bool(false)),
          _showTriangulation(new bool(false)),
          _regionOfStudy(nullptr),
          _gamma2(nullptr) {
    }

    ~CalculationResultComponent() {
        delete vertices;
        delete colors;
    }

    Renderer *createRenderer() const override;

    QString triangulationSwitches() {
        return _triangulationSwitches;
    }

    bool showTriangulation() {
        return *_showTriangulation;
    }

    QObject* regionOfStudy() {
        return _regionOfStudy;
    }

    QObject* gamma2() {
        return _gamma2;
    }

    QJSValue conservacyAreas() {
        return _conservacyAreas;
    }

    bool calculateGamma2() {
        return _calculateGamma2;
    }

    double windX() {
        return _windX;
    }

    double windY() {
        return _windY;
    }

    double mu() {
        return _mu;
    }

    double sigma() {
        return _sigma;
    }

    double alpha() {
        return _alpha;
    }

    bool hasData() {
        return _hasData;
    }

    void setTriangulationSwitches(QString triangulationSwitches) {
        _triangulationSwitches = triangulationSwitches;
        emit triangulationSwitchesChanged();
    }

    void setShowTriangulation(bool showTriangulation) {
        *_showTriangulation = showTriangulation;
        emit showTriangulationChanged();
    }

    void setRegionOfStudy(QObject* regionOfStudy) {
        _regionOfStudy = regionOfStudy;
        emit regionOfStudyChanged();
    }

    void setGamma2(QObject* gamma2) {
        _gamma2 = gamma2;
        emit gamma2Changed();
    }

    void setConservacyAreas(QJSValue conservacyAreas) {
        _conservacyAreas = conservacyAreas;
        emit conservacyAreasChanged();
    }

    void setCalculateGamma2(bool calculateGamma2) {
        _calculateGamma2 = calculateGamma2;
        emit calculateGamma2Changed();
    }

    void setWindX(double windX) {
        _windX = windX;
        emit windXChanged();
    }

    void setWindY(double windY) {
        _windY = windY;
        emit windYChanged();
    }

    void setMu(double mu) {
        _mu = mu;
        emit muChanged();
    }

    void setSigma(double sigma) {
        _sigma = sigma;
        emit sigmaChanged();
    }

    void setAlpha(double alpha) {
        _alpha = alpha;
        emit alphaChanged();
    }

    Q_INVOKABLE QVariant doCalculate();
    Q_INVOKABLE void clear();

signals:
    void triangulationSwitchesChanged();
    void showTriangulationChanged();
    void regionOfStudyChanged();
    void gamma2Changed();
    void conservacyAreasChanged();
    void calculateGamma2Changed();
    void windXChanged();
    void windYChanged();
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
    double _windX;
    double _windY;
    double _mu;
    double _sigma;
    double _alpha;
    bool _hasData;
};

#endif // CALCULATIONRESULTCOMPONENT_H

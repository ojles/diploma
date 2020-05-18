#ifndef INTEGRAL_CALCULATION_H
#define INTEGRAL_CALCULATION_H

#include <math.h>
#include <eigen3/Eigen/Geometry>

#include "triangulate.h"

using std::pair;

namespace intcalc {
    struct Vector2d {
        double x;
        double y;

        Vector2d() {
        }

        Vector2d(double x, double y) : x(x), y(y) {
        }

        Vector2d operator/(double value) const {
            return Vector2d(x / value, y / value);
        }

        double operator*(Vector2d other) {
            return this->x * other.x + this->y * other.y;
        }

        Vector2d operator*(double scalar) {
            return Vector2d(x * scalar, y * scalar);
        }
    };

    struct DiscreteElement {
        double x[3];
        double y[3];

        double eLen(uint8_t idx) {
            if (idx > 2) {
                throw "Invalid index for DexreteElement edge length!";
            }

            uint8_t a = (idx + 1) % 3;
            uint8_t b = (idx + 2) % 3;
            return sqrt((x[a] - x[b]) * (x[a] - x[b]) + (y[a] - y[b]) * (y[a] - y[b]));
        }

        Vector2d eCenter(uint8_t idx) {
            if (idx > 2) {
                throw "Invalid index for DexreteElement edge center!";
            }

            uint8_t a = (idx + 1) % 3;
            uint8_t b = (idx + 2) % 3;
            return Vector2d(
                (x[a] + x[b]) / 2.0,
                (y[a] + y[b]) / 2.0
            );
        }
    };

    struct ElementMatrix {
        double data[3][3];
        int globalIndices[3];
    };

    class GlobalMatrix {
    public:
        GlobalMatrix(int size, vector<int> innerIndices, int allPoints)
        {
            this->size = size;

            idxMap = new int[allPoints];
            for (int i = 0; i < allPoints; i++) {
                idxMap[i] = -1;
            }
            for (int i = 0; i < innerIndices.size(); i++) {
                idxMap[innerIndices[i]] = i;
            }

            data = new double*[size];
            for (int i = 0; i < size; i++) {
                data[i] = new double[size];
                for (int j = 0; j < size; j++) {
                    data[i][j] = 0;
                }
            }
        }

        ~GlobalMatrix() {
            for (int i = 0; i < size; i++) {
                delete data[i];
            }
            delete data;
            delete idxMap;
        }

        inline void setAt(int i, int j, double value) {
            data[idxMap[i]][idxMap[j]] = value;
        }

        inline double getAt(int i, int j) {
            return data[idxMap[i]][idxMap[j]];
        }

        inline double getAtNormal(int i, int j) {
            return data[i][j];
        }

        void merge(ElementMatrix& elm, vector<int> vertices) {
            for (auto i : vertices) {
                for (auto j : vertices) {
                    setAt(
                        elm.globalIndices[i],
                        elm.globalIndices[j],
                        getAt(elm.globalIndices[i], elm.globalIndices[j]) + elm.data[i][j]
                    );
                }
            }
        }

        int getSize() {
            return size;
        }

        int* getIdxMap() {
            return idxMap;
        }

    private:
        double **data;
        int* idxMap;
        int size;
    };

    struct Point2DValue {
        double x;
        double y;
        double value;
    };

    struct CalcSolution {
        vector<Point2DValue> triangles;
    };

    class FEMCalculator {
    public:
        FEMCalculator();

        CalcSolution solve();

        // TODO: rename to region of study or something else
        void setPoints(vector<Vector2d>* points) {
            this->points = points;
        }

        void setBoundaryCondition(vector<Vector2d>* boundaryCondition) {
            this->boundaryCondition = boundaryCondition;
        }

        void setMu(double mu) {
            this->mu = mu;
        }

        void setAlpha(double alpha) {
            this->alpha = alpha;
        }

        void setBeta(Vector2d (*beta)(double x, double y)) {
            this->beta = beta;
        }

        void setSigma(double sigma) {
            this->sigma = sigma;
        }

        void setF(std::function<double(double, double)> f) {
            this->f = f;
        }

        void setTriangulationSwitches(QString triangulationSwitches) {
            this->triangulationSwitches = triangulationSwitches;
        }

    private:
        pair<vector<int>, vector<int>> splitInnerAndOuterVertices(triangulateio& out);
        double b(Vector2d edgeCenter);
        double m_ij(int i, int j, DiscreteElement& el, vector<uint8_t>& edges);
        ElementMatrix m(DiscreteElement& el, vector<int> vertices);
        GlobalMatrix M(triangulateio& out, vector<int> innerVertices);
        void requireDataNotNull();

        vector<Vector2d>* points;
        vector<Vector2d>* boundaryCondition;

        double mu;
        double alpha;
        double sigma;
        std::function<Vector2d(double, double)> beta;
        std::function<double(double, double)> f;
        QString triangulationSwitches;
    };
}

#endif // INTEGRAL_CALCULATION_H

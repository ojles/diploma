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

        bool onLine(const Vector2d& a, const Vector2d& b) const {
            if (!onSegment(a, *this, b)) {
                return false;
            }

            double diff = (x - a.x) / (b.x - a.x) - (y - a.y) / (b.y - a.y);
            // TODO: move this value to a constant
            return abs(diff) <= 0.000000001;
        }

        double distanceTo(const Vector2d& other) const {
            return sqrt(pow(this->x - other.x, 2) + pow(this->y - other.y, 2));
        }

    private:
        bool onSegment(const Vector2d& p, const Vector2d& q, const Vector2d& r) const {
            if (q.x <= std::max(p.x, r.x)
                    && q.x >= std::min(p.x, r.x)
                    && q.y <= std::max(p.y, r.y)
                    && q.y >= std::min(p.y, r.y)) {
               return true;
            }
            return false;
        }
    };

    struct VertexInfo : Vector2d {
        enum Type {
            NONE,
            GAMMA_1,
            GAMMA_2,
            INNER
        };

        Type type;
        unsigned int globalIdx;

        VertexInfo(double x, double y, unsigned int globalIdx)
            : Vector2d(x, y),
              type(NONE),
              globalIdx(globalIdx) {
        }

        void setType(Type type) {
            if (this->type == NONE) {
                this->type = type;
            } else {
                throw "You can set a vertex type only once!";
            }
        }
    };

    class Region {
    public:
        Region()
            : _points(nullptr), _isClosed(false) {
        }

        Region(const QVector<Vector2d>* points, bool isClosed)
            : _points(points), _isClosed(isClosed) {
        }

        bool hasVertexOnContour(const Vector2d& vertex) const {
            if (_points == nullptr) {
                return false;
            }

            const int pointsSize = _points->size();
            for (int i = 0; i < pointsSize - 1; i++) {
                Vector2d a = _points->at(i);
                Vector2d b = _points->at(i + 1);
                if (vertex.onLine(a, b)) {
                    return true;
                }
            }

            Vector2d a = _points->at(0);
            Vector2d b = _points->at(pointsSize - 1);
            if (_isClosed && vertex.onLine(a, b)) {
                return true;
            }
            return false;
        }

        bool hasLineOnContour(const Vector2d& a, const Vector2d& b) const {
            if (_points == nullptr) {
                return false;
            }

            const int pointsSize = _points->size();
            for (int i = 0; i < pointsSize - 1; i++) {
                Vector2d c = _points->at(i);
                Vector2d d = _points->at(i + 1);
                if (a.onLine(c, d) && b.onLine(c, d)) {
                    return true;
                }
            }

            Vector2d c = _points->at(0);
            Vector2d d = _points->at(pointsSize - 1);
            if (_isClosed && a.onLine(c, d) && b.onLine(c, d)) {
                return true;
            }
            return false;
        }

        const QVector<Vector2d>* points() const {
            return _points;
        }

    private:
        const QVector<Vector2d>* _points;
        bool _isClosed;
    };

    struct DiscreteElement {
        const VertexInfo* v[3];

        double eLen(uint8_t idx) const {
            if (idx > 2) {
                throw "Invalid index for DexreteElement edge length!";
            }

            uint8_t a = (idx + 1) % 3;
            uint8_t b = (idx + 2) % 3;
            return v[a]->distanceTo(*v[b]);
        }

        Vector2d eCenter(uint8_t idx) const {
            if (idx > 2) {
                throw "Invalid index for DexreteElement edge center!";
            }

            uint8_t a = (idx + 1) % 3;
            uint8_t b = (idx + 2) % 3;
            return Vector2d(
                (v[a]->x + v[b]->x) / 2.0,
                (v[a]->y + v[b]->y) / 2.0
            );
        }
    };

    struct ElementMatrix {
        static const unsigned int SIZE = 3;
        double data[SIZE][SIZE];
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

        Point2DValue()
            : x(0.0), y(0.0), value(0.0) {
        }
    };

    struct CalcSolution {
        QVector<Point2DValue> vertices;
        QVector<int> triangleIndices;
    };

    class FEMCalculator {
    public:
        FEMCalculator();

        CalcSolution solve();

        // TODO: rename to region of study or something else
        void setRegionOfStudy(const QVector<Vector2d>* rosPoints) {
            _regionOfStudy = Region(rosPoints, true);
        }

        void setGamma2(QVector<Vector2d>* gamma2Points) {
            _gamma2 = Region(gamma2Points, false);
        }

        void setMu(double mu) {
            _mu = mu;
        }

        void setAlpha(double alpha) {
            _alpha = alpha;
        }

        void setBeta(Vector2d (*beta)(const Vector2d& vertex)) {
            _beta = beta;
        }

        void setSigma(double sigma) {
            _sigma = sigma;
        }

        void setF(std::function<double(const VertexInfo&)> f) {
            _f = f;
        }

        void setTriangulationSwitches(QString triangulationSwitches) {
            _triangulationSwitches = triangulationSwitches;
        }

    private:
        pair<vector<int>, vector<int>> splitInnerAndOuterVertices(triangulateio& out);
        double b(Vector2d edgeCenter);
        double m_ij(int i, int j, const DiscreteElement& el, vector<int>& edges);
        void m(double** g, const DiscreteElement& el, const int* idx);
        double** M(QVector<VertexInfo>* vertices, QVector<int*> triangles);
        void requireDataNotNull();

        Region _regionOfStudy;
        Region _gamma2;

        double _mu;
        double _alpha;
        double _sigma;
        std::function<Vector2d(const Vector2d&)> _beta;
        std::function<double(const VertexInfo&)> _f;
        QString _triangulationSwitches;
    };
}

#endif // INTEGRAL_CALCULATION_H

#ifndef INTEGRAL_CALCULATION_H
#define INTEGRAL_CALCULATION_H

#include <math.h>
#include <eigen3/Eigen/Geometry>
#include <limits>
#include <QDebug>

#include "triangulate.h"

using std::pair;

namespace intcalc {
    struct Vector2d {
        double x;
        double y;

        Vector2d() : Vector2d(0, 0) {
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
            if (!onSegment(a, b)) {
                return false;
            }

            double diff = (x - a.x) / (b.x - a.x) - (y - a.y) / (b.y - a.y);
            // TODO: move this value to a constant
            return abs(diff) <= 0.000000001;
        }

        double distanceTo(const Vector2d& other) const {
            return sqrt(pow(this->x - other.x, 2) + pow(this->y - other.y, 2));
        }

        bool onSegment(const Vector2d& a, const Vector2d& b) const {
            return _onSegment(a, *this, b);
        }

        static int _orientation(const Vector2d& p, const Vector2d& q, const Vector2d& r) {
            double val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

            // TODO: define this constant somewhere
            if (abs(val) <= 0.000000001) {
                return 0;
            }

            return (val > 0) ? 1 : 2; // clock or counterclock wise
        }

        static bool _onSegment(const Vector2d& p, const Vector2d& q, const Vector2d& r) {
            if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x)
                    && q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y)) {
               return true;
            }
            return false;
        }

        static bool _doIntersect(const Vector2d& p1, const Vector2d& q1, const Vector2d& p2, const Vector2d& q2) {
            int o1 = _orientation(p1, q1, p2);
            int o2 = _orientation(p1, q1, q2);
            int o3 = _orientation(p2, q2, p1);
            int o4 = _orientation(p2, q2, q1);

            // we need the lines to be colinear
            if (o1 != o2 && o3 != o4) {
                return true;
            }
            // p1, q1 and p2 are colinear and p2 lies on segment p1q1
            if (o1 == 0 && _onSegment(p1, p2, q1)) {
                return true;
            }
            // p1, q1 and q2 are colinear and q2 lies on segment p1q1
            if (o2 == 0 && _onSegment(p1, q2, q1)) {
                return true;
            }
            // p2, q2 and p1 are colinear and p1 lies on segment p2q2
            if (o3 == 0 && _onSegment(p2, p1, q2)) {
                return true;
            }
             // p2, q2 and q1 are colinear and q1 lies on segment p2q2
            if (o4 == 0 && _onSegment(p2, q1, q2)) {
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


        VertexInfo(double x, double y)
            : Vector2d(x, y),
              _type(NONE),
              _isInConservacyArea(false) {
        }

        void setType(Type type) {
            if (_type == NONE) {
                _type = type;
            } else {
                throw "You can set a vertex type only once!";
            }
        }

        void setIsInConservacyArea(bool isInConservacyArea) {
            _isInConservacyArea = isInConservacyArea;
        }

        Type type() const {
            return _type;
        }

        bool isInConservacyArea() const {
            return _isInConservacyArea;
        }

    private:
        Type _type;
        bool _isInConservacyArea;
    };

    class Region {
    public:
        Region()
            : _points(nullptr), _isClosed(false) {
        }

        Region(const QVector<Vector2d>* points, bool isClosed)
            : _points(points), _isClosed(isClosed) {
            if (isClosed) {
                _bottomLeft.x = points->at(0).x;
                _bottomLeft.y = points->at(0).y;
                _topRight.x = points->at(0).x;
                _topRight.y = points->at(0).y;
                for (auto point : *points) {
                    if (point.x < _bottomLeft.x) {
                        _bottomLeft.x = point.x;
                    }
                    if (point.x > _topRight.x) {
                        _topRight.x = point.x;
                    }
                    if (point.y < _bottomLeft.y) {
                        _bottomLeft.y = point.y;
                    }
                    if (point.y > _topRight.y) {
                        _topRight.y = point.y;
                    }
                }
            }
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

            return false;
        }

        bool hasVertexInside(const Vector2d& vertex) const {
            if (!_isClosed) {
                throw "This region is not closed!";
            }

            if (_bottomLeft.x > vertex.x || vertex.x > _topRight.x
                    || _bottomLeft.y > vertex.y || vertex.y > _topRight.y) {
                return false;
            }

            Vector2d extreme(vertex.x, std::numeric_limits<double>::max());
            int count = 0;
            const int pointsSize = _points->size();
            for (int i = 0; i < pointsSize - 1; i++) {
                Vector2d a = _points->at(i);
                Vector2d b = _points->at(i + 1);
                if (Vector2d::_doIntersect(a, b, vertex, extreme)) {
                    if (Vector2d::_orientation(a, vertex, b) == 0) {
                        return Vector2d::_onSegment(a, vertex, b);
                    }
                    count++;
                }
            }

            return (count % 2) == 1;
        }

        const QVector<Vector2d>* points() const {
            return _points;
        }

    private:
        const QVector<Vector2d>* _points;
        Vector2d _bottomLeft;
        Vector2d _topRight;
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

    struct Point2DValue {
        double x;
        double y;
        double value;
        bool isOnContour;

        Point2DValue()
            : x(0.0), y(0.0), value(0.0), isOnContour(true) {
        }
    };

    struct CalcSolution {
        QVector<Point2DValue> vertices;
        QVector<int> triangleIndices;
        QVector<Point2DValue> minVertices;
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

        void setConservacyAreas(QVector<QVector<Vector2d>*> conservacyAreas) {
            for (auto area : conservacyAreas) {
                _conservacyAreas.push_back(Region(area, true));
            }
        }

        void setMu(double mu) {
            _mu = mu;
        }

        void setAlpha(double alpha) {
            _alpha = alpha;
        }

        void setBeta(double x, double y) {
            _beta = Vector2d(x, y);
        }

        void setSigma(double sigma) {
            _sigma = sigma;
        }

        void setTriangulationOptions(double minAngle, double maxArea) {
            _triangulationSwitches = QString("zq%0a%1")
                    .arg(minAngle)
                    .arg(QString::number(maxArea, 'f', 10));
        }

    private:
        QVector<VertexInfo>* prepareDiscreteVerticies(triangulateio& out);
        double b(Vector2d edgeCenter);
        double m_ij(int i, int j, const DiscreteElement& el, vector<int>& edges);
        void m(double** g, const DiscreteElement& el, const int* idx);
        double** M(QVector<VertexInfo>* vertices, QVector<int*> triangles);
        void requireDataNotNull();

        Region _regionOfStudy;
        Region _gamma2;
        QVector<Region> _conservacyAreas;

        double _mu;
        double _alpha;
        double _sigma;
        Vector2d _beta;
        QString _triangulationSwitches;
    };
}

#endif // INTEGRAL_CALCULATION_H

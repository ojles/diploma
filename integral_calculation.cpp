#include "integral_calculation.h"

#include <math.h>
#include <algorithm>
#include <QDebug>
#include <QElapsedTimer>

#include "integral_calculation_utils.h"

#ifdef INTCALC_DEBUG
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#endif

namespace intcalc {
    FEMCalculator::FEMCalculator()
        : _beta(nullptr) {
    }

    double jacobian(const DiscreteElement& el) {
        return (el.v[1]->x - el.v[0]->x) * (el.v[2]->y - el.v[0]->y) - (el.v[2]->x - el.v[0]->x) * (el.v[1]->y - el.v[0]->y);
    }

    Vector2d nablaPhi(int vertex, const DiscreteElement& el) {
        Vector2d vector;
        if (vertex == 0) {
            vector.x = el.v[1]->y - el.v[2]->y;
            vector.y = el.v[2]->x - el.v[1]->x;
        }
        else if (vertex == 1) {
            vector.x = el.v[2]->y - el.v[0]->y;
            vector.y = el.v[0]->x - el.v[2]->x;
        }
        else if (vertex == 2) {
            vector.x = el.v[0]->y - el.v[1]->y;
            vector.y = el.v[1]->x - el.v[0]->x;
        }
        else {
            throw "Invalid value of vertex for nablaPhi";
        }
        return vector / jacobian(el);
    }

    double firstIntegral(int i, int j, const DiscreteElement& el) {
        return (abs(jacobian(el)) / 2.0) * (nablaPhi(i, el) * nablaPhi(j, el));
    }

    Vector2d secondIntegral(int j, const DiscreteElement& el) {
        return nablaPhi(j, el) * (abs(jacobian(el)) / 6.0);
    }

    double thirdIntegral(int i, int j, const DiscreteElement& el) {
        if (i == j) {
            return abs(jacobian(el)) / 12.0;
        }
        else {
            return abs(jacobian(el)) / 24.0;
        }
    }

    double fourthIntegral(int i, int j, int edgeIndex, double l) {
        if (edgeIndex < 0 || edgeIndex > 2) {
            throw "Invalid edgeIndex value";
        }

        if (i == edgeIndex || j == edgeIndex) {
            return 0;
        }

        if (i == j) {
            return l / 3;
        }
        else {
            return l / 6;
        }
    }

    Vector2d n(Vector2d& edge) {
        Vector2d nVector;
        nVector.x = 1 / sqrt(1 + pow(edge.x / edge.y, 2));
        nVector.y = - (edge.x / edge.y) * nVector.x;
        return nVector;
    }

    double FEMCalculator::b(Vector2d edgeCenter) {
        return _alpha - _beta(edgeCenter) * n(edgeCenter);
    }

    double FEMCalculator::m_ij(int i, int j, const DiscreteElement& el, vector<int>& edges) {
        double result = _mu * firstIntegral(i, j, el)
                + _beta(*el.v[0]) * secondIntegral(j, el)
                + _sigma * thirdIntegral(i, j, el);

        for (unsigned int k = 0; k < edges.size(); k++) {
            result += b(el.eCenter(edges[k])) * fourthIntegral(i, j, edges[k], el.eLen(edges[k]));
        }

        return result;
    }

    // TODO: rename this method
    // there is a confusion between the two boundary conditions
    vector<int> findOutBoundaryEdges(const DiscreteElement& el, const Region& gamma2) {
        VertexInfo::Type v0Type = el.v[0]->type();
        VertexInfo::Type v1Type = el.v[1]->type();
        VertexInfo::Type v2Type = el.v[2]->type();
        VertexInfo v0 = *el.v[0];
        VertexInfo v1 = *el.v[1];
        VertexInfo v2 = *el.v[2];

        vector<int> edges;
        if (v0Type == v1Type
                && v0Type == VertexInfo::Type::GAMMA_2
                && gamma2.hasLineOnContour(v0, v1)) {
            edges.push_back(2);
        }
        if (v1Type == v2Type
                && v1Type == VertexInfo::Type::GAMMA_2
                && gamma2.hasLineOnContour(v1, v2)) {
            edges.push_back(0);
        }
        if (v2Type == v0Type
                && v2Type == VertexInfo::Type::GAMMA_2
                && gamma2.hasLineOnContour(v2, v0)) {
            edges.push_back(1);
        }
        return edges;
    }

    void FEMCalculator::m(double** g, const DiscreteElement& el, const int* triangle) {
        vector<int> edges = findOutBoundaryEdges(el, _gamma2);

        for (int i = 0; i < 3; i++) {
            if (el.v[i]->type() == VertexInfo::Type::GAMMA_1) {
                continue;
            }
            for (int j = 0; j < 3; j++) {
                if (el.v[j]->type() == VertexInfo::Type::GAMMA_1) {
                    continue;
                }
                g[triangle[i]][triangle[j]] += m_ij(i, j, el, edges);
            }
        }
    }

    double** FEMCalculator::M(QVector<VertexInfo>* vertices, QVector<int*> triangles) {
        int gSize = vertices->size();
        double** g = new double*[gSize];
        for (int i = 0; i < gSize; i++) {
            g[i] = new double[gSize];
            for (int j = 0; j < gSize; j++) {
                g[i][j] = 0.0;
            }
        }

        for (unsigned int i = 0; i < triangles.size(); i++) {
            const int* triangle = triangles[i];
            DiscreteElement el;
            el.v[0] = &vertices->at(triangle[0]);
            el.v[1] = &vertices->at(triangle[1]);
            el.v[2] = &vertices->at(triangle[2]);
            m(g, el, triangle);
        }

        return g;
    }

    QVector<VertexInfo>* FEMCalculator::prepareDiscreteVerticies(triangulateio& out) {
        QVector<VertexInfo>* vertices = new QVector<VertexInfo>();
        for (int i = 0; i < out.numberofpoints; i++) {
            double x = out.pointlist[i * 2];
            double y = out.pointlist[i * 2 + 1];
            VertexInfo vertex(x, y);

            if (!_regionOfStudy.hasVertexOnContour(vertex)) {
                vertex.setType(VertexInfo::Type::INNER);
            } else if (_gamma2.hasVertexOnContour(vertex)) {
                vertex.setType(VertexInfo::Type::GAMMA_2);
            } else {
                vertex.setType(VertexInfo::Type::GAMMA_1);
            }

            vertex.setIsInConservacyArea(false);
            for (auto region : _conservacyAreas) {
                if (region.hasVertexInside(vertex)) {
                    vertex.setIsInConservacyArea(true);
                }
            }

            vertices->push_back(vertex);
        }
        return vertices;
    }

    QVector<int*> prepareTrianglesVector(const triangulateio& out) {
        QVector<int*> triangles;
        for (int i = 0; i < out.numberoftriangles * out.numberofcorners; i += out.numberofcorners) {
            triangles.push_back(new int[out.numberofcorners] {
                out.trianglelist[i] - 1,
                out.trianglelist[i + 1] - 1,
                out.trianglelist[i + 2] - 1
            });
        }
        return triangles;
    }

    Eigen::MatrixXd solveMatrix(double** g, QVector<VertexInfo>* vertices, std::function<double(const VertexInfo&)> f) {
        vector<int> nonGamma1;
        for (int i = 0; i < vertices->size(); i++) {
            if (vertices->at(i).type() == VertexInfo::Type::GAMMA_1) {
                continue;
            }
            nonGamma1.push_back(i);
        }

        if (nonGamma1.size() <= 0) {
            throw "Triangulation to small, no inner vertices found!";
        }

        Eigen::MatrixXd a(nonGamma1.size(), nonGamma1.size());
        Eigen::MatrixXd b(nonGamma1.size(), 1);
        int localI = 0;
        int localJ = 0;
        for (auto i : nonGamma1) {
            localJ = 0;
            for (auto j : nonGamma1) {
                a(localI, localJ) = g[i][j];
                localJ++;
            }
            b(localI, 0) = f(vertices->at(i));
            localI++;
        }

        return a.fullPivLu().solve(b);
    }

    CalcSolution FEMCalculator::solve() {
        QElapsedTimer timer;
        timer.start();
        qInfo() << "";
        qInfo() << "Star calculations...";
        qInfo() << "Received parameters:";
        qInfo() << "-- mu: " << _mu;
        qInfo() << "-- sigma: " << _sigma;
        qInfo() << "-- alpha: " << _alpha;
        qInfo() << "-- riangulation switches: " << _triangulationSwitches;

        requireDataNotNull();

        triangulateio out = intcalc_utils::doTriangulate(_triangulationSwitches, _regionOfStudy);
        qInfo() << "Finish triangulation (" << timer.restart() << "ms )";
        qInfo() << "--- numberofverticies: " << out.numberofpoints;
        qInfo() << "--- numberoftriangles: " << out.numberoftriangles;

        QVector<VertexInfo>* vertices = prepareDiscreteVerticies(out);
        QVector<int*> triangles = prepareTrianglesVector(out);
        qInfo() << "Prepared verticies (" << timer.restart() << "ms )";

        double** g = M(vertices, triangles);
        qInfo() << "Calculated global matrix M (" << timer.restart() << "ms )";

        Eigen::MatrixXd solutionMatrix = solveMatrix(g, vertices, [](const VertexInfo& vertex) ->double {
            return vertex.isInConservacyArea() ? 1 : 0;
        });
        qInfo() << "Solved Au=f (" << timer.restart() << "ms )";

        CalcSolution solution;
        int solIndex = 0;
        for (auto vertex : *vertices) {
            Point2DValue resultVertex;
            resultVertex.x = vertex.x;
            resultVertex.y = vertex.y;
            resultVertex.value = 0;
            if (vertex.type() != VertexInfo::Type::GAMMA_1) {
                resultVertex.value = solutionMatrix(solIndex, 0);
                solIndex++;
            }
            solution.vertices.push_back(resultVertex);
        }

        for (int i = 0; i < out.numberoftriangles; i++) {
            for (int j = 0; j < 3; j++) {
                solution.triangleIndices.push_back(triangles[i][j]);
            }
        }

        // clean resources
        free(out.trianglelist);
        free(out.pointlist);
        for (auto triangle : triangles) {
            delete triangle;
        }

        qInfo() << "Displayed result (" << timer.restart() << "ms )";
        qInfo() << "End.";
        qInfo() << "";

        return solution;
    }

    void FEMCalculator::requireDataNotNull() {
        if (_beta == nullptr) {
            throw "Beta not specified!";
        }

        if (_regionOfStudy.points() == nullptr) {
            throw "Can't calculate result, regionOfStudy not provided";
        }
    }
}

#include "integral_calculation.h"

#include <math.h>
#include <algorithm>

#include "integral_calculation_utils.h"

#ifdef INTCALC_DEBUG
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#endif

namespace intcalc {
    FEMCalculator::FEMCalculator()
        : points(nullptr),
          boundaryCondition(nullptr),
          beta(nullptr),
          f(nullptr) {
    }

    triangulateio doTriangulate(QString triangulationSwitches, vector<Vector2d>* points) {
        vector<TriangulatePoint> triangulationIn;
        for (unsigned long i = 0; i < points->size(); i++) {
            TriangulatePoint point;
            point.x = points->at(i).x;
            point.y = points->at(i).y;
            triangulationIn.push_back(point);
        }
        Triangulate triangulate;
        char* switchesArray = new char[triangulationSwitches.length()];
        memcpy(switchesArray, triangulationSwitches.toStdString().c_str(), triangulationSwitches.length());
        triangulateio result = triangulate.triangulate(switchesArray, triangulationIn);
        delete[] switchesArray;
        return result;
    }

    vector<int> filterTriPointsOnLine(vector<Vector2d>* line, vector<int> triPoints, triangulateio out) {
        vector<int> res;
        for (unsigned int i = 0; i < line->size() - 1; i++) {
            for (int triPoint : triPoints) {
                Vector2d triPointObj(out.pointlist[triPoint * 2], out.pointlist[triPoint * 2 + 1]);
                if (intcalc_utils::onLine(line->at(i), triPointObj, line->at(i + 1)) ) {
                    res.push_back(triPoint);
                }
            }
        }
        // TODO: check if unique
        return res;
    }

    CalcSolution FEMCalculator::solve() {
        requireDataNotNull();

        // TODO: check if triangulationio is deleted
        triangulateio out = doTriangulate(triangulationSwitches, points);

        pair<vector<int>, vector<int>> splitVertices = splitInnerAndOuterVertices(out);
        vector<int> firstBoundary = splitVertices.first;
        if (boundaryCondition != nullptr) {
            vector<int> secondBoundary = filterTriPointsOnLine(boundaryCondition, splitVertices.second, out);
            firstBoundary.reserve(firstBoundary.size() + secondBoundary.size());
            firstBoundary.insert(firstBoundary.end(), secondBoundary.begin(), secondBoundary.end());
        }

        GlobalMatrix globalMatrix = M(out, firstBoundary);

        vector<double> fValues;
        for (auto i : firstBoundary) {
            fValues.push_back(f(out.pointlist[i * 2], out.pointlist[i * 2 + 1]));
        }
        Eigen::MatrixXd solutionMatrix = intcalc_utils::solveGlobalMatrix(globalMatrix, fValues);

        CalcSolution solution;
        int* idxMap = globalMatrix.getIdxMap();
        for (int i = 0; i < out.numberofpoints; i++) {
            int globalIdx = idxMap[i];
            Point2DValue point;
            point.x = out.pointlist[i * 2];
            point.y = out.pointlist[i * 2 + 1];
            point.value = globalIdx == -1 ? 0 : solutionMatrix(globalIdx, 0);
            solution.vertices.push_back(point);
        }
        for (int i = 0; i < out.numberoftriangles * out.numberofcorners; i++) {
            solution.triangleIndices.push_back(out.trianglelist[i] - 1);
        }

        return solution;
    }

    double jacobian(DiscreteElement& el) {
        return (el.x[1] - el.x[0]) * (el.y[2] - el.y[0]) - (el.x[2] - el.x[0]) * (el.y[1] - el.y[0]);
    }

    Vector2d nablaPhi(int vertex, DiscreteElement& el) {
        Vector2d vector;
        if (vertex == 0) {
            vector.x = el.y[1] - el.y[2];
            vector.y = el.x[2] - el.x[1];
        }
        else if (vertex == 1) {
            vector.x = el.y[2] - el.y[0];
            vector.y = el.x[0] - el.x[2];
        }
        else if (vertex == 2) {
            vector.x = el.y[0] - el.y[1];
            vector.y = el.x[1] - el.x[0];
        }
        else {
            throw "Invalid value of vertex for nablaPhi";
        }
        return vector / jacobian(el);
    }

    double firstIntegral(int i, int j, DiscreteElement& el) {
        return (abs(jacobian(el)) / 2.0) * (nablaPhi(i, el) * nablaPhi(j, el));
    }

    Vector2d secondIntegral(int i, int j, DiscreteElement& el) {
        return nablaPhi(j, el) * (abs(jacobian(el)) / 6.0);
    }

    double thirdIntegral(int i, int j, DiscreteElement& el) {
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

    pair<vector<int>, vector<int>> FEMCalculator::splitInnerAndOuterVertices(triangulateio& out) {
        vector<int> inner;
        vector<int> outer;
        for (int i = 0; i < out.numberofpoints; i++) {
            inner.push_back(i);
        }

        // TODO: rename points to outer points/lines
        // TODO: rename this->points to simething else, like a contour
        for (unsigned int i = 0; i < points->size(); i++) {
            Vector2d first = points->at(i);
            Vector2d second = (i + 1 < points->size()) ? points->at(i + 1) : points->at(0);
            for (int j = 0; j < out.numberofpoints; j++) {
                Vector2d triangleVertex(out.pointlist[j * 2], out.pointlist[j * 2 + 1]);
                if (intcalc_utils::onLine(first, triangleVertex, second)) {
                    auto outerVertex = find(inner.begin(), inner.end(), j);
                    if (outerVertex != inner.end()) {
                        inner.erase(outerVertex);
                        outer.push_back(j);
                    }
                }
            }
        }

        // TODO: it's probably useless here because the array is already sorted
        std::sort(inner.begin(), inner.end());
        std::sort(outer.begin(), outer.end());

        return pair<vector<int>, vector<int>>(inner, outer);
    }

    double FEMCalculator::b(Vector2d edgeCenter) {
        return alpha - beta(edgeCenter.x, edgeCenter.y) * n(edgeCenter);
    }

    double FEMCalculator::m_ij(int i, int j, DiscreteElement& el, vector<uint8_t>& edges) {
        double result = mu * firstIntegral(i, j, el)
                + beta(el.x[0], el.y[0]) * secondIntegral(i, j, el)
                + sigma * thirdIntegral(i, j, el);

        for (unsigned int k = 0; k < edges.size(); k++) {
            result += b(el.eCenter(edges[k])) * fourthIntegral(i, j, edges[k], el.eLen(edges[k]));
        }

        return result;
    }

    // TODO: rename this method
    // there is a confusion between the two boundary conditions
    vector<uint8_t> findOutBoundaryEdges(DiscreteElement& el, vector<Vector2d>* boundaryCondition) {
        vector<uint8_t> edges;
        for (unsigned int i = 0; i < boundaryCondition->size() - 1; i++) {
            Vector2d p1(el.x[0], el.y[0]);
            Vector2d q1(el.x[1], el.y[1]);
            Vector2d p2(boundaryCondition->at(i).x, boundaryCondition->at(i).y);
            Vector2d q2(boundaryCondition->at(i + 1).x, boundaryCondition->at(i + 1).y);
            if (intcalc_utils::doIntersect(p1, q1, p2, q2)) {
                edges.push_back(2);
            }

            p1.x = el.x[1];
            p1.y = el.y[1];
            q1.x = el.x[2];
            q1.y = el.y[2];
            if (intcalc_utils::doIntersect(p1, q1, p2, q2)) {
                edges.push_back(0);
            }

            p1.x = el.x[2];
            p1.y = el.y[2];
            q1.x = el.x[0];
            q1.y = el.y[0];
            if (intcalc_utils::doIntersect(p1, q1, p2, q2)) {
                edges.push_back(1);
            }
        }

        std::vector<uint8_t>::iterator it = std::unique(edges.begin(), edges.end());
        edges.resize(std::distance(edges.begin(), it));

#ifdef INTCALC_DEBUG
        if (edges.size() > 0) {
            cout << "Found " << edges.size() << " edges!" << endl;
        }
#endif

        return edges;
    }

    ElementMatrix FEMCalculator::m(DiscreteElement& el, vector<int> vertices) {
        vector<uint8_t> edges;
        if (boundaryCondition != nullptr) {
            edges = findOutBoundaryEdges(el, boundaryCondition);
        }

        ElementMatrix matrix;
        for (auto i : vertices) {
            for (auto j : vertices) {
                matrix.data[i][j] = m_ij(i, j, el, edges);
            }
        }

        return matrix;
    }

    GlobalMatrix FEMCalculator::M(triangulateio& out, vector<int> innerVertices) {
        GlobalMatrix globalMatrix(innerVertices.size(), innerVertices, out.numberofpoints);
        for (int i = 0; i < out.numberoftriangles; i++) {
            int firstVertex = out.trianglelist[i * out.numberofcorners] - 1;
            int secondVertex = out.trianglelist[i * out.numberofcorners + 1] - 1;
            int thirdVertex = out.trianglelist[i * out.numberofcorners + 2] - 1;

            DiscreteElement element;
            element.x[0] = out.pointlist[firstVertex * 2];
            element.y[0] = out.pointlist[firstVertex * 2 + 1];
            element.x[1] = out.pointlist[secondVertex * 2];
            element.y[1] = out.pointlist[secondVertex * 2 + 1];
            element.x[2] = out.pointlist[thirdVertex * 2];
            element.y[2] = out.pointlist[thirdVertex * 2 + 1];

            vector<int> localVertices;
            if (find(innerVertices.begin(), innerVertices.end(), firstVertex) != innerVertices.end()) {
                localVertices.push_back(0);
            }
            if (find(innerVertices.begin(), innerVertices.end(), secondVertex) != innerVertices.end()) {
                localVertices.push_back(1);
            }
            if (find(innerVertices.begin(), innerVertices.end(), thirdVertex) != innerVertices.end()) {
                localVertices.push_back(2);
            }

            ElementMatrix elementMatrix = m(element, localVertices);
            elementMatrix.globalIndices[0] = firstVertex;
            elementMatrix.globalIndices[1] = secondVertex;
            elementMatrix.globalIndices[2] = thirdVertex;

            globalMatrix.merge(elementMatrix, localVertices);

#ifdef INTCALC_DEBUG
            if (out.numberoftriangles <= 100) {
                cout << endl << "Triangle #" << i << endl;
                cout << firstVertex << ": (" << element.x[0] << ", " << element.y[0] << "); "
                     << secondVertex << ": (" << element.x[1] << ", " << element.y[1] << "); "
                     << thirdVertex << ": (" << element.x[2] << ", " << element.y[2] << ");"
                     << endl;
                cout << "Element matrix:" << endl;
                cout << elementMatrix.data[0][0] << "\t" << elementMatrix.data[0][1] << "\t" << elementMatrix.data[0][2] << endl
                     << elementMatrix.data[1][0] << "\t" << elementMatrix.data[1][1] << "\t" << elementMatrix.data[1][2] << endl
                     << elementMatrix.data[2][0] << "\t" << elementMatrix.data[2][1] << "\t" << elementMatrix.data[2][2] << endl;
            }
#endif
        }

#ifdef INTCALC_DEBUG
            if (out.numberoftriangles <= 100) {
                cout << "Size: " << globalMatrix.getSize() << endl
                     << "triangle count: " << out.numberoftriangles << endl
                     << "vertex count: " << out.numberofedges << endl;
                cout << endl << "(=) Global matrix:" << endl;
                for (int i = 0; i < globalMatrix.getSize(); i++) {
                    for (int j = 0; j < globalMatrix.getSize(); j++) {
                        cout << globalMatrix.getAtNormal(i, j) << ", ";
                    }
                    cout << endl;
                }
            }

#endif

        return globalMatrix;
    }

    void FEMCalculator::requireDataNotNull() {
        if (beta == nullptr || f == nullptr) {
            throw "Functions not specified!";
        }

        if (points == nullptr) {
            throw "Can't calculate result, points not provided";
        }
    }
}

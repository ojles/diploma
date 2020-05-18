#include "integral_calculation_utils.h"


bool intcalc_utils::onSegment(intcalc::Vector2d p, intcalc::Vector2d q, intcalc::Vector2d r) {
    if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x)
            && q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y)) {
       return true;
    }

    return false;
}

int intcalc_utils::orientation(intcalc::Vector2d p, intcalc::Vector2d q, intcalc::Vector2d r) {
    double val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    // TODO: define this constant somewhere
    if (abs(val) <= 0.000000001) {
        return 0;
    }

    return (val > 0) ? 1 : 2; // clock or counterclock wise
}

bool intcalc_utils::doIntersect(intcalc::Vector2d p1, intcalc::Vector2d q1, intcalc::Vector2d p2, intcalc::Vector2d q2) {
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // we need the lines to be colinear
    if (o1 != o2 && o3 != o4) {
        return false;
    }

    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) {
        return true;
    }

    // p1, q1 and q2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) {
        return true;
    }

    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) {
        return true;
    }

     // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) {
        return true;
    }

    return false;
}

bool intcalc_utils::onLine(intcalc::Vector2d a, intcalc::Vector2d b, intcalc::Vector2d c) {
    if (!onSegment(a, b, c)) {
        return false;
    }

    double diff = (b.x - a.x) / (c.x - a.x) - (b.y - a.y) / (c.y - a.y);
    // TODO: move this value to a constant
    return abs(diff) <= 0.000000001;
}

Eigen::MatrixXd intcalc_utils::solveGlobalMatrix(intcalc::GlobalMatrix& matrix, vector<double>& fValues) {
    Eigen::MatrixXd a(matrix.getSize(), matrix.getSize());
    for (int i = 0; i < matrix.getSize(); i++) {
        for (int j = 0; j < matrix.getSize(); j++) {
            a(i, j) = matrix.getAtNormal(i, j);
        }
    }

    Eigen::MatrixXd b(matrix.getSize(), 1);
    for (unsigned int i = 0; i < fValues.size(); i++) {
        b(i, 0) = fValues[i];
    }

    return a.fullPivLu().solve(b);
}

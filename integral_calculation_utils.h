#ifndef INTEGRAL_CALCULATION_UTILS_H
#define INTEGRAL_CALCULATION_UTILS_H

#include <eigen3/Eigen/Geometry>

#include "integral_calculation.h"

namespace intcalc_utils
{
    // Given three colinear points p, q, r, the function checks if
    // point q lies on line segment 'pr'
    bool onSegment(intcalc::Vector2d p, intcalc::Vector2d q, intcalc::Vector2d r);

    // 0 --> p, q and r are colinear
    // 1 --> Clockwise
    // 2 --> Counterclockwise
    int orientation(intcalc::Vector2d p, intcalc::Vector2d q, intcalc::Vector2d r);

    // The main function that returns true if line segment 'p1q1'
    // and 'p2q2' intersect.
    bool doIntersect(intcalc::Vector2d p1, intcalc::Vector2d q1, intcalc::Vector2d p2, intcalc::Vector2d q2);

    // Check if point b is on line ac
    bool onLine(intcalc::Vector2d a, intcalc::Vector2d b, intcalc::Vector2d c);

    // wrapper function for triangulation
    triangulateio doTriangulate(QString triangulationSwitches, intcalc::Region& regionOfStudy);

    // remove those points from triPoints that are on the line
    vector<int> filterTriPointsOnLine(vector<intcalc::Vector2d>* line, vector<int> triPoints, triangulateio out);
}

#endif // INTEGRAL_CALCULATION_UTILS_H

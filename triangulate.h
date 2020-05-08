#ifndef TRIANGULATE_H
#define TRIANGULATE_H

#include <vector>
#include <triangle.h>
#include <QObject>

using std::vector;

extern "C"
{
    void triangulate(char *triswitches, triangulateio *in, triangulateio *out, triangulateio *vorout);
}

struct TriangulatePoint
{
    double x;
    double y;
};

class Triangulate
{
public:
    Triangulate();

    triangulateio triangulate(char* switches, vector<TriangulatePoint> &points);

    vector<TriangulatePoint> triangulateSimplified(char* switches, vector<TriangulatePoint> &points);
};

#endif // TRIANGULATE_H

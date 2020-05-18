#include "triangulate.h"

Triangulate::Triangulate() {
}

triangulateio Triangulate::triangulate(char* switches, vector<TriangulatePoint> &points) {
    triangulateio in, out;

    in.numberofpoints = (int) points.size();
    in.numberofpointattributes = 0;
    in.numberofsegments = 0;
    in.numberofholes = 0;
    in.numberofregions = 0;
    in.pointlist = new double[in.numberofpoints * 2];
    for (int i = 0; i < in.numberofpoints * 2; i += 2) {
        TriangulatePoint point = points.at(i / 2);
        in.pointlist[i] = point.x;
        in.pointlist[i + 1] = point.y;
    }

    in.pointmarkerlist = new int[in.numberofpoints];
    for (int i = 0; i < in.numberofpoints; i++) {
        in.pointmarkerlist[i] = 0;
    }

    out.pointlist = nullptr;
    out.pointattributelist = nullptr;
    out.pointmarkerlist = nullptr;
    out.trianglelist = nullptr;
    out.triangleattributelist = nullptr;
    out.neighborlist = nullptr;
    out.segmentlist = nullptr;
    out.segmentmarkerlist = nullptr;
    out.edgelist = nullptr;
    out.edgemarkerlist = nullptr;

    ::triangulate(switches, &in, &out, nullptr);

    free(in.pointlist);
    free(in.pointmarkerlist);

    return out;
}

vector<TriangulatePoint> Triangulate::triangulateSimplified(char* switches, vector<TriangulatePoint> &points) {
    triangulateio out = triangulate(switches, points);

    vector<TriangulatePoint> resultPoints;
    for (int i = 0; i < out.numberoftriangles; i++) {
        for (int j = 0; j < out.numberofcorners; j++) {
            int vertexIndex = out.trianglelist[i * out.numberofcorners + j] - 1;
            TriangulatePoint point;
            point.y = out.pointlist[vertexIndex * 2 + 1];
            point.x = out.pointlist[vertexIndex * 2];
            resultPoints.push_back(point);
        }
    }

    free(out.pointlist);
    free(out.pointattributelist);
    free(out.pointmarkerlist);
    free(out.trianglelist);
    free(out.triangleattributelist);
    free(out.neighborlist);
    free(out.segmentlist);
    free(out.segmentmarkerlist);
    free(out.edgelist);
    free(out.edgemarkerlist);

    return resultPoints;
}

import QtQuick 2.0
import QtPositioning 5.12
import QtLocation 5.12

import "Vector.js" as V

MapPolyline {
    property var mapRegion
    property var _mapRegionEdges
    property var _vi

    onMapRegionChanged: {
        _mapRegionEdges = null;
    }

    function clearFromMap() {
        path = [];
        _mapRegionEdges = null;
    }

    function addCoordinateToContour(coordinate) {
        if (!_mapRegionEdges) {
            _mapRegionEdges = _createEdgesFromCoordinates(mapRegion.path);
            _vi = [
                ...Array(_mapRegionEdges.length).keys(),
                ...Array(_mapRegionEdges.length).keys()
            ];
        }

        const projection = _findClosestProjection(coordinate);

        if (path.length === 0) {
            addCoordinate(QtPositioning.coordinate(projection.latitude, projection.longitude));
            return;
        }

        let start = _findClosestProjection(path[path.length - 1]);
        let end = projection;

        if (typeof start.vertexIndex !== 'undefined' && typeof end.vertexIndex !== 'undefined'
                && start.vertexIndex === end.vertexIndex) {
            return;
        }

        if (start.latitude === end.latitude && start.longitude === end.longitude) {
            return;
        }

        _generateInnerVertices(start, end)
            .forEach(vertex => {
                addCoordinate(mapRegion.path[vertex]);
            });

        addCoordinate(QtPositioning.coordinate(projection.latitude, projection.longitude));
    }

    function _createEdgesFromCoordinates(coordinates) {
        const edges = [];
        for (let i = 0; i < coordinates.length - 1; i++) {
            edges.push(
                new V.Vector(
                   coordinates[i].latitude,
                   coordinates[i].longitude,
                   coordinates[i + 1].latitude,
                   coordinates[i + 1].longitude
                )
           );
        }
        return edges;
    }

    function _findClosestProjection(coordinate) {
        const projection = _mapRegionEdges
            .filter((edge, index) => {
                const v1 = new V.Vector(coordinate.latitude, coordinate.longitude, edge.x1, edge.y1);
                const v2 = new V.Vector(coordinate.latitude, coordinate.longitude, edge.x2, edge.y2);
                const v1DotLine = v1.dot(edge);
                const v2DotLine = v2.dot(edge);
                return (v1DotLine > 0 && v2DotLine < 0) || (v1DotLine < 0 && v2DotLine > 0);
            })
            .map(edge => {
                const projection = edge.projectPoint(coordinate.latitude, coordinate.longitude);
                const distanceToEdge = Math.sqrt(
                     Math.pow(coordinate.latitude - projection.x, 2)
                     +
                     Math.pow(coordinate.longitude - projection.y, 2)
                );
                return {
                    latitude: projection.x,
                    longitude: projection.y,
                    edge,
                    distanceToEdge
                };
            })
            .concat(
                mapRegion.path
                    .slice(0, mapRegion.path.length - 1)
                    .map((vertex, vertexIndex) => ({
                        latitude: vertex.latitude,
                        longitude: vertex.longitude,
                        vertexIndex,
                        distanceToEdge: Math.sqrt(
                             Math.pow(coordinate.latitude - vertex.latitude, 2)
                             +
                             Math.pow(coordinate.longitude - vertex.longitude, 2)
                        )
                    }))
             )
            .sort((projection1, projection2) => projection1.distanceToEdge - projection2.distanceToEdge)
            [0];

        if (projection.edge) {
            projection.edgeIndex = _mapRegionEdges.indexOf(projection.edge);
        }

        return projection;
    }


    function _generateInnerVertices(start, end) {
        let vertices = [];
        if (typeof start.edgeIndex !== 'undefined' && typeof end.edgeIndex !== 'undefined') {
            if (start.edgeIndex === end.edgeIndex) {
                const a = mapRegion.path[start.edgeIndex];
                const b = mapRegion.path[start.edgeIndex + 1];
                const fromAToStart = Math.pow(a.latitude - start.latitude, 2) + Math.pow(a.longitude - start.longitude, 2);
                const fromAToEnd = Math.pow(a.latitude - end.latitude, 2) + Math.pow(a.longitude - end.longitude, 2);
                if (fromAToStart < fromAToEnd) {
                    return [];
                } else {
                    return _genSequence(start.edgeIndex + 1, start.edgeIndex);
                }
            } else {
                return _genSequence(start.edgeIndex + 1, end.edgeIndex);
            }
        }

        if (typeof start.vertexIndex !== 'undefined' && typeof end.vertexIndex !== 'undefined') {
            if (start.vertexIndex === end.vertexIndex || end.vertexIndex - start.vertexIndex === 1) {
                return [];
            } else {
                return _genSequence(start.vertexIndex + 1, end.vertexIndex - 1);
            }
        }

        if (typeof start.vertexIndex !== 'undefined') {
            if (start.vertexIndex === end.edgeIndex) {
                return [];
            } else {
                return _genSequence(start.vertexIndex + 1, end.edgeIndex);
            }
        }

        // TODO: refactor this code so that edge or vertex index overflows are handled automatically
        if (typeof start.edgeIndex !== 'undefined') {
            if (((start.edgeIndex + 1) % _mapRegionEdges.length) === end.vertexIndex) {
                return [];
            } else {
                return _genSequence(start.edgeIndex + 1, end.vertexIndex - 1);
            }
        }

        return [];
    }

    function _genSequence(start, end) {
        let limit = _mapRegionEdges.length;
        if (start >= limit) {
            start %= limit;
        }
        if (end >= limit) {
            end %= limit;
        }
        if (start < 0) {
            start += limit;
        }
        if (end < 0) {
            end += limit;
        }

        const seq = [];
        let i = start;
        while (true) {
            seq.push(_vi[i]);
            if (_vi[i] === end) {
                break;
            }
            i++;
        }
        return seq;
    }
}

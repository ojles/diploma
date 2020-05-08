import QtQuick 2.0
import QtPositioning 5.12
import QtLocation 5.12

MapItemGroup {
    Timer {
        id: betaVectorTimer
        interval: 1000
        repeat: false
        onTriggered: {
            _redraw();
        }
    }

    property var map;
    property var xx;
    property var yy;
    property var origin;
    property bool show;

    readonly property real _scaleValue: 0.1;
    readonly property real _hueScaleValue: 1600;
    readonly property int _vectorDensityInPixels: 60;
    readonly property var _vectors: [];
    readonly property var _mapVectorComponent: Qt.createComponent("MapVector.qml");

    onShowChanged: {
        if (show) {
            betaVectorTimer.start();
        } else {
            betaVectorTimer.stop();
            clear();
        }
    }

    function redraw() {
        if (show) {
            betaVectorTimer.restart();
        }
    }

    function _redraw() {
        // clear old vectors
        clear();

        var {bottomLeft, topRight} = _retrieveMapCorners();
        const vectorDensity = _calculateVectorDensity();

        // make vector coordinate boundary smaller then vector density
        // so that the tip of one vector doesn't reach the origin of the other vector
        const vectorCoordinateBoundary = vectorDensity / 1.5;

        for (let lt0 = bottomLeft.latitude; lt0 <= topRight.latitude; lt0 += vectorDensity) {
            for (let lg0 = bottomLeft.longitude; lg0 <= topRight.longitude; lg0 += vectorDensity) {
                const {lg1, lt1} = _calculateVectorCoordinates(lg0, lt0);
                const hue = _calculateHue(lg1, lt1);
                const {lg2, lt2} = _fitVector(lg1, lt1, vectorCoordinateBoundary);
                _putVectorOnMap(lg0, lt0, lg0 + lg2, lt0 + lt2, hue);
            }
        }
    }

    function clear() {
        while(_vectors.length !== 0) {
            map.removeMapItem(_vectors.pop());
        }
    }

    function _calculateVectorCoordinates(lg0, lt0) {
        return {
            lg1: xx(lg0 - origin.longitude, lt0 - origin.latitude) * _scaleValue,
            lt1: yy(lg0 - origin.longitude, lt0 - origin.latitude) * _scaleValue
        };
    }

    function _calculateHue(lg, lt) {
        let hue = (Math.pow(lg, 2) + Math.pow(lt, 2)) * _hueScaleValue;
        if (hue >= 1) {
            hue = 0.75;
        } else {
            hue *= 0.75;
        }
        return 0.75 - hue;
    }

    function _fitVector(lg1, lt1, boundary) {
        if (lg1 > boundary) {
            lg1 = boundary;
        } else if (lg1 < -boundary) {
            lg1 = -boundary;
        }
        if (lt1 > boundary) {
            lt1 = boundary;
        } else if (lt1 < -boundary) {
            lt1 = -boundary;
        }

        return {
            lg2: lg1,
            lt2: lt1
        };
    }

    function _putVectorOnMap(lg0, lt0, lg1, lt1, hue) {
        let vector = _mapVectorComponent.createObject(map);
        vector.draw(lt0, lg0, lt1, lg1, hue);
        map.addMapItem(vector);
        _vectors.push(vector);
    }

    function _calculateVectorDensity() {
        var first = map.toCoordinate(Qt.point(0, 0));
        var second = map.toCoordinate(Qt.point(0, _vectorDensityInPixels));
        return Math.abs(second.latitude - first.latitude);
    }

    function _retrieveMapCorners() {
        var mapVisibleRegion = QtPositioning.shapeToPolygon(map.visibleRegion);
        return {
            bottomLeft: mapVisibleRegion.coordinateAt(1),
            topRight: mapVisibleRegion.coordinateAt(3)
        }
    }
}

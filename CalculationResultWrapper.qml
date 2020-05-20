import QtQuick 2.0
import QtPositioning 5.12
import QtLocation 5.12

import lnu.oles.CalculationResult 1.0

CalculationResult {
    readonly property double padding: 0.00015;

    property var map;
    // short for region of study
    property var ros;

    property var corners;

    onHasDataChanged: {
        if (!hasData) {
            corners = null;
        }
    }

    function calculate() {
        _updatePositionOnMap();
        updateDimensions(true);
        doCalculate();
    }

    function updateDimensions(force) {
        if (!force && (!hasData || !corners)) {
            return;
        }

        const topLeftPoint = map.fromCoordinate(corners.topLeft.coord, false);
        const bottomRightPoint = map.fromCoordinate(corners.bottomRight.coord, false);
        calcMapItem.sourceItem.width = bottomRightPoint.x - topLeftPoint.x;
        calcMapItem.sourceItem.height = bottomRightPoint.y - topLeftPoint.y;
    }

    function _updatePositionOnMap() {
        corners = {
            topLeft: {
                lat: ros.path[0].latitude,
                lon: ros.path[0].longitude
            },
            bottomRight: {
                lat: ros.path[0].latitude,
                lon: ros.path[0].longitude
            }
        }

        for (let i = 1; i < ros.pathLength(); i++) {
            if (corners.topLeft.lat < ros.path[i].latitude) {
                corners.topLeft.lat = ros.path[i].latitude;
            }
            if (corners.topLeft.lon > ros.path[i].longitude) {
                corners.topLeft.lon = ros.path[i].longitude;
            }
            if (corners.bottomRight.lat > ros.path[i].latitude) {
                corners.bottomRight.lat = ros.path[i].latitude;
            }
            if (corners.bottomRight.lon < ros.path[i].longitude) {
                corners.bottomRight.lon = ros.path[i].longitude;
            }
        }
        corners.topLeft.coord = QtPositioning.coordinate(corners.topLeft.lat - padding,
                                                         corners.topLeft.lon + padding);
        corners.bottomRight.coord = QtPositioning.coordinate(corners.bottomRight.lat + padding,
                                                             corners.bottomRight.lon - padding);
        calcMapItem.coordinate = corners.topLeft.coord;
    }
}

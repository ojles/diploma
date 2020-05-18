import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtPositioning 5.12
import QtLocation 5.12

import lnu.oles.Triangulation 1.0
import lnu.oles.CalculationResult 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    // TODO: change title to Diploma title
    title: qsTr("Hello World")

    // TODO: fix items alignment, maybe make it a widget
    SidePanel {
        id: sidePanel
        width: 215
        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
        }
        readyToCalculate: map.dataReadyForCalculation;
        onRestart: function() {
            map.restart();
        }
        onCalculate: function() {
            map.triggerCalculation();
        }
    }

    // TODO: return all vertices not individually
    Triangulation {
        readonly property var triangles: []

        id: triangulation
        polyline: regionOfStudy
        switches: sidePanel.triangulationSwitches
        onTriangulationFinished: {
            clearFromMap();
            var triangle = Qt.createComponent('Triangle.qml')
            for (var i = 0; i < verticesCount(); i += 3) {
                var triangleObject = triangle.createObject(map);
                triangleObject.addCoordinate(vertexAt(i));
                triangleObject.addCoordinate(vertexAt(i + 1));
                triangleObject.addCoordinate(vertexAt(i + 2));
                map.addMapItem(triangleObject);
                triangles.push(triangleObject);
            }
        }

        function clearFromMap() {
            while (triangles.length !== 0) {
                map.removeMapItem(triangles.pop());
            }
        }
    }

    LvivMap {
        readonly property var omegaPolylineComponent: Qt.createComponent('Omega.qml')
        readonly property var omegaPolylines: []
        property var currentPolyline: regionOfStudy
        property bool dataReadyForCalculation: false

        id: map
        anchors {
            top: parent.top
            left: sidePanel.right
            bottom: parent.bottom
            right: parent.right
        }

        onCenterChanged: {
            betaVectorField.redraw();

            // TODO: cache topLeft and bottomRight
            // FIXME: when ROS is cropped because of a big zoom
            if (sidePanel.readyToCalculate) {
                let topLeft = {
                    lat: regionOfStudy.path[0].latitude,
                    lon: regionOfStudy.path[0].longitude
                };
                let bottomRight = {
                    lat: regionOfStudy.path[0].latitude,
                    lon: regionOfStudy.path[0].longitude
                };
                for (let i = 1; i < regionOfStudy.pathLength(); i++) {
                    if (topLeft.lat < regionOfStudy.path[i].latitude) {
                        topLeft.lat = regionOfStudy.path[i].latitude;
                    }
                    if (topLeft.lon > regionOfStudy.path[i].longitude) {
                        topLeft.lon = regionOfStudy.path[i].longitude;
                    }
                    if (bottomRight.lat > regionOfStudy.path[i].latitude) {
                        bottomRight.lat = regionOfStudy.path[i].latitude;
                    }
                    if (bottomRight.lon < regionOfStudy.path[i].longitude) {
                        bottomRight.lon = regionOfStudy.path[i].longitude;
                    }
                }
                const topLeftC = QtPositioning.coordinate(topLeft.lat, topLeft.lon);
                const bottomRightC = QtPositioning.coordinate(bottomRight.lat, bottomRight.lon);
                const topLeftP = map.fromCoordinate(topLeftC);
                const bottomRightP = map.fromCoordinate(bottomRightC);
                calcMapItem.coordinate = topLeftC;
                calcMapItem.sourceItem.width = bottomRightP.x - topLeftP.x;
                calcMapItem.sourceItem.height = bottomRightP.y - topLeftP.y;
            }
        }

        RegionOfStudy {
            id: regionOfStudy
            line {
                width: 2
                color: "green"
            }
        }

        BoundaryCondition {
            id: boundaryCondition
            mapRegion: regionOfStudy
            line {
                width: 7
                color: "brown"
            }
        }

        BetaVectorField {
            id: betaVectorField
            map: map
            origin: map.lvivCoordinates
            show: sidePanel.showBetaVectorField
            xx: function(x, y) {
                return x;
            }
            yy: function(x, y) {
                return y;
            }
        }

        MapQuickItem {
            id: calcMapItem
            zoomLevel: map.zoomLevel
            sourceItem: Rectangle {
                width: 500
                height: 500
                border {
                    width: 1
                    color: "black"
                }
                color: Qt.rgba(0, 0, 0, 0);
                CalculationResult {
                    id: calculationResult
                    anchors.fill: parent
                    regionOfStudy: regionOfStudy
                    boundaryCondition: boundaryCondition
                    triangulationSwitches: sidePanel.triangulationSwitches
                    calculateBoundaryCondition: sidePanel.calculateBoundaryCondition
                    mu: sidePanel.mu
                    sigma: sidePanel.sigma
                    alpha: sidePanel.alpha
                }
            }
        }

        MouseArea {
            property bool wasLastMouseClickOutsideROS: false

            anchors.fill: parent

            onClicked: {
                let coordinate = map.toCoordinate(Qt.point(mouse.x, mouse.y));
                if (map.currentPolyline === boundaryCondition) {
                    wasLastMouseClickOutsideROS = false;
                    map.currentPolyline.addCoordinateToContour(coordinate);
                    return;
                }
                if (map.currentPolyline !== regionOfStudy && !regionOfStudy.hasInside(coordinate)) {
                    wasLastMouseClickOutsideROS = true;
                    return;
                }
                wasLastMouseClickOutsideROS = false;
                map.currentPolyline.addCoordinate(coordinate);
            }

            onDoubleClicked: {
                mouse.accepted = true

                let minimumVertices = 3;
                if (map.currentPolyline === boundaryCondition) {
                    minimumVertices = 2;
                }

                // the +1 is because even though a double-click was detected
                // the single-click is still processed and those the last point should be ignorred
                if (map.currentPolyline.path.length < minimumVertices + 1) {
                    if (!wasLastMouseClickOutsideROS) {
                        map.currentPolyline.removeCoordinate(map.currentPolyline.path.length - 1)
                    }
                    return;
                }

                if (map.currentPolyline === boundaryCondition) {
                    map.currentPolyline = map.omegaPolylineComponent.createObject(map);
                    map.addMapItem(map.currentPolyline);
                    map.omegaPolylines.push(map.currentPolyline);
                    map.dataReadyForCalculation = true;
                    return;
                }

                if (wasLastMouseClickOutsideROS) {
                    map.currentPolyline.joinEnds();
                } else {
                    map.currentPolyline.joinEndsIgnoringLastPoint();
                }
                if (map.currentPolyline === regionOfStudy) {
                    map.currentPolyline = boundaryCondition;
                } else {
                    map.currentPolyline = map.omegaPolylineComponent.createObject(map);
                    map.addMapItem(map.currentPolyline);
                    map.omegaPolylines.push(map.currentPolyline);
                }
            }
        }

        function restart() {
            dataReadyForCalculation = false;

            regionOfStudy.path = [];
            boundaryCondition.clearFromMap();
            triangulation.clearFromMap();
            while (omegaPolylines.length !== 0) {
                removeMapItem(omegaPolylines.pop());
            }

            currentPolyline = regionOfStudy;
        }

        function triggerCalculation() {
            if (!dataReadyForCalculation) {
                return;
            }
            triangulation.doTriangulate();
            calculationResult.doCalculate();
        }
    }
}

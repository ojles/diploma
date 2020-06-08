import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.14
import QtPositioning 5.12
import QtLocation 5.12

ApplicationWindow {
    visible: true
    width: 1024
    height: 720
    // TODO: change title to Diploma title
    title: qsTr("Hello World")

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // TODO: fix items alignment, maybe make it a widget
        ControlPanel {
            id: controlPanel
            readyToCalculate: map.dataReadyForCalculation;

            SplitView.preferredWidth: 215
            SplitView.minimumWidth: 215
            SplitView.maximumWidth: 300

            onRestart: function() {
                map.restart();
            }
            onCalculate: function() {
                map.triggerCalculation();
            }
        }

        SplitView {
            orientation: Qt.Vertical

            SplitView.fillWidth: true

            LvivMap {
                id: map

                readonly property var omegaPolylineComponent: Qt.createComponent('Omega.qml')
                readonly property var polutionSourceMarkerComponent: Qt.createComponent('PolutionSourceMapMarker.qml')
                readonly property var omegaPolylines: []
                readonly property var circles: []
                readonly property int circlesAmount: 15
                property var currentPolyline: regionOfStudy
                property bool dataReadyForCalculation: false

                SplitView.fillHeight: true

                Component.onCompleted: {
                    for (var i = 0; i < circlesAmount; i++) {
                        circles[i] = polutionSourceMarkerComponent.createObject(map);
                        map.addMapItem(circles[i]);
                    }
                }

                onCenterChanged: {
                    betaVectorField.redraw();
                    calculationResult.updateDimensions();
                }

                RegionOfStudy {
                    id: regionOfStudy
                    line {
                        width: 2
                        color: "green"
                    }
                }

                Gamma2 {
                    id: gamma2
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
                    show: controlPanel.showBetaVectorField
                    xx: controlPanel.windX;
                    yy: controlPanel.windY;
                }

                MapQuickItem {
                    id: calcMapItem
                    zoomLevel: map.zoomLevel
                    sourceItem: Rectangle {
                        color: Qt.rgba(0, 0, 0, 0);
                        CalculationResultWrapper {
                            id: calculationResult
                            anchors.fill: parent
                            map: map
                            regionOfStudy: regionOfStudy
                            gamma2: gamma2
                            conservacyAreas: map.omegaPolylines
                            triMinAngle: controlPanel.triMinAngle
                            triMaxArea: controlPanel.triMaxArea
                            showTriangulation: controlPanel.showTriangulation
                            calculateGamma2: controlPanel.calculateGamma2
                            windX: controlPanel.windX
                            windY: controlPanel.windY
                            mu: controlPanel.mu
                            sigma: controlPanel.sigma
                            alpha: controlPanel.alpha
                        }
                    }
                }

                MouseArea {
                    property bool lastMouseClickOutsideROS: false

                    anchors.fill: parent

                    onClicked: {
                        let coordinate = map.toCoordinate(Qt.point(mouse.x, mouse.y));
                        if (map.currentPolyline === gamma2) {
                            lastMouseClickOutsideROS = false;
                            map.currentPolyline.addCoordinateToContour(coordinate);
                            return;
                        }
                        if (map.currentPolyline !== regionOfStudy && !regionOfStudy.hasInside(coordinate)) {
                            lastMouseClickOutsideROS = true;
                            return;
                        }
                        lastMouseClickOutsideROS = false;
                        map.currentPolyline.addCoordinate(coordinate);
                    }

                    onDoubleClicked: {
                        mouse.accepted = true

                        let minimumVertices = 3;
                        if (map.currentPolyline === gamma2) {
                            minimumVertices = 2;
                        }

                        // the +1 is because even though a double-click was detected
                        // the single-click is still processed and those the last point should be ignored
                        if (map.currentPolyline.path.length < minimumVertices + 1) {
                            if (!lastMouseClickOutsideROS) {
                                map.currentPolyline.removeCoordinate(map.currentPolyline.path.length - 1)
                            }
                            return;
                        }

                        if (map.currentPolyline === gamma2) {
                            map.currentPolyline = map.omegaPolylineComponent.createObject(map);
                            map.addMapItem(map.currentPolyline);
                            map.omegaPolylines.push(map.currentPolyline);
                            return;
                        }

                        if (lastMouseClickOutsideROS) {
                            map.currentPolyline.joinEnds();
                        } else {
                            map.currentPolyline.joinEndsIgnoringLastPoint();
                        }

                        if (map.currentPolyline === regionOfStudy) {
                            map.currentPolyline = gamma2;
                        } else {
                            map.currentPolyline = map.omegaPolylineComponent.createObject(map);
                            map.addMapItem(map.currentPolyline);
                            map.omegaPolylines.push(map.currentPolyline);
                            map.dataReadyForCalculation = true;
                        }
                    }
                }

                function restart() {
                    dataReadyForCalculation = false;

                    regionOfStudy.path = [];
                    gamma2.clearFromMap();
                    while (omegaPolylines.length !== 0) {
                        removeMapItem(omegaPolylines.pop());
                    }

                    currentPolyline = regionOfStudy;
                    calculationResult.clear();

                    circles.forEach(circle => circle.hide());
                }

                function triggerCalculation() {
                    if (!dataReadyForCalculation) {
                        return;
                    }
                    var minPoints = calculationResult.calculate();
                    for (var i = 0; i < Math.min(minPoints.length, circlesAmount); i++) {
                        circles[i].show(minPoints[i]);
                    }
                }
            }

            MessageBox {
                SplitView.preferredHeight: 140
                SplitView.minimumHeight: 120
                SplitView.maximumHeight: 500
            }
        }
    }
}

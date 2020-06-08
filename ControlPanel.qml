import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.14

// TODO: limit triangulation to certain number of triangles
Rectangle {
    property bool readyToCalculate;
    property var onRestart;
    property var onCalculate;

    property bool showBetaVectorField: showBetaVectorFieldCheckbox.checked;
    property bool showTriangulation: showTriangulationCheckbox.checked;
    property bool calculateGamma2: calculateGamma2CheckBox.checked;
    property string windX: windDirectionX.text;
    property string windY: windDirectionY.text;
    property string triMinAngle: minAngleTextField.text
    property string triMaxArea: maxAreaTextField.text
    property string mu: muTextField.text
    property string sigma: sigmaTextField.text
    property string alpha: alphaTextField.text

    color: '#f2f2f2'

    Flickable {
        anchors.fill: parent
        contentHeight: cpanel.height
        contentWidth: parent.width
        flickableDirection: Flickable.VerticalFlick
        flickDeceleration: 5000
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            snapMode: ScrollBar.SnapAlways
        }

        ColumnLayout {
            id: cpanel
            width: parent.parent.width - 6

            GroupBox {
                title: qsTr("Wind direction")
                padding: 9
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.topMargin: 8
                ColumnLayout {
                    anchors.fill: parent
                    id: windDirectionGroupRootLayout
                    CheckBox {
                        id: showBetaVectorFieldCheckbox
                        text: qsTr("Show wind direction")
                    }
                    RowLayout {
                        ColumnLayout {
                            Text {
                                text: "x:"
                                Layout.leftMargin: 5
                            }
                            TextField {
                                id: windDirectionX
                                text: "5"
                                placeholderText: "x"
                                implicitWidth: windDirectionGroupRootLayout.width / 2 - 3
                                validator: DoubleValidator {}
                            }
                        }
                        ColumnLayout {
                            Text {
                                text: "y:"
                                Layout.leftMargin: 5
                            }
                            TextField {
                                id: windDirectionY
                                text: "3"
                                placeholderText: "y"
                                implicitWidth: windDirectionGroupRootLayout.width / 2 - 3
                                validator: DoubleValidator {}
                            }
                        }
                    }
                }
            }

            GroupBox {
                title: qsTr("Triangulation")
                padding: 9
                leftPadding: 3
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.topMargin: 8
                ColumnLayout {
                    anchors.fill: parent
                    id: triangulationGroupRootLayout
                    CheckBox {
                        id: showTriangulationCheckbox
                        text: qsTr("Show triangulation")
                        Layout.leftMargin: 3
                    }
                    Text {
                        text: qsTr("Min angle:")
                        Layout.leftMargin: 11
                    }
                    TextField {
                        id: minAngleTextField
                        text: "25"
                        placeholderText: qsTr("Min angle")
                        Layout.leftMargin: 6
                        Layout.fillWidth: true
                        validator: DoubleValidator {}
                    }
                    Text {
                        text: qsTr("Max area:")
                        Layout.leftMargin: 11
                    }
                    TextField {
                        id: maxAreaTextField
                        text: "0.00005"
                        placeholderText: qsTr("Max area")
                        Layout.leftMargin: 6
                        Layout.fillWidth: true
                        validator: DoubleValidator {}
                    }
                }
            }

            CheckBox {
                id: calculateGamma2CheckBox
                Layout.topMargin: 15
                text: qsTr("Second boundary condition")
            }

            Text {
                text: qsTr("Mu:")
                Layout.leftMargin: 6
            }
            TextField {
                id: muTextField
                text: "0.1"
                Layout.leftMargin: 6
                Layout.fillWidth: true
                validator: DoubleValidator {}
            }

            Text {
                text: qsTr("Sigma:")
                Layout.leftMargin: 6
            }
            TextField {
                id: sigmaTextField
                text: "1.0"
                Layout.leftMargin: 6
                Layout.fillWidth: true
                validator: DoubleValidator {}
            }

            Text {
                text: qsTr("Alpha:")
                Layout.leftMargin: 6
            }
            TextField {
                id: alphaTextField
                text: "1.0"
                Layout.leftMargin: 6
                Layout.fillWidth: true
                validator: DoubleValidator {}
            }

            Button {
                text: qsTr("Restart")
                Layout.leftMargin: 6
                Layout.topMargin: 15
                Layout.fillWidth: true
                onClicked: {
                    onRestart();
                }
            }

            Button {
                text: qsTr("Calculate")
                enabled: readyToCalculate
                Layout.leftMargin: 6
                Layout.fillWidth: true
                onClicked: {
                    onCalculate();
                }
            }
        }
    }
}

import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// TODO: limit triangulation to certain number of triangles
Rectangle {
    property bool readyToCalculate;
    property var onRestart;
    property var onCalculate;

    property bool showBetaVectorField: showBetaVectorFieldCheckbox.checked;
    property bool calculateBoundaryCondition: calculateBoundaryConditionCheckBox.checked;
    property string triangulationSwitches: triangulationSwitchesTextField.text
    property string mu: muTextField.text
    property string sigma: sigmaTextField.text
    property string alpha: alphaTextField.text

    color: '#f2f2f2'
    ScrollView {
        anchors.fill: parent
        ColumnLayout {
            anchors.fill: parent
            CheckBox {
                id: showBetaVectorFieldCheckbox
                text: qsTr("Show beta vector field")
            }
            TextField {
                id: triangulationSwitchesTextField
                text: "q30a.00005"
                placeholderText: qsTr("Triangulation Switches")
                Layout.leftMargin: 6
                Layout.fillWidth: true
            }
            CheckBox {
                id: calculateBoundaryConditionCheckBox
                Layout.topMargin: 15
                text: "Enable boundary condition"
            }
            Text {
                text: "Mu:"
                Layout.leftMargin: 6
            }
            TextField {
                id: muTextField
                text: "0.1"
                Layout.leftMargin: 6
                validator: DoubleValidator {}
            }
            Text {
                text: "Sigma:"
                Layout.leftMargin: 6
            }
            TextField {
                id: sigmaTextField
                text: "1.0"
                Layout.leftMargin: 6
                validator: DoubleValidator {}
            }
            Text {
                text: "Alpha:"
                Layout.leftMargin: 6
            }
            TextField {
                id: alphaTextField
                text: "1.0"
                Layout.leftMargin: 6
                validator: DoubleValidator {}
            }
            Button {
                text: "Restart"
                Layout.leftMargin: 6
                Layout.topMargin: 15
                Layout.fillWidth: true
                onClicked: {
                    onRestart();
                }
            }
            Button {
                text: "Calculate"
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

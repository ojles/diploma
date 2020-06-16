import QtQuick 2.12
import QtQuick.Controls 2.14

Button {
    property var options;
    property int defaultOption: 0;
    property int currentOption: defaultOption;

    text: options[currentOption];

    function toggle() {
        currentOption = (currentOption + 1) % options.length;
        text = options[currentOption];
    }
}

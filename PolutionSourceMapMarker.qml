import QtQuick 2.0
import QtPositioning 5.12
import QtLocation 5.12

MapCircle {
    color: "red"
    radius: 0
    border {
        width: 0
    }

    function show(coorinate) {
        radius = 200;
        center = coorinate;
    }

    function hide() {
        radius = 0;
    }
}

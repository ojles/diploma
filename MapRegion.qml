import QtQuick 2.0
import QtPositioning 5.12
import QtLocation 5.12

MapPolyline {
    line {
        width: 2
    }

    function joinEnds() {
        addCoordinate(path[0]);
    }

    function joinEndsIgnoringLastPoint() {
        replaceCoordinate(path.length - 1, path[0]);
    }
}

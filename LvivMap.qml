import QtQuick 2.0
import QtPositioning 5.12
import QtLocation 5.12

Map {
    readonly property var lvivCoordinates: QtPositioning.coordinate(49.841952, 24.0315921);

    center: lvivCoordinates
    zoomLevel: 12
    plugin: Plugin {
        name: "osm"
    }
}

import QtQuick 2.0
import QtPositioning 5.12
import QtLocation 5.12

MapPolygon {
    readonly property real arrowWidthToVectorLength: 0.15;

    border {
        width: 1
        color: "gray"
    }

    function draw(lt1, lg1, lt2, lg2, hue) {
        // calculate vector coordinates
        const a = lg2 - lg1;
        const b = lt2 - lt1;

        // move starting point left and right perpendicular to the vector
        // those create an arrow shape
        const lg0 = lg1 + b * arrowWidthToVectorLength;
        const lt0 = lt1 - a * arrowWidthToVectorLength;
        lg1 = lg1 - b * arrowWidthToVectorLength;
        lt1 = lt1 + a * arrowWidthToVectorLength;

        // draw arrow
        this.color = Qt.hsla(hue, 1, 0.5, 1);
        addCoordinate(QtPositioning.coordinate(lt0, lg0));
        addCoordinate(QtPositioning.coordinate(lt1, lg1));
        addCoordinate(QtPositioning.coordinate(lt2, lg2));
    }
}

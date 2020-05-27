import QtQuick 2.14
import QtQuick.Controls 2.14

Flickable {
    id: flick
    contentWidth: textEdit.paintedWidth
    contentHeight: textEdit.paintedHeight
    flickDeceleration: 7000
    flickableDirection: Flickable.VerticalFlick
    clip: true
    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
        snapMode: ScrollBar.SnapAlways
    }

    function ensureVisible(r) {
        if (contentX >= r.x) {
            contentX = r.x;
        }
        else if (contentX+width <= r.x+r.width) {
            contentX = r.x+r.width-width;
        }

        if (contentY >= r.y) {
            contentY = r.y;
        }
        else if (contentY+height <= r.y+r.height) {
            contentY = r.y+r.height-height;
        }
    }

    TextEdit {
        objectName: "messageBoxTextEdit"
        id: textEdit
        textFormat: TextEdit.RichText
        width: flick.width
        height: flick.height
        focus: false
        readOnly: true
        wrapMode: TextEdit.Wrap
        leftPadding: 7
        rightPadding: 7
        bottomPadding: 30
        font.pixelSize: 13
        onCursorRectangleChanged: {
            flick.ensureVisible(cursorRectangle)
        }
        onTextChanged: {
            cursorPosition = text.length
        }
    }
  }


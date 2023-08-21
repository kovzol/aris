import QtQuick 2.15
import QtQuick.Controls 2.5

ToolButton{

    property string txt: "Symbol"

    height: thefont.pointSize + 15

    hoverEnabled: true
    ToolTip.visible: hovered
    ToolTip.text: "Operation Name"

    focusPolicy: Qt.NoFocus         // Text is passed to focused TextArea, so the button musn't steal focus

    onClicked: {
        if (isTextField(activeFocusItem))
            activeFocusItem.insert(activeFocusItem.cursorPosition, txt)

    }

    Text{
        text: txt
        font: thefont
        color: darkMode? "#BB86FC" : "black"
        anchors.centerIn: parent
        fontSizeMode: Text.HorizontalFit
    }

}


import QtQuick 2.15
import QtQuick.Controls 2.5

ToolButton{

    text: "Symbol"

    hoverEnabled: true
    ToolTip.visible: hovered
    ToolTip.text: "Operation Name"

    focusPolicy: Qt.NoFocus         // Text is passed to focused TextArea, so the button musn't steal focus

    onClicked: {

        if (isTextField(activeFocusItem))
            activeFocusItem.insert(activeFocusItem.cursorPosition, text)

    }

}


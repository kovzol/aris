import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import GNU.Aris

Item {
    Layout.fillWidth: true
    implicitHeight: textEdit.implicitHeight

    TextArea {
        id: textEdit
        anchors.fill: parent
        placeholderText: "Type here..."
        font.family: "CMU Serif"
        font.pointSize: 29
        textFormat: TextEdit.PlainText
        KeyNavigation.priority: KeyNavigation.BeforeItem
        KeyNavigation.tab: nextItemInFocusChain()
        onEditingFinished: highlighter.resetCursorPosition()
        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Greater && text.charAt(cursorPosition - 1) === "-") {
                event.accepted = true;
                remove(cursorPosition - 1, cursorPosition);
                insert(cursorPosition, "→");
            }
        }
        ProofLineHighlighter {
            id: highlighter
            document: textEdit.textDocument
            cursorPosition: textEdit.cursorPosition
        }
    }

    MouseArea {
        id: mouseArea
        acceptedButtons: Qt.NoButton
        cursorShape: Qt.IBeamCursor
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: false
        propagateComposedEvents: true
        onEntered: highlighter.setMousePosition(-1.0, -1.0)
        onExited: {
            highlighter.setMousePosition(-1.0, -1.0)
            symbolTooltip.close()
            hoverTimer.stop();
        }
        onPositionChanged: (mouse) => {
            if (textEdit.length > 0) {
                let lastCharRect = textEdit.positionToRectangle(textEdit.length);
                let endOfTextBoundary = lastCharRect.x + lastCharRect.width;
                if (mouse.x > endOfTextBoundary) {
                    highlighter.setMousePosition(-1.0, -1.0)
                    return;
                }
            }
            hoverTimer.stop();
            let pos = highlighter.setMousePosition(mouse.x, mouse.y)
            if (pos >= 0 && textEdit.text[pos] === "→")
            {
                hoverTimer.id = "→"
                hoverTimer.toolTipX = Qt.binding(()=>mapToGlobal(mouseX, mouseY).x)
                hoverTimer.toolTipY = Qt.binding(()=>mapToGlobal(mouseX, mouseY).y-10)
                hoverTimer.text = "Implication: If, then."
                hoverTimer.start();
            }
            else
            {
                symbolTooltip.close()
            }
        }
    }
}

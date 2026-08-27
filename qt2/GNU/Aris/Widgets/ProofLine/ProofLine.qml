import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import GNU.Aris.Widgets

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
        HighlighterBridge {
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
        onEntered: highlighter.unhighlight()
        onExited: {
            highlighter.unhighlight()
            symbolTooltip.close()
            hoverTimer.stop();
        }
        onPositionChanged: (mouse) => {
            if (textEdit.length === 0)
                return
            let lastCharRect = textEdit.positionToRectangle(textEdit.length);
            let endOfTextBoundary = lastCharRect.x + lastCharRect.width;
            if (mouse.x > endOfTextBoundary) {
                highlighter.unhighlight()
                return;
            }
            hoverTimer.stop();
            let pos = highlighter.hitTest(mouse.x, mouse.y)
            highlighter.unhighlight()
            if (pos >= 0 && textEdit.text[pos] === "→")
            {
                highlighter.highlight(pos, 1)
                hoverTimer.id = "→"
                hoverTimer.toolTipX = Qt.binding(()=>mapToGlobal(mouseX, mouseY).x)
                hoverTimer.toolTipY = Qt.binding(()=>mapToGlobal(mouseX, mouseY).y-10)
                hoverTimer.text = "Implication: If, then."
                hoverTimer.start()
            }
            else
            {
                symbolTooltip.close()
            }
        }
    }
}

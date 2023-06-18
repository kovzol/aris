import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Dialogs

Window {
    id: rootID
    width: 1200
    height: 700
    visible: true
    title: qsTr("GNU Aris")

    Button{
        id: burgerMenu
        height: keyboardID.width
        width: keyboardID.width

        hoverEnabled: true
        ToolTip.visible: hovered
        ToolTip.text: "Menu"

        Text{
            anchors.centerIn: parent
            text: "\u2630"
            minimumPointSize: 20
        }

        onClicked: {
            menuOptions.open()
        }
    }

    function isTextArea(item){
        return item instanceof TextArea
    }

    Frame{
        id: keyboardID
        anchors.verticalCenter: parent.verticalCenter

        ToolBar{

            Column{
                id: keyColID

                ToolButton{
                    id: conjunctionButton
                    text: "\u2227"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Conjunction"
                    focusPolicy: Qt.NoFocus

                    onClicked: {
                        console.log("hi");
                        console.log(activeFocusItem);
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)


                    }

                }

                ToolButton{
                    id: disjunctionButton
                    text: "\u2228"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Disjunction"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: negationButton
                    text: "\u00ac"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Negation"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: implicationButton
                    text: "\u2192"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Implication"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: biconditionalButton
                    text: "\u2194"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Bi-conditional"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: forAllButton
                    text: "\u2200"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "For all"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: thereExistsButton
                    text: "\u2203"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "There exists"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: tautologyButton
                    text: "\u22a4"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Tautology"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: contradictionButton
                    text: "\u22a5"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Contradiction"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: belongsToButton
                    text: "\u2208"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Belongs To"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: nullButton
                    text: "\u2205"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Null"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }
            }
        }
    }

    Drawer{
        id: menuOptions
        width: drawerToolBar.implicitWidth
        height: rootID.height
        interactive: true

        ToolBar{
            id: drawerToolBar
            anchors.fill: parent

            ColumnLayout{

                ToolButton{
                    text: qsTr("Open")
                    icon.source: "/assets/folder.png"
                    onClicked: {
                        fileDialogID.open()
                    }
                }

                ToolButton{
                    text: qsTr("Save")
                    icon.source: "/assets/save.png"
                }

                ToolButton{
                    text: qsTr("Save As")
                    icon.source: "/assets/saveas.png"

                    onClicked: {
                        saveAsID.open()
                    }
                }

                ToolButton{
                    text: qsTr("Export To LaTeX")
                    icon.source: "/assets/export.png"
                }

                ToolSeparator{
                    orientation: Qt.Horizontal
                }

                ToolButton{
                    text: qsTr("Toggle Goals")
                    icon.source: "/assets/goal.png"

                    onClicked: {
                        goalDialogID.open()
                    }
                }

                ToolButton{
                    text: qsTr("Toggle Boolean Mode")
                    icon.source: "/assets/boolean.png"
                }

                ToolButton{
                    text: qsTr("Import Proof")
                    icon.source: "/assets/import.png"
                }

                ToolSeparator{
                    orientation: Qt.Horizontal
                }

                ToolButton{
                    text: qsTr("Font")
                    icon.source: "/assets/font.png"

                    onClicked: {
                        fontDialogID.open()
                    }
                }

                ToolSeparator{
                    orientation: Qt.Horizontal
                }

                ToolButton{
                    text: qsTr("Help")
                    icon.source: "/assets/help.png"
                    onClicked: {
                        Qt.openUrlExternally("https://www.gnu.org/software/aris/manual/aris.pdf")
                    }
                }

                ToolButton{
                    text: qsTr("About")
                    icon.source: "/assets/about.png"
                    onClicked: {
                        Qt.openUrlExternally("https://www.gnu.org/software/aris/")
                    }
                }

            }

        }
    }

    FileDialog{
        id: fileDialogID
        title: "Choose the proof file"
        //selectFolder: false
    }

    FileDialog{
        id: saveAsID
        title: "Save As"
        fileMode: FileDialog.SaveFile
    }

    FontDialog{
        id: fontDialogID
        title: "Choose Font"
    }

    Dialog{
        id: goalDialogID
        title: "Goals"
        width: parent.width/2
        height: parent.height/2
        x: (parent.width - width)/2
        y: (parent.height - height)/2
        parent: Overlay.overlay
        focus: true
        modal: true
        standardButtons: Dialog.Apply | Dialog.Cancel

        ColumnLayout{
            id: goalAreaID
            anchors.fill: parent

            ListView{
                model: goalDataID
                delegate: goalLineID
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10
                ScrollBar.vertical: ScrollBar{}
            }

        }

    }

    ListModel{
        id: goalDataID

        ListElement{
            line: 1
        }
    }

    Component{
        id: goalLineID

        RowLayout{
            spacing: 10
            width: parent.width
            Layout.fillWidth: true

            Rectangle{
                id: goalLineNumID
                height: goalTextID
                width: height + 10

                Text{
                    anchors.centerIn: parent
                    font.italic: true
                    text: line
                }
            }

            TextField{
                id: goalTextID
                height: font.pointSize + 10
                width: 200
                Layout.fillWidth: true
                background: Rectangle{
                    color: "lightgrey"
                }
                wrapMode: TextArea.Wrap
                placeholderText: qsTr("Start Typing here...")

            }

            Button{
                id: goalPlusID
                height: goalTextID.height

                Text{
                    anchors.centerIn: parent
                    text: "+ / \u2013"
                }

                onClicked: {
                    goalOptionsID.open()
                }

                Menu{
                    id: goalOptionsID
                    Action{
                        text: "Add Goal"
                        onTriggered: {
                            goalDataID.insert(index+1,{"line":index + 2});
                            updateLines(goalDataID)
                        }
                    }
                    Action{
                        text: "Remove Goal"
                        onTriggered: {
                            if (goalDataID.count > 1){
                                goalDataID.remove(index)
                                updateLines(goalDataID)
                            }
                            else
                                console.log("Invalid Operation")
                        }
                    }
                    Action{
                        text: "Check Line"
                        onTriggered: {
                            console.log("Coming Soon")
                        }
                    }
                }
            }
        }
    }

    ProofArea{}
    function updateLines(item){

        for (var i = 0; i < item.count; i++){
            item.set(i,{"line":i+1})
        }

    }

}

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

    // Updates Line Numbers after a add/delete operation in the Proof Area

    function updateLines(item){

        for (var i = 0; i < item.count; i++){
            item.set(i,{"line":i+1})
        }

    }

    // Checks if the item is a TextArea QML Type

    function isTextField(item){
        return item instanceof TextField
    }

    // Burger Button

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

    // On-Screen Keyboard

    Frame{
        id: keyboardID

        anchors.verticalCenter: parent.verticalCenter

        KeyGroup{}
    }

    // Drawer associated with the Burger Button

    Drawer{
        id: menuOptions

        width: drawerToolBar.implicitWidth
        height: rootID.height
        interactive: true

        DrawerTools{
            id: drawerToolBar
        }
    }

    // Dialogs associated with the DrawerTools

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
                delegate: GoalLine{}

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

    ProofArea{}

}

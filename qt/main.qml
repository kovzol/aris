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

    // Function to check if the item is a TextField QML Type

    function isTextField(item){
        return item instanceof TextField
    }

    function fillWrap(){
        for (var i = 0; i < proofData.pmLines.count; i++){
            var item_i = proofData.pmLines.get(i);
            Wrapper.textAppend(item_i.text);
            Wrapper.ruleAppend(item_i.type);
            Wrapper.depthAppend(item_i.indent);
            for (var ii = 0; ii < item_i.refs.count; ii++){
//                console.log(proofDataID.get(i).refs.get(ii).num);
                Wrapper.refsAppend(i,item_i.refs.get(ii).num);
            }
        }
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
        defaultSuffix: "tle"
        onAccepted: {
            Wrapper.clearData();
            fillWrap();
            Wrapper.computeIndices();
            Wrapper.computeRules();
            Wrapper.saveProof(selectedFile)
        }
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

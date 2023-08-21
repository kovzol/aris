import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Dialogs
import proof.model 1.0
import goal.model 1.0

ApplicationWindow {
    id: rootID

    property font thefont: rootID.font
    property bool isExtFile: false  // If the file is opened from an external file
    property bool darkMode: false

    // Function to check if the item is a TextField QML Type
    function isTextField(item){
        return item instanceof TextField
    }

    width: 1200
    height: 700
    visible: true
    title: qsTr("GNU Aris")
    font: thefont
    color: darkMode ? "#121212": "white"

    // Footer to display error messages
    footer: Label{
        height: statusID.implicitHeight
        visible: !(cConnector.evalText === "Correct!" || cConnector.evalText === "Evaluate Proof")

        Text{
            id: statusID
            text: cConnector.evalText
            color: darkMode?"#CF6679":"red"
        }
    }


    // Burger Button
    Button{
        id: burgerMenu

        height: keyboardID.width
        width: keyboardID.width
        palette{
            button: darkMode? "#121212": "white"
        }

        hoverEnabled: true
        ToolTip.visible: hovered
        ToolTip.text: "Menu"

        onClicked: {
            menuOptions.open()
        }

        Text{
            anchors.centerIn: parent
            color: darkMode? "#BB86FC": "black"
            text: "\u2630"
            minimumPointSize: 20
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
        nameFilters: ["Aris files (*.tle)"]
        fileMode: FileDialog.OpenFile
        defaultSuffix: "tle"
        onAccepted: cConnector.openProof(selectedFile,theData,theGoals)

    }

    FileDialog{
        id: saveAsID

        nameFilters: ["Aris files (*.tle)"]
        title: "Save As"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "tle"
        onAccepted: cConnector.saveProof(selectedFile,theData,theGoals)

    }

    FileDialog{
        id: latexID

        nameFilters: ["Latex files (*.tex)"]
        title: "Save As"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "tex"
        onAccepted: auxConnector.latex(selectedFile,theData,cConnector)

    }

    FileDialog{
        id: importID

        nameFilters: ["Aris files (*.tle)"]
        title: "Import Proof"
        fileMode: FileDialog.OpenFile
        defaultSuffix: "tle"
        onAccepted: auxConnector.importProof(selectedFile,theData,cConnector,proofModel)

    }

    FontDialog{
        id: fontDialogID

        title: "Choose Font"
        onAccepted: thefont = currentFont;

    }

    Dialog{
        id: goalDialogID

        title: "Goals"
        width: parent.width/2
        height: parent.height/2
        x: (parent.width - width)/2
        y: (parent.height - height)/2

        background: Rectangle{
            anchors.fill: parent
            color: darkMode? "#1F1B24": "white"
            opacity: 0.6
        }

        palette {
            button: darkMode? "#1F1A24" : "white"
            buttonText: darkMode? "white": "black"
            text: darkMode? "white": "black"
            window: darkMode? "#1F1B24": "white"
        }

        parent: Overlay.overlay
        focus: true
        //modal: true
        closePolicy: Popup.CloseOnEscape    // To allow keyboard access
        standardButtons: Dialog.Ok

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

    GoalModel{
        id: goalDataID
        glines: theGoals
    }

    ProofModel{
        id: proofModel
        lines: theData
    }

    ProofArea{}

}

// TODO:
//  1)  Implement premiseIndex to keep track of latest premise and enter any new premise after it
//  2)  Dark Mode for Drawer

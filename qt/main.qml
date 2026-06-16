
/* The main window.

   Copyright (C) 2023 Saksham Attri.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
    property bool isExtFile: false // If the file is opened from an external file
    property bool darkMode: false
    property string filename: "Untitled"
    property bool fileExists: isExtFile
    property bool fileModified: false
    property bool computePremise: false // set to true if Open or Import Proof are used
    property int importMode: 0

    //  Zoom infrastructure (Proof Line section only) 
    property real zoomFactor: 1.0          // single source of truth
    readonly property real zoomMin:  0.3
    readonly property real zoomMax:  2.0
    readonly property real zoomStep: 0.1

    // Convenience helpers consumed by ProofArea.qml
    readonly property real scaledFontSize: Math.round(thefont.pointSize * zoomFactor)
    readonly property real scaledSpacing:  Math.round(10 * zoomFactor)

    function zoomIn()    { zoomFactor = Math.min(zoomMax, Math.round((zoomFactor + zoomStep) * 10) / 10) }
    function zoomOut()   { zoomFactor = Math.max(zoomMin, Math.round((zoomFactor - zoomStep) * 10) / 10) }
    function zoomReset() { zoomFactor = 1.0 }
   

    // premiseCount is now a C++ Q_PROPERTY on proofModel.
    // Use proofModel.recomputePremiseCount() after opening/importing a file.

    // Function to check if the item is a TextField QML Type
    function isTextField(item) {
        return item instanceof TextField
    }

    function resetMainWindow() {
        theData.reset()
        proofModel.updateLines()

        theGoals.reset()

        proofModel.premiseCount = 1
        computePremise = false
        isExtFile = false
        fileExists = false
        fileModified = false
        filename = "Untitled"
        cConnector.evalText = "Evaluate Proof"
        proofModel.clearErrors()

        if (goalDialogID.opened)
            goalDialogID.close()

        proofID.resetViewState()
        menuOptions.close()
    }

    function requestResetWindow() {
        if (fileModified)
            resetDialogID.open()
        else
            resetMainWindow()
    }

    function startImportFlow(mode) {
        importMode = mode
        cConnector.evalText = "Evaluate Proof"
        proofModel.clearErrors()
        isExtFile = true
        computePremise = true

        importBehaviorID.close()
        menuOptions.close()

        if (Qt.platform.os === "wasm") {
            auxConnector.wasmImportProofWithMode(theData, cConnector, proofModel, importMode)
        } else {
            // Defer the native dialog so the QML dialog can close smoothly first
            Qt.callLater(function() {
                importID.open()
            })
        }
    }

    width: 1200
    height: 700
    visible: true
    title: qsTr(filename.slice(filename.lastIndexOf(
                                   "/") + 1) + " | " + "GNU Aris")
    font: thefont
    color: darkMode ? "#121212" : "white"

    // Automatic layout mirroring for RTL languages
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    // Footer to display error messages
    footer: Label {
        height: statusID.implicitHeight + 10
        leftPadding: 12
        visible: !(cConnector.evalText === "Correct!"
                   || cConnector.evalText === "Evaluate Proof")

        Text {
            id: statusID
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 12
            text: cConnector.evalText === "Errors found"
                  ? "⚠ Errors found — see inline details above each failing line."
                  : cConnector.evalText
            color: darkMode ? "#CF6679" : "red"
            font.pointSize: thefont.pointSize + 1
            font.bold: true
        }
    }

    Component.onCompleted: () => {
        qsTr("QT_LAYOUT_DIRECTION", "QGuiApplication");
    }

    onClosing: function(close) {
        if (fileModified) {
            unsavedChangesID.open()
            close.accepted = false;
        }
    }

    // Burger Button
    Button {
        id: burgerMenu

        anchors.left: parent.left
        anchors.top: parent.top
        height: keyboardID.width
        width: keyboardID.width
        palette {
            button: darkMode ? "#121212" : "white"
        }

        hoverEnabled: true
        ToolTip.visible: hovered
        ToolTip.text: "Menu"

        onClicked: menuOptions.open()

        Text {
            anchors.centerIn: parent
            color: darkMode ? "#BB86FC" : "black"
            text: "\u2630"
            minimumPointSize: 20
        }
    }

    // On-Screen Keyboard
    Frame {
        id: keyboardID

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        KeyGroup {}
    }

    // Drawer associated with the Burger Button
    Drawer {
        id: menuOptions

        width: drawerToolBar.implicitWidth
        height: rootID.height
        interactive: true
        edge: Qt.application.layoutDirection === Qt.LeftToRight ? Qt.LeftEdge : Qt.RightEdge

        DrawerTools {
            id: drawerToolBar
        }
    }

    Connections {
        target: auxConnector

        function onImportFinished(success) {
            if (success)
                fileModified = true
        }
    }

    Connections {
        target: cConnector

        function onSmartPasteStarted() {
            // Called at the very start of smartPaste() — mark proof as
            // coming from an external source so the UI renders ref numbers.
            isExtFile = true
            computePremise = true
            cConnector.evalText = "Evaluate Proof"
            proofModel.clearErrors()
        }

        function onSmartPasteDone() {
            // Called at the end of smartPaste() — proof rows are all inserted.
            // isExtFile stays true (the proof is now an "opened" document).
            fileModified = true
            menuOptions.close()
        }
    }

    // Dialogs associated with the DrawerTools
    FileDialog {
        id: fileDialogID

        title: "Choose the proof file"
        //selectFolder: false
        nameFilters: ["Aris files (*.tle)"]
        fileMode: FileDialog.OpenFile
        defaultSuffix: "tle"
        onAccepted: {
            cConnector.openProof(selectedFile, theData, theGoals)
            filename = selectedFile
            isExtFile = true
            fileModified = false
            computePremise = true
            proofModel.recomputePremiseCount()
        }
    }

    FileDialog {
        id: saveAsID

        property bool closeOnSave: false

        nameFilters: ["Aris files (*.tle)"]
        title: "Save As"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "tle"
        onAccepted: {
            cConnector.saveProof(selectedFile, theData, theGoals)
            filename = selectedFile
            fileExists = true
            fileModified = false
            if (closeOnSave) rootID.close()
        }
    }

    FileDialog {
        id: latexID

        nameFilters: ["Latex files (*.tex)"]
        title: "Save As"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "tex"
        onAccepted: auxConnector.latex(selectedFile, theData, cConnector)
    }

    FileDialog {
        id: importID

        nameFilters: ["Aris files (*.tle)"]
        title: "Import Proof"
        fileMode: FileDialog.OpenFile
        defaultSuffix: "tle"
        onAccepted: {
            auxConnector.importProofWithMode(selectedFile, theData, cConnector,
                                     proofModel, importMode)
            proofModel.recomputePremiseCount()
        }
    }

    FontDialog {
        id: fontDialogID

        title: "Choose Font"
        onAccepted: thefont = currentFont
    }

    Dialog {
        id: goalDialogID

        title: "Goals"
        width: parent.width / 2
        height: parent.height / 2
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        background: Rectangle {
            anchors.fill: parent
            color: darkMode ? "#1F1B24" : "white"
            opacity: 1 //0.6
            border.width: 2
        }

        palette {

            button: darkMode ? "#1F1A24" : "white"
            buttonText: darkMode ? "white" : "black"
            text: darkMode ? "white" : "black"
            window: darkMode ? "#1F1B24" : "white"
        }

        parent: Overlay.overlay
        focus: true
        //modal: true
        closePolicy: Popup.CloseOnEscape // To allow keyboard access
        standardButtons: Dialog.Ok

        ColumnLayout {
            id: goalAreaID

            anchors.fill: parent

            ListView {
                model: goalDataID
                delegate: GoalLine {}

                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                ScrollBar.vertical: ScrollBar {}
            }
        }
    }

    // Message Dialog when Closing App
    Dialog {
        id: unsavedChangesID

        title: qsTr("Unsaved changes")
        anchors.centerIn: parent

        parent: Overlay.overlay
        modal: true
        closePolicy: Popup.CloseOnEscape

        standardButtons: MessageDialog.Save | MessageDialog.Discard | MessageDialog.Cancel
        onAccepted: {
            if (Qt.platform.os === "wasm")
                cConnector.wasmSaveProof(theData, theGoals);
            else if (fileExists)
                cConnector.saveProof(filename, theData, theGoals);
            else {
                saveAsID.closeOnSave = true
                saveAsID.open();
                return
            }
            fileModified = false
            rootID.close();
        }
        onDiscarded: {
            fileModified = false;
            rootID.close();
        }

        Label {
            text: isExtFile
                 ? qsTr("Save file before closing?")
                 : qsTr("Save new file before closing?")
        }
    }

    Dialog {
        id: resetDialogID

        width: Math.min(rootID.width * 0.40, 420)
        anchors.centerIn: parent

        parent: Overlay.overlay
        modal: true
        closePolicy: Popup.CloseOnEscape
        padding: 20

        Overlay.modal: Rectangle {
            color: darkMode ? "#66121212" : "#66CFCFCF"
        }

        background: Rectangle {
            radius: 12
            color: darkMode ? "#1F1B24" : "white"
            border.width: 1
            border.color: darkMode ? "#50485A" : "#D9D9D9"
        }

        contentItem: ColumnLayout {
            width: resetDialogID.availableWidth
            spacing: 20

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("Reset the current proof and goals?")
                color: darkMode ? "white" : "black"
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 18

                Button {
                    text: qsTr("Cancel")
                    Layout.fillWidth: true

                    palette {
                        button: darkMode ? "#2A2631" : "white"
                        buttonText: darkMode ? "white" : "black"
                    }

                    onClicked: resetDialogID.close()
                }

                Button {
                    text: qsTr("Reset")
                    Layout.fillWidth: true

                    palette {
                        button: darkMode ? "#2A2631" : "white"
                        buttonText: darkMode ? "white" : "black"
                    }

                    onClicked: {
                        resetDialogID.close()
                        resetMainWindow()
                    }
                }
            }
        }
    }

    Dialog {
        id: importBehaviorID

        width: Math.min(rootID.width * 0.42, 440)
        anchors.centerIn: parent

        parent: Overlay.overlay
        modal: true
        closePolicy: Popup.CloseOnEscape
        padding: 20

        Overlay.modal: Rectangle {
            color: darkMode ? "#66121212" : "#66CFCFCF"
        }

        background: Rectangle {
            radius: 12
            color: darkMode ? "#1F1B24" : "white"
            border.width: 1
            border.color: darkMode ? "#50485A" : "#D9D9D9"
        }

        contentItem: ColumnLayout {
            width: importBehaviorID.availableWidth
            spacing: 20

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("Choose import behavior")
                color: darkMode ? "white" : "black"
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 14

                Button {
                    text: qsTr("Overwrite")
                    Layout.fillWidth: true

                    palette {
                        button: darkMode ? "#2A2631" : "white"
                        buttonText: darkMode ? "white" : "black"
                    }

                    onClicked: startImportFlow(0)
                }

                Button {
                    text: qsTr("Append End")
                    Layout.fillWidth: true

                    palette {
                        button: darkMode ? "#2A2631" : "white"
                        buttonText: darkMode ? "white" : "black"
                    }

                    onClicked: startImportFlow(1)
                }

                Button {
                    text: qsTr("Prepend")
                    Layout.fillWidth: true

                    palette {
                        button: darkMode ? "#2A2631" : "white"
                        buttonText: darkMode ? "white" : "black"
                    }

                    onClicked: startImportFlow(2)
                }
            }
        }
    }

    GoalModel {
        id: goalDataID
        glines: theGoals
    }

    ProofModel {
        id: proofModel
        lines: theData
    }

    //  Zoom keyboard shortcuts
    Shortcut { sequences: [ StandardKey.ZoomIn ];  onActivated: zoomIn()    }
    Shortcut { sequence: "Ctrl+=";            onActivated: zoomIn()    }
    Shortcut { sequences: [ StandardKey.ZoomOut ]; onActivated: zoomOut()   }
    Shortcut { sequence: "Ctrl+0";            onActivated: zoomReset() }

    // Smart paste — Ctrl+V at window scope so TextFields still handle their own Ctrl+V.
    // Only fires when no text-editing widget has consumed the key.
    Shortcut {
        sequence: "Ctrl+V"
        onActivated: {
            // smartPasteStarted signal sets isExtFile/computePremise before rows are inserted
            cConnector.smartPaste(theData, proofModel)
        }
    }

    // Smart copy — Ctrl+C at window scope; TextFields consume it first when focused.
    Shortcut {
        sequence: "Ctrl+C"
        onActivated: {
            cConnector.smartCopy(theData, proofID.selectedIndices)
        }
    }

    ProofArea {
        id: proofID
    }
}

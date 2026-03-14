
/* Drawer ToolBar Custom QML Type.

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
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs

ToolBar {

    anchors.fill: parent

    palette {
        buttonText: darkMode ? "white" : "black"
        window: darkMode ? "#121212" : "white"
        button: darkMode ? "#5C469C" : "white"
    }

    ColumnLayout {

        ToolButton {
            text: qsTr("New")
            icon.name: "new"
            icon.color: darkMode ? "white" : "black"

            onClicked: {
                if (Qt.platform.os === "wasm")
                    Qt.openUrlExternally("https://matek.hu/zoltan/aris/")
                else
                    auxConnector.newWindow()
            }
        }

        ToolButton {
            text: qsTr("Open")
            icon.name: "folder"
            icon.color: darkMode ? "white" : "black"

            onClicked: {
                cConnector.evalText = "Evaluate Proof"

                if (Qt.platform.os === "wasm") {
                    isExtFile = true
                    cConnector.wasmOpenProof(theData, theGoals)
                    computePremise = true
                } else
                    fileDialogID.open()

                menuOptions.close()
            }
        }

        ToolButton {
            text: qsTr("Save")
            icon.name: "save"
            icon.color: darkMode ? "white" : "black"
            visible: !(Qt.platform.os === "wasm")

            onClicked: {

                if (fileExists)
                    cConnector.saveProof(filename, theData, theGoals)
                else
                    saveAsID.open()

                menuOptions.close()
            }
        }

        ToolButton {
            text: qsTr("Save As")
            icon.name: "saveas"
            icon.color: darkMode ? "white" : "black"

            onClicked: {

                if (Qt.platform.os === "wasm")
                    cConnector.wasmSaveProof(theData, theGoals)
                else
                    saveAsID.open()

                menuOptions.close()
            }
        }

        ToolButton {
            text: qsTr("Export To LaTeX")
            icon.name: "export"
            icon.color: darkMode ? "white" : "black"

            onClicked: {
                if (Qt.platform.os === "wasm")
                    auxConnector.wasmLatex(theData, cConnector)
                else
                    latexID.open()

                menuOptions.close()
            }
        }

        ToolSeparator {
            orientation: Qt.Horizontal
        }

        ToolButton {
            text: qsTr("Toggle Goals")
            icon.name: "goal"
            icon.color: darkMode ? "white" : "black"

            onClicked: {
                if (goalDialogID.opened)
                    goalDialogID.close()
                else
                    goalDialogID.open()
                menuOptions.close()
            }
        }

        ToolButton {
            text: qsTr("Toggle Dark Mode")
            icon.name: "boolean"
            icon.color: darkMode ? "white" : "black"

            onClicked: darkMode = !darkMode
        }

        ToolButton {
            text: qsTr("Import Proof")
            icon.name: "import"
            icon.color: darkMode ? "white" : "black"

            onClicked: {
                cConnector.evalText = "Evaluate Proof"
                if (Qt.platform.os === "wasm") {
                    isExtFile = true
                    auxConnector.wasmImportProof(theData, cConnector,
                                                 proofModel)
                    computePremise = true
                } else {
                    importID.open()
                }

                menuOptions.close()
            }
        }

        ToolSeparator {
            orientation: Qt.Horizontal
        }

        ToolButton {
            text: qsTr("Font")
            icon.name: "font"
            icon.color: darkMode ? "white" : "black"
            visible: !(Qt.platform.os === "wasm")

            onClicked: fontDialogID.open()
        }

        ToolSeparator {
            visible: !(Qt.platform.os === "wasm")
            orientation: Qt.Horizontal
        }

        ToolButton {
            text: qsTr("Help")
            icon.name: "help"
            icon.color: darkMode ? "white" : "black"
            onClicked: Qt.openUrlExternally(
                           "https://www.gnu.org/software/aris/manual/aris.pdf")
        }

        ToolButton {
            text: qsTr("About")
            icon.name: "about"
            icon.color: darkMode ? "white" : "black"
            onClicked: Qt.openUrlExternally(
                           "https://www.gnu.org/software/aris/")
        }
    }
}

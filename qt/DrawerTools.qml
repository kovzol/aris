import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs

ToolBar{

    anchors.fill: parent

    ColumnLayout{

        ToolButton{
            text: qsTr("Open")
            icon.source: "/assets/folder.png"
            onClicked: {
                cConnector.evalText = "Evaluate Proof";
                if (cConnector.isWasm())
                    cConnector.wasmOpenProof(theData)
                else
                    fileDialogID.open()

                menuOptions.close()
                isExtFile = true;
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
                if (cConnector.isWasm())
                    cConnector.wasmSaveProof(theData);
                else
                    saveAsID.open()

                menuOptions.close()
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

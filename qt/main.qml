import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

Window {
    id: rootID
    width: 640
    height: 480
    visible: true
    title: qsTr("GNU Aris")

    Rectangle{
        id: burgerMenu
        height: 55
        width: 50
        border.color: "black"

        Text{
            anchors.centerIn: parent
            text: "\u2630"
            minimumPointSize: 20
        }

        MouseArea{
            anchors.fill: parent
            onClicked: {
                menuOptions.open()
            }
        }
    }

    Frame{
        anchors.verticalCenter: parent.verticalCenter
        ToolBar{

            Column{

                ToolButton{
                    id: conjunctionButton
                    text: "\u2227"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Conjunction"
                }

                ToolButton{
                    id: disjunctionButton
                    text: "\u2228"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Disjunction"
                }

                ToolButton{
                    id: negationButton
                    text: "\u00ac"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Negation"
                }

                ToolButton{
                    id: implicationButton
                    text: "\u2192"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Implication"
                }

                ToolButton{
                    id: biconditionalButton
                    text: "\u2194"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Bi-conditional"
                }

                ToolButton{
                    id: forAllButton
                    text: "\u2200"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "For all"
                }

                ToolButton{
                    id: thereExistsButton
                    text: "\u2203"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "There exists"
                }

                ToolButton{
                    id: tautologyButton
                    text: "\u22a4"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Tautology"
                }

                ToolButton{
                    id: contradictionButton
                    text: "\u22a5"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Contradiction"
                }

                ToolButton{
                    id: belongsToButton
                    text: "\u2208"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Belongs To"
                }

                ToolButton{
                    id: nullButton
                    text: "\u2205"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Null"
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
                }

                ToolButton{
                    text: qsTr("Save")
                    icon.source: "/assets/save.png"
                }

                ToolButton{
                    text: qsTr("Save As")
                    icon.source: "/assets/saveas.png"
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
                }

                ToolSeparator{
                    orientation: Qt.Horizontal
                }

                ToolButton{
                    text: qsTr("Help")
                    icon.source: "/assets/help.png"
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
}

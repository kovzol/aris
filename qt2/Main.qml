import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import GNU.Aris.Widgets

ApplicationWindow {
    id: window
    visible: true
    title: qsTr("GNU Aris")

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            Action { text: qsTr("&New...") }
            Action { text: qsTr("&Open") }
            Action { text: qsTr("&Save") }
            Action { text: qsTr("Save &As...") }
            MenuSeparator { }
            Action { text: qsTr("&Quit") }
        }
        Menu {
            title: qsTr("&Edit")
            Action { text: qsTr("Cu&t") }
            Action { text: qsTr("&Copy") }
            Action { text: qsTr("&Paste") }
        }
        Menu {
            title: qsTr("&Help")
            Action { text: qsTr("&About") }
        }
    }


    ToolTip {
        id: symbolTooltip
        property string toolTipText

        delay: 0
        contentItem: Text {
            text: "
              <h4>Logical Implication (<font face='CMU Serif'><i>P</i> → <i>Q</i></font>)</h4>
              If <font face='CMU Serif'><i>P</i></font> is true, then <font face='CMU Serif'><i>Q</i></font> must also be true.
            "
            color: "#ffffff"
            textFormat: Text.RichText
        }
    }

    Timer {
        property string id: ""
        property string text: ""
        property int toolTipX;
        property int toolTipY;
        id: hoverTimer
        interval: 500
        repeat: false
        onTriggered: {
            symbolTooltip.toolTipText = text;
            symbolTooltip.x = Qt.binding(()=>toolTipX);
            symbolTooltip.y = Qt.binding(()=>toolTipY);
            symbolTooltip.open();
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        ColumnLayout {
            width: parent.width
            spacing: 10
            RowLayout {
                spacing: 0
                Layout.fillHeight: true
                Button {
                    text: "1"
                    font.family: "CMU Serif"
                    font.pointSize: 30
                }
                ProofLine {
                }
            }
            RowLayout {
                spacing: 0
                Layout.fillHeight: true
                Button {
                    text: "2"
                    font.family: "CMU Serif"
                    font.pointSize: 30
                }
                ProofLine {
                }
            }
            RowLayout {
                spacing: 0
                Layout.fillHeight: true
                Button {
                    text: "3"
                    font.family: "CMU Serif"
                    font.pointSize: 30
                }
                ProofLine {
                }
            }
            RowLayout {
                spacing: 0
                Layout.fillHeight: true
                Button {
                    text: "4"
                    font.family: "CMU Serif"
                    font.pointSize: 30
                }
                ProofLine {
                }
            }
        }
    }
}

import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import goal.model 1.0


RowLayout{
    spacing: 10
    width: (parent)? parent.width: 0
    Layout.fillWidth: true

    Label{
        id: goalLineNumID

        height: goalTextID
        width: height + 10

        Text{
            id: resNumID
            anchors.centerIn: parent
            font.italic: true
            text: (line > 0) ? line: ((line === -3)?  "X" : "?")
            color: (text === "?")? "yellow": ((text === "X") ? "red": ((model.valid)? "green": "blue"))

        }

        property string toolTipText: (resNumID.color === Qt.color("green"))? "Goal was met at line " + line : ((resNumID.color === Qt.color("blue")) ? "Goal was met at line "+ line+"\n\t but the proof has errors": ((resNumID.color === Qt.color("red"))?"Goal was not met": "Not yet evaluated"))  //aaaaaaaaaaaaaaaaaaaaaaaaaaa
        ToolTip.visible: toolTipText ? moID.containsMouse : false
        ToolTip.text: toolTipText

        MouseArea{
            id: moID
            anchors.fill: parent
            hoverEnabled: true
            onClicked: console.log(line);
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

        text: model.text
        //        wrapMode: TextArea.Wrap
        placeholderText: qsTr("Start Typing here...")

        onEditingFinished: {
            model.text = text
        }

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
                    theGoals.insertgLine(index + 1,-2,false,"");
                    //                    listView.currentIndex = index + 1;
                }
            }
            Action{
                text: "Remove Goal"
                onTriggered: {
                    if (goalDataID.rowCount() > 1){
                        theGoals.removegLineAt(index)
                    }
                    else
                        console.log("Invalid Operation: Cannot remove all Lines")
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


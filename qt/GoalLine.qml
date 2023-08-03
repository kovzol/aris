import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import goal.model 1.0


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

        text: model.text
//        wrapMode: TextArea.Wrap
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
                    theGoals.insertgLine(index + 1,-1,"");
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


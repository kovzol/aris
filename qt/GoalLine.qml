import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5



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
        wrapMode: TextArea.Wrap
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
                    goalDataID.insert(index+1,{"line":index + 2});
                    updateLines(goalDataID)
                }
            }
            Action{
                text: "Remove Goal"
                onTriggered: {
                    if (goalDataID.count > 1){
                        goalDataID.remove(index)
                        updateLines(goalDataID)
                    }
                    else
                        console.log("Invalid Operation")
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


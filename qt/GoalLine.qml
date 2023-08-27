/* Goal Line Custom QML Type.

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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import goal.model 1.0


RowLayout{

    property string toolTipText: (resNumID.color === Qt.color("green"))? "Goal was met at line " + line : ((resNumID.color === Qt.color("blue")) ? "Goal was met at line "+ line+"\n\t but the proof has errors": ((resNumID.color === Qt.color("red"))?"Goal was not met": "Not yet evaluated"))  //aaaaaaaaaaaaaaaaaaaaaaaaaaa

    spacing: 10
    width: (parent)? parent.width: 0
    Layout.fillWidth: true

    Label{
        id: goalLineNumID

        height: goalTextID
        width: height + 10

        ToolTip.visible: toolTipText ? moID.containsMouse : false
        ToolTip.text: toolTipText

        MouseArea{
            id: moID
            anchors.fill: parent
            hoverEnabled: true
            onClicked: console.log(line);
        }

        Text{
            id: resNumID
            anchors.centerIn: parent
            font.italic: true
            text: (line > 0) ? line: ((line === -3)?  "X" : "?")
            color: (text === "?")? (darkMode?"yellow":"brown"): ((text === "X") ? "red": ((model.valid)? "green": "blue"))

        }

    }

    TextField{
        id: goalTextID

        height: font.pointSize + 10
        width: 200
        Layout.fillWidth: true
        background: Rectangle{
            color: darkMode?"#332940":"lightgrey"
        }

        text: model.text
        //        wrapMode: TextArea.Wrap
        //        placeholderText: qsTr("Start Typing here...")

        // Implementing Keyboard Macros
        onTextChanged: {

            if (goalTextID.length >= 2){
                const last_two = text.slice(cursorPosition-2,cursorPosition)
                if (last_two.includes('/\\')){
                    goalTextID.remove(cursorPosition-2, cursorPosition)
                    goalTextID.insert(cursorPosition,"\u2227")
                }
                else if (last_two.includes('\\/')){
                    goalTextID.remove(cursorPosition-2, cursorPosition)
                    goalTextID.insert(cursorPosition,"\u2228")
                }
                else if (last_two.includes('->')){
                    goalTextID.remove(cursorPosition-2, cursorPosition)
                    goalTextID.insert(cursorPosition,"\u2192")
                }
                else if (last_two.includes('<'+"\u2192")){
                    goalTextID.remove(cursorPosition-2, cursorPosition)
                    goalTextID.insert(cursorPosition,"\u2194")
                }

            }

        }

        onEditingFinished: model.text = text

    }

    Button{
        id: goalPlusID

        height: goalTextID.height

        onClicked: goalOptionsID.open()

        Text{
            anchors.centerIn: parent
            text: "+ / \u2013"
            color: darkMode? "white": "black"
        }

        Menu{
            id: goalOptionsID

            Action{
                text: "Add Goal"
                onTriggered: {
                    theGoals.insertgLine(index + 1,-2,false,"");
                }
            }

            Action{
                text: "Remove Goal"
                onTriggered: {
                    if (goalDataID.rowCount() > 1){
                        theGoals.removegLineAt(index);
                    }
                    else
                        console.log("Invalid Operation: Cannot remove all Lines")
                }
            }


        }

    }
}


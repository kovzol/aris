
/* Buttons associated with on-screen keyboard.

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

ToolButton {

    property string txt: "Symbol"

    height: thefont.pointSize + 15

    hoverEnabled: true
    ToolTip.visible: hovered
    ToolTip.text: "Operation Name"

    focusPolicy: Qt.NoFocus // Text is passed to focused TextArea, so the button musn't steal focus

    onClicked: {
        if (isTextField(activeFocusItem))
            activeFocusItem.insert(activeFocusItem.cursorPosition, txt)
    }

    Text {
        text: txt
        font: thefont
        color: darkMode ? "#BB86FC" : "black"
        anchors.centerIn: parent
        fontSizeMode: Text.HorizontalFit
    }
}

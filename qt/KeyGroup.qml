/* Container for the on-screen keyboard buttons.

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

ToolBar{
    spacing: 5

    background: Rectangle{
        anchors.fill: parent
        color: darkMode? "#121212":"white"
    }

    Column{
        id: keyColID

        KeyButton{
            id: conjunctionButton


            txt: "\u2227"
            ToolTip.text: "Conjunction"
        }

        KeyButton{
            id: disjunctionButton


            txt: "\u2228"
            ToolTip.text: "Disjunction"
        }

        KeyButton{
            id: negationButton


            txt: "\u00ac"
            ToolTip.text: "Negation"
        }

        KeyButton{
            id: implicationButton


            txt: "\u2192"
            ToolTip.text: "Implication"
        }

        KeyButton{
            id: biconditionalButton

            txt: "\u2194"
            ToolTip.text: "Bi-conditional"
        }

        KeyButton{
            id: forAllButton

            txt: "\u2200"
            ToolTip.text: "For all"
        }

        KeyButton{
            id: thereExistsButton


            txt: "\u2203"
            ToolTip.text: "There exists"
        }

        KeyButton{
            id: tautologyButton


            txt: "\u22a4"
            ToolTip.text: "Tautology"
        }

        KeyButton{
            id: contradictionButton


            txt: "\u22a5"
            ToolTip.text: "Contradiction"
        }

        KeyButton{
            id: belongsToButton


            txt: "\u2208"
            ToolTip.text: "Belongs To"
        }

        KeyButton{
            id: nullButton



            txt: "\u2205"
            ToolTip.text: "Null"
        }

        ToolSeparator{
            orientation: Qt.Horizontal
        }

        // Evaluate Proof Button

        ToolButton{
            id: evalButton

            icon {
                source: "assets/eval.png"
                height: (Qt.platform.os === "wasm") ? nullButton.height/1.5 : nullButton.height/2
                width:  (Qt.platform.os === "wasm") ? nullButton.width/1.5 : nullButton.width/2
            }
            hoverEnabled: true
            ToolTip.visible: hovered
            ToolTip.text: cConnector.evalText
            background: Rectangle{
                id: runButtonID
                color: (cConnector.evalText === "Evaluate Proof")? (darkMode?"#BB86FC":"white"): (cConnector.evalText === "Correct!")? (darkMode?"springgreen":"green"): "red"
                radius: 5
            }

            onClicked: {
                cConnector.evalProof(theData,theGoals);
                goalDataID.evalGoals(theGoals,cConnector);
                animationID.start();
            }

            SequentialAnimation {
                id: animationID

                // Expand the button
                PropertyAnimation {
                    target: runButtonID
                    property: "scale"
                    to: 1.2
                    duration: 200
                    easing.type: Easing.InOutQuad
                }

                // Shrink back to normal
                PropertyAnimation {
                    target: runButtonID
                    property: "scale"
                    to: 1.0
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}

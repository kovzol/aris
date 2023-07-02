import QtQuick 2.15
import QtQuick.Controls 2.5

ToolBar{

    Column{
        id: keyColID

        KeyButton{
            id: conjunctionButton

            text: "\u2227"
            ToolTip.text: "Conjunction"
        }

        KeyButton{
            id: disjunctionButton

            text: "\u2228"
            ToolTip.text: "Disjunction"
        }

        KeyButton{
            id: negationButton

            text: "\u00ac"
            ToolTip.text: "Negation"
        }

        KeyButton{
            id: implicationButton

            text: "\u2192"
            ToolTip.text: "Implication"
        }

        KeyButton{
            id: biconditionalButton

            text: "\u2194"
            ToolTip.text: "Bi-conditional"
        }

        KeyButton{
            id: forAllButton

            text: "\u2200"
            ToolTip.text: "For all"
        }

        KeyButton{
            id: thereExistsButton

            text: "\u2203"
            ToolTip.text: "There exists"
        }

        KeyButton{
            id: tautologyButton

            text: "\u22a4"
            ToolTip.text: "Tautology"
        }

        KeyButton{
            id: contradictionButton

            text: "\u22a5"
            ToolTip.text: "Contradiction"
        }

        KeyButton{
            id: belongsToButton

            text: "\u2208"
            ToolTip.text: "Belongs To"
        }

        KeyButton{
            id: nullButton

            text: "\u2205"
            ToolTip.text: "Null"
        }

        ToolSeparator{
            orientation: Qt.Horizontal
        }

        // Evaluate Proof Button

        ToolButton{
            id: evalButton

            icon.source: "/assets/eval.png"
            hoverEnabled: true
            ToolTip.visible: hovered
            ToolTip.text: "Evaluate Proof"

                    onClicked: {
                        Wrapper.clearData();
                        fillWrap();
                        Wrapper.computeIndices();
                        Wrapper.computeRules();
                        Wrapper.displayData();
                    }
        }
    }
}

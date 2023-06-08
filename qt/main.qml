import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Dialogs

Window {
    id: rootID
    width: 1200
    height: 700
    visible: true
    title: qsTr("GNU Aris")

    Button{
        id: burgerMenu
        height: keyboardID.width
        width: keyboardID.width

        hoverEnabled: true
        ToolTip.visible: hovered
        ToolTip.text: "Menu"

        Text{
            anchors.centerIn: parent
            text: "\u2630"
            minimumPointSize: 20
        }

        onClicked: {
            menuOptions.open()
        }

    }

    function isTextArea(item){
        return item instanceof TextArea
    }

    Frame{
        id: keyboardID
        anchors.verticalCenter: parent.verticalCenter

        ToolBar{

            Column{
                id: keyColID

                ToolButton{
                    id: conjunctionButton
                    text: "\u2227"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Conjunction"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: disjunctionButton
                    text: "\u2228"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Disjunction"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: negationButton
                    text: "\u00ac"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Negation"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: implicationButton
                    text: "\u2192"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Implication"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: biconditionalButton
                    text: "\u2194"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Bi-conditional"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: forAllButton
                    text: "\u2200"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "For all"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: thereExistsButton
                    text: "\u2203"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "There exists"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: tautologyButton
                    text: "\u22a4"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Tautology"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: contradictionButton
                    text: "\u22a5"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Contradiction"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: belongsToButton
                    text: "\u2208"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Belongs To"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
                }

                ToolButton{
                    id: nullButton
                    text: "\u2205"

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "Null"

                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        if (isTextArea(activeFocusItem))
                            activeFocusItem.insert(activeFocusItem.cursorPosition, text)
                    }
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
                    onClicked: {
                        fileDialogID.open()
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
                        saveAsID.open()
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

    FileDialog{
        id: fileDialogID
        title: "Choose the proof file"
        //selectFolder: false
    }

    FileDialog{
        id: saveAsID
        title: "Save As"
        fileMode: FileDialog.SaveFile
    }

    FontDialog{
        id: fontDialogID
        title: "Choose Font"
    }

    ColumnLayout{
        id: proofAreaID
        anchors.fill: parent
        anchors.leftMargin: keyboardID.width + 20
        anchors.topMargin: 20
        anchors.rightMargin: 20

        ListView{
            model: proofDataID
            delegate: proofLineID
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            ScrollBar.vertical: ScrollBar{}
        }
    }

    ListModel{
        id: proofDataID

        ListElement{
            line: 1 ; type: "premise"; sub: false; subStart: false; subEnd: false; indent: 0
        }

    }

    Component{
        id: proofLineID

        RowLayout{
            spacing: 10
            width: parent.width
            Layout.fillWidth: true

            Button{
                //Layout.leftMargin: indent
                id: lineNumberID
                height: theTextID.height
                width: height

                Text{
                    anchors.centerIn: parent
                    font.italic: true
                    text: line
                }
            }

            TextArea{
                id: theTextID
                height: font.pointSize + 10
                Layout.leftMargin: indent
                Layout.fillWidth: true
                background: Rectangle{
                    color: "lightgrey"
                }
                wrapMode: TextArea.Wrap
                placeholderText: qsTr("Start Typing here...")
            }

            Label{
                id: ruleID
                height: theTextID.height
                width: 50
                visible: (type === "choose")? false: true

                Text{
                    anchors.centerIn: parent
                    font.italic: true
                    text: type
                }
            }

            ComboBox{
                id: chooseID
                visible: (type === "choose")? true: false
                height: theTextID.height

                model: ["Inference", "Equivalence", "Predicate", "Miscellaneous", "Boolean"]
            }

            ComboBox{
                id: conclusionRuleID
                visible: (type === "choose")? true: false
                height: theTextID.height

                hoverEnabled: true
                ToolTip.visible: hovered
                ToolTip.text: currentText

                model:  (chooseID.currentText === "Inference")?
                            ["modus ponens", "addition", "simplification", "conjunction", "hypothetical syllogism", "disjunctive syllogism", "excluded middle", "constructive dilemma"]:
                            (chooseID.currentText === "Equivalence")?
                                ["Implication", "DeMorgan", "Association", "Commutativity", "Idempotence","Distribution","Equivalence","Double Negation", "Exportation", "Subsumption"]:
                                (chooseID.currentText === "Predicate")?
                                    ["Universal Generalization", "Universal Instantiation", "Existential Generalization", "Existential Instantiation", "Bound Variable Substitution", "Null Quantifier", "Prenex", "Identity", "Free Variable Substitution"]:
                                    (chooseID.currentText === "Miscellaneous")?
                                        ["Lemma","Subproof","Sequence","Induction"]:
                                        ["Identity","Negation","Dominance","Symbol Negation"]
            }

            Button{
                id: plusID
                height: theTextID.height

                Text{
                    anchors.centerIn: parent
                    text: "+ / -"
                }

                onClicked: {
                    optionsID.open()
                }

                Menu{
                    id: optionsID
                    Action{
                        text: "Add Premise"
                        onTriggered: {
                            proofDataID.insert(0,{"line": 1 , "type" : "premise"})
                            updateLines()

                        }
                    }
                    Action{
                        text: "Add Conclusion"
                        onTriggered: {
                            proofDataID.insert(index + 1,{"line": index +2, "type": "choose", "indent": indent, "sub": sub})
                            updateLines()
                        }
                    }
                    Action{
                        text: "Start Subproof"
                        onTriggered:{
                            proofDataID.insert(index + 1,{"line": index + 2 , "type" : "subproof", "sub": true, "subStart": true, "subEnd": false, "indent": indent+20})
                            updateLines()
                        }
                    }
                    Action{
                        text: "End Subproof"
                        onTriggered: {
                            if (sub === false)
                                console.log("Invalid Operation")
                            else{
                                proofDataID.insert(index + 1,{"line": index + 2 , "type" : "sub-concl.", "sub": (indent > 20) ? true : false, "subStart": false, "subEnd": true, "indent": indent-20})
                                updateLines()
                            }
                        }

                    }

                    Action{
                        text: "Remove this Line"
                        onTriggered: {
                            if (proofDataID.count > 1){
                                proofDataID.remove(index)
                                updateLines()
                            }
                            else
                                console.log("Invalid Operation")
                        }
                    }
                }
            }
        }
    }

    function updateLines(){

        for (var i = 0; i < proofDataID.count; i++){
            proofDataID.set(i,{"line":i+1})
        }

    }

}

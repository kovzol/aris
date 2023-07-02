import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

Item {
    anchors.fill: parent
    ColumnLayout{
        id: proofAreaID

        anchors{
            fill: parent
            leftMargin: keyboardID.width + 20
            topMargin: 20
            rightMargin: 20
        }

        ListView{
            id: listView

            model: proofDataID
            delegate: proofLineID
            highlight: highlightID

            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            ScrollBar.vertical: ScrollBar{}

            onCurrentItemChanged: {
                currentItem.children[1].forceActiveFocus()
            }
        }

    }

//    ListModel{
//        id: proofDataID

//        ListElement{
//            line: 1 ; type: "premise"; sub: false; subStart: false; subEnd: false; indent: 0; refs: [ListElement{ num : -1}]
//        }

//    }

    Component{
        id: proofLineID

        RowLayout{
            spacing: 10
            width: parent.width
            Layout.fillWidth: true

            Button{
                id: lineNumberID

                height: theTextID.height
                width: height

                Text{
                    anchors.centerIn: parent
                    font.italic: true
                    text: line
                }

                onClicked: {
                    if (listView.currentIndex <= index)
                        console.log("Invalid Operation : Can only reference to smaller line numbers");
                    else if (proofDataID.get(listView.currentIndex).type === "premise")
                        console.log("Invalid Operation: Current Line is a premise");
                    else if (proofDataID.get(listView.currentIndex).subStart === true || proofDataID.get(listView.currentIndex).subEnd === true)
                        console.log("Invalid Operation: subproof");
                    else if (proofDataID.get(listView.currentIndex).indent < indent)
                        console.log("Invalid Operation: invalid reference to subproof");
                    else{
                        for(var i = 0; i < proofDataID.get(listView.currentIndex).refs.count; i++){
                            if (proofDataID.get(listView.currentIndex).refs.get(i).num === line){
                                proofDataID.get(listView.currentIndex).refs.remove(i);
                                return;
                            }
                        }
                        proofDataID.get(listView.currentIndex).refs.append({num:line});
                    }
                }
            }

            TextField{
                id: theTextID

                height: font.pointSize + 10
                Layout.leftMargin: indent
                Layout.fillWidth: true
                background: Rectangle{
                    color: "lightgrey"
                }

//                wrapMode: TextArea.Wrap
                placeholderText: qsTr("Start Typing here...")

                MouseArea{
                    anchors.fill: parent
                    propagateComposedEvents: true
                    onClicked: {
                        listView.currentIndex = index;
                        parent.forceActiveFocus();
                        parent.cursorPosition = parent.positionAt(mouseX, mouseY);
                    }
                }

                onAccepted: {
                    plusID.clicked()
                }

                onTextChanged: {
                    model.text = text;
                }
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

            Row{

                Repeater{

                    model: refs

                    Button{
                        visible: (num === -1)? false: true
                        text: num
                        onClicked: {
                            refs.remove(index);
                        }

                    }

                }

            }

            Button{
                id: plusID

                height: theTextID.height

                Text{
                    anchors.centerIn: parent
                    text: "+ / \u2013"
                }

                onClicked: {
                    optionsID.open()
                }

                Menu{
                    id: optionsID

                    Action{
                        text: "Add Premise"
                        onTriggered: {
                            proofDataID.insert(0,{"line": 1 , "type" : "premise", "refs": [{ num : -1}]});
                            updateLines(proofDataID);
                            listView.currentIndex = 0;
                        }
                    }

                    Action{
                        text: "Add Conclusion"
                        onTriggered: {
                            proofDataID.insert(index+1,{"line": index +2, "type": "choose", "indent": indent, "sub": sub, "refs": [{ num : -1}]});
                            updateLines(proofDataID);
                            listView.currentIndex = index + 1;
                        }
                    }

                    Action{
                        text: "Start Subproof"
                        onTriggered:{
                            proofDataID.insert(index+1,{"line": index + 2 , "type" : "subproof", "sub": true, "subStart": true, "subEnd": false, "indent": indent+20});
                            updateLines(proofDataID);
                            listView.currentIndex = index + 1;
                        }
                    }

                    Action{
                        text: "End Subproof"
                        onTriggered: {
                            if (sub === false)
                                console.log("Invalid Operation")
                            else{
                                proofDataID.insert(index+1,{"line": index + 2 , "type" : "sub-concl.", "sub": (indent > 20) ? true : false, "subStart": false, "subEnd": true, "indent": indent-20});
                                updateLines(proofDataID);
                                listView.currentIndex = index + 1;
                            }
                        }

                    }

                    Action{
                        text: "Remove this Line"
                        onTriggered: {
                            if (proofDataID.count > 1){
                                proofDataID.remove(index)
                                updateLines(proofDataID)
                            }
                            else
                                console.log("Invalid Operation")
                        }
                    }
                }
            }
        }
    }

    Component{
        id: highlightID

        Rectangle{
            width: parent.width
            color: "wheat"
            radius: 10
        }
    }

}

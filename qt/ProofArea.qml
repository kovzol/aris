import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import proof.model 1.0

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

            model: proofModel
            delegate: proofLineID
            highlight: highlightID

            currentIndex: -1

            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            ScrollBar.vertical: ScrollBar{}

            onCurrentItemChanged: {
                if (currentItem)
                    currentItem.children[1].forceActiveFocus()
            }


        }

    }

    Component{
        id: proofLineID


        RowLayout{
            id: root_delegate

            property bool editCombos: (!isExtFile || type === "choose");
            property var arr: model.refs;
            property string type: model.type;
            property int indexx: model.index;
            property bool vis: type === "premise" || type === "subproof" || type === "sf";
            property string textFieldColor: (listView.currentIndex !== -1)?(proofModel.data(proofModel.index(listView.currentIndex,0),263).includes(model.line) ? (darkMode?"brown":"yellow") : (darkMode ? "#1F1A24":"white")):(darkMode?"#332940":"lightgrey");

            // Function to refresh the line color after selecting/de-selecting references
            function refreshTextFieldColor(){
                var temp = listView.currentIndex;
                listView.currentIndex =  -1;
                listView.currentIndex = temp;
            }

            spacing: 10
            width: (parent)? parent.width: 0
            Layout.fillWidth: true

            // Line Number Button
            Button{
                id: lineNumberID

                height: theTextID.height
                width: height
                palette {
                    button: darkMode? "#1F1A24" : "white"
                }

                Text{
                    anchors.centerIn: parent
                    font.italic: true
                    text: model.line
                    color: theTextID.color
                }


                // Add this button's line to the current line's references
                onClicked: {
                    if (listView.currentIndex <= index)
                        console.log("Invalid Operation : Can only reference to smaller line numbers");
                    else if (proofModel.data(proofModel.index(listView.currentIndex,0),257) === "premise")
                        console.log("Invalid Operation: Current Line is a premise");
                    else if (proofModel.data(proofModel.index(listView.currentIndex,0),260) === true )//|| proofModel.data(proofModel.index(listView.currentIndex,0),261) === true)
                        console.log("Invalid Operation: Subproof beginning");
                    else if (proofModel.data(proofModel.index(listView.currentIndex,0),262) < model.ind && proofModel.data(proofModel.index(listView.currentIndex,0),261) === false)
                        console.log("Invalid Operation: Invalid reference to subproof");
                    else{
                        var array = Array.from(proofModel.data(proofModel.index(listView.currentIndex,0),263));
                        for(var i = 0; i < array.length; i++){
                            if (array[i] === model.line){
                                array.splice(i,1);
                                proofModel.setData(proofModel.index(listView.currentIndex,0),array,263);
                                refreshTextFieldColor();
                                return;
                            }
                        }
                        array.push(model.line);
                        proofModel.setData(proofModel.index(listView.currentIndex,0),array,263);
                        refreshTextFieldColor();
                    }
                }
            }

            // The Typing Area
            TextField{
                id: theTextID

                color: darkMode? "white": "black"
                height: font.pointSize + 10
                Layout.leftMargin: model.ind
                Layout.fillWidth: true

                background: Rectangle{
                    id: backRectID
                    border.width: 1
                    border.color: ((cConnector.evalText).includes("Error in line "+(indexx+1)+" -") && cConnector.evalText !== "Evaluate Proof")? "red" : (cConnector.evalText === "Evaluate Proof") ? ( darkMode ? "white":"black") : "springgreen"
                    color: textFieldColor
                }

                //placeholderText: indexx === 0 ? qsTr("Start Typing here..."): ""
                text: model.lText

                MouseArea{

                    anchors.fill: parent
                    //propagateComposedEvents: true
                    onClicked: {
                        listView.currentIndex = index;
                        parent.forceActiveFocus();
                        parent.cursorPosition = parent.positionAt(mouseX, mouseY);
                    }
                }

                // Click the +/- button on pressing Enter
                onAccepted: plusID.clicked()


                // Implementation for Keyboard Macros
                onTextChanged: {
                    // TODO: Improve implementation later

                    if (theTextID.length >= 2){
                        const last_two = text.slice(cursorPosition-2,cursorPosition)
                        if (last_two.includes('/\\')){
                            theTextID.remove(cursorPosition-2, cursorPosition)
                            theTextID.insert(cursorPosition,"\u2227")
                        }
                        else if (last_two.includes('\\/')){
                            theTextID.remove(cursorPosition-2, cursorPosition)
                            theTextID.insert(cursorPosition,"\u2228")
                        }
                        else if (last_two.includes('->')){
                            theTextID.remove(cursorPosition-2, cursorPosition)
                            theTextID.insert(cursorPosition,"\u2192")
                        }
                        else if (last_two.includes('<'+"\u2192")){
                            theTextID.remove(cursorPosition-2, cursorPosition)
                            theTextID.insert(cursorPosition,"\u2194")
                        }

                    }
                }

                // Save Text inside Model
                onEditingFinished: model.lText = text;

            }

            // Label for premise, subproofs
            Label{
                id: ruleID

                height: theTextID.height
                width: 50
                visible: vis

                Text{
                    anchors.centerIn: parent
                    font.italic: true
                    text: model.type
                    color: darkMode ? "white" : "black"
                    opacity: darkMode ? 0.87 : 1
                }



            }

            // First ComboBox to select rule
            ComboBox{
                id: chooseID

                palette {
                    button: darkMode ? "#CF6679" : "white"
                }

                visible: !vis
                height: theTextID.height

                model: (editCombos) ? ["Inference", "Equivalence", "Predicate", "Miscellaneous", "Boolean"]:
                                      ((["Modus Ponens", "Addition", "Simplification", "Conjunction", "Hypothetical Syllogism", "Disjunctive Syllogism", "Excluded middle", "Constructive Dilemma"].includes(conclusionRuleID.currentText))?
                                           ["Inference"]: (["Implication", "DeMorgan", "Association", "Commutativity", "Idempotence","Distribution","Equivalence","Double Negation", "Exportation", "Subsumption"].includes(conclusionRuleID.currentText))?
                                               ["Equivalence"]: (["Universal Generalization", "Universal Instantiation", "Existential Generalization", "Existential Instantiation", "Bound Variable Substitution", "Null Quantifier", "Prenex", "Identity", "Free Variable Substitution"].includes(conclusionRuleID.currentText))?
                                                                     ["Predicate"]: (["Lemma","Subproof","Sequence","Induction"].includes(conclusionRuleID.currentText))?
                                                                         ["Miscellaneous"]: ["Boolean"])

            }

            // Second ComboBox to select rule
            ComboBox{
                id: conclusionRuleID

                palette {
                    button: darkMode ? "#CF6679" : "white"
                }

                // TODO: Fix width maybe
                visible: !vis
                height: theTextID.height

                hoverEnabled: true
                ToolTip.visible: hovered
                ToolTip.text: currentText

                onActivated: {
                    editCombos = true;
                    proofModel.setData(proofModel.index(indexx,0),currentText,258)
                    asteriskID.visible = false;
                }

                model:  (editCombos)?
                            ((chooseID.currentText === "Inference")?
                                ["Modus Ponens", "Addition", "Simplification", "Conjunction", "Hypothetical Syllogism", "Disjunctive Syllogism", "Excluded middle", "Constructive Dilemma"]:
                                (chooseID.currentText === "Equivalence")?
                                    ["Implication", "DeMorgan", "Association", "Commutativity", "Idempotence","Distribution","Equivalence","Double Negation", "Exportation", "Subsumption"]:
                                    (chooseID.currentText === "Predicate")?
                                        ["Universal Generalization", "Universal Instantiation", "Existential Generalization", "Existential Instantiation", "Bound Variable Substitution", "Null Quantifier", "Prenex", "Identity", "Free Variable Substitution"]:
                                        (chooseID.currentText === "Miscellaneous")?
                                            ["Lemma","Subproof","Sequence","Induction"]:
                                            ["Identity","Negation","Dominance","Symbol Negation"]):
                                                [type]

            }


            // Display Asterisk next to ComboBox if rule not chosen
            Label{
                id: asteriskID

                property string toolTipText: "Rule Not Chosen"

                text: "*"
                font.bold: true

                height: theTextID.height
                width: 50
                visible: !vis && editCombos

                ToolTip.visible: toolTipText ? mID.containsMouse : false
                ToolTip.text: toolTipText

                MouseArea{
                    id: mID
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }


            // Row of references
            Row{
                id: refID

                Repeater{
                    id: repID
                    model: arr

                    onModelChanged: {
                        root_delegate.refreshTextFieldColor();
                    }

                    Button{
                        visible: (modelData === -1)? false: true

                        palette {
                            button: darkMode? "#1F1A24" : "white"
                        }

                        onClicked: {
                            var ar = Array.from(arr)
                            ar.splice(index,1)
                            var ok = parent.parent.parent;
                            proofModel.setData(proofModel.index(listView.currentIndex,0),ar,263);
                        }

                        Text{
                            anchors.centerIn: parent
                            font.italic: true
                            text: modelData
                            color: darkMode?"white":"black"
                        }

                    }


                }

            }


            // The +/- Button
            Button{
                id: plusID

                height: theTextID.height
                palette {
                    button: darkMode? "#1F1A24" : "white"
                }

                onClicked: {
                    optionsID.open()
                }

                Text{
                    anchors.centerIn: parent
                    text: "+ / \u2013"
                    color: theTextID.color
                }

                Menu{
                    id: optionsID

                    Action{
                        text: "Add Premise"
                        onTriggered: {
                            theData.insertLine(0,1,"","premise",false,false,false,0,[-1])
                            proofModel.updateLines();
                            proofModel.updateRefs(0,true);
                            listView.currentIndex = 0;
                            cConnector.evalText = "Evaluate Proof";
                        }
                    }
                    Action{
                        text: "Add Conclusion"
                        onTriggered: {
                            theData.insertLine(index + 1,index+2,"","choose",model.sub,false,false,model.ind,[-1]);
                            proofModel.updateLines();
                            proofModel.updateRefs(index+1,true);
                            listView.currentIndex = index + 1;
                            cConnector.evalText = "Evaluate Proof";
                        }
                    }

                    Action{
                        text: "Start Subproof"
                        onTriggered:{
                            theData.insertLine(index + 1,index+2,"","sf",true,true,false,model.ind + 20,[-1]);
                            proofModel.updateLines();
                            proofModel.updateRefs(index+1,true);
                            listView.currentIndex = index + 1;
                            cConnector.evalText = "Evaluate Proof";
                        }
                    }

                    Action{
                        text: "End Subproof"
                        onTriggered: {
                            if (model.sub === false)
                                console.log("Invalid Operation: There are no ongoing SubProofs")
                            else{
                                theData.insertLine(index + 1,index+2,"","subproof",(model.ind >= 20)? true: false,false,true,model.ind -20,[-1]);
                                proofModel.updateLines();
                                proofModel.updateRefs(index+1,true);
                                listView.currentIndex = index + 1;
                                cConnector.evalText = "Evaluate Proof";
                            }
                        }
                    }

                    Action{
                        text: "Remove this Line"
                        onTriggered: {
                            if (proofModel.rowCount() > 1){
                                theData.removeLineAt(index)
                                proofModel.updateLines()
                                proofModel.updateRefs(index,false);
                            }
                            else
                                console.log("Invalid Operation: Cannot remove all Lines")
                        }
                    }
                }
            }

        }
    }

    // Background to highlight current line
    Component{
        id: highlightID

        Rectangle{
            width: (parent) ? parent.width : 0
            color: darkMode ? "#3700B3" : "lightblue"
            radius: 10
        }
    }

}

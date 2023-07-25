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
            spacing: 10
            width: (parent)? parent.width: 0
            Layout.fillWidth: true

            property bool editCombos: (!isExtFile || type === "choose")
            property var arr: model.refs
            property var type: model.type
            property int indexx: model.index
            property bool vis: type === "premise" || type === "subproof" || type === "sub-concl"

            // Line Number Button
            Button{
                id: lineNumberID

                height: theTextID.height
                width: height

                Text{
                    anchors.centerIn: parent
                    font.italic: true
                    text: model.line
                }

                // Add this button's line to the current line's references
                onClicked: {
                    if (listView.currentIndex <= index)
                        console.log("Invalid Operation : Can only reference to smaller line numbers");
                    else if (proofModel.data(proofModel.index(listView.currentIndex,0),257) === "premise")
                        console.log("Invalid Operation: Current Line is a premise");
                    else if (proofModel.data(proofModel.index(listView.currentIndex,0),260) === true || proofModel.data(proofModel.index(listView.currentIndex,0),261) === true)
                        console.log("Invalid Operation: subproof");
                    else if (proofModel.data(proofModel.index(listView.currentIndex,0),262) < model.ind)
                        console.log("Invalid Operation: invalid reference to subproof");
                    else{
                        var array = Array.from(proofModel.data(proofModel.index(listView.currentIndex,0),263));
                        for(var i = 0; i < array.length; i++){
                            if (array[i] === model.line){
                                array.splice(i,1);
                                proofModel.setData(proofModel.index(listView.currentIndex,0),array,263);
                                return;
                            }
                        }
                        array.push(model.line);
                        proofModel.setData(proofModel.index(listView.currentIndex,0),array,263);
                    }
                }
            }

            // The Typing Area
            TextField{
                id: theTextID

                height: font.pointSize + 10
                Layout.leftMargin: model.ind
                Layout.fillWidth: true
                background: Rectangle{
                    border.width: 2
                    border.color: ((cConnector.evalText).includes("Error in line "+(indexx+1)+" -") && cConnector.evalText !== "Evaluate Proof")? "red" : (cConnector.evalText === "Evaluate Proof") ? "black" : "lightgreen"
                    color: "lightgrey"
                }

//                wrapMode: TextArea.Wrap
                placeholderText: qsTr("Start Typing here...")
                text: model.lText
                MouseArea{

                    anchors.fill: parent
                    propagateComposedEvents: true
                    onClicked: {
                        listView.currentIndex = index;
                        parent.forceActiveFocus();
                        parent.cursorPosition = parent.positionAt(mouseX, mouseY);
                    }
                }

                // Click the +/- button on pressing Enter
                onAccepted: {
                    plusID.clicked()
                }

                // Implementation for Keyboard Macros
                onTextChanged: {
                    // TODO: Improve implementation later
//                    if (theTextID.length >= 2){
//                        const last_two = text.slice(theTextID.length-2)
//                        if (last_two.includes('/\\')){
//                            theTextID.remove(theTextID.length-2, theTextID.length)
//                            theTextID.insert(cursorPosition,"\u2227")
//                        }
//                        else if (last_two.includes('\\/')){
//                            theTextID.remove(theTextID.length-2, theTextID.length)
//                            theTextID.insert(cursorPosition,"\u2228")
//                        }
//                        else if (last_two.includes('->')){
//                            theTextID.remove(theTextID.length-2, theTextID.length)
//                            theTextID.insert(cursorPosition,"\u2192")
//                        }
//                        else if (last_two.includes('<'+"\u2192")){
//                            theTextID.remove(theTextID.length-2, theTextID.length)
//                            theTextID.insert(cursorPosition,"\u2194")
//                        }

//                    }
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
//                    model.pText = text;
                }

                // Save Text inside Model
                onEditingFinished: {
                    model.lText = text;
                }
            }

            Label{
                id: ruleID

                height: theTextID.height
                width: 50
                visible: vis

                Text{
                    anchors.centerIn: parent
                    font.italic: true
                    text: model.type
                }



            }

            // First ComboBox
            ComboBox{
                id: chooseID

                visible: !vis
                height: theTextID.height

                model: (editCombos) ? ["Inference", "Equivalence", "Predicate", "Miscellaneous", "Boolean"]:
                                      ((["Modus Ponens", "Addition", "Simplification", "Conjunction", "Hypothetical Syllogism", "Disjunctive Syllogism", "Excluded middle", "Constructive Dilemma"].includes(conclusionRuleID.currentText))?
                                           ["Inference"]: (["Implication", "DeMorgan", "Association", "Commutativity", "Idempotence","Distribution","Equivalence","Double Negation", "Exportation", "Subsumption"].includes(conclusionRuleID.currentText))?
                                               ["Equivalence"]: (["Universal Generalization", "Universal Instantiation", "Existential Generalization", "Existential Instantiation", "Bound Variable Substitution", "Null Quantifier", "Prenex", "Identity", "Free Variable Substitution"].includes(conclusionRuleID.currentText))?
                                                                     ["Predicate"]: (["Lemma","Subproof","Sequence","Induction"].includes(conclusionRuleID.currentText))?
                                                                         ["Miscellaneous"]: ["Boolean"])
            }

            // Second ComboBox
            ComboBox{
                id: conclusionRuleID

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
                text: "*"
                font.bold: true

                height: theTextID.height
                width: 50
                visible: !vis && editCombos

                property string toolTipText: "Rule Not Chosen"
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

                Repeater{

                    model: arr

                    Button{
                        visible: (modelData === -1)? false: true
                        text: modelData
                        onClicked: {
                            var ar = Array.from(arr)
                            ar.splice(index,1)
                            proofModel.setData(proofModel.index(listView.currentIndex,0),ar,263);

                        }

                    }

                }

            }


            // The +/- Button
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
                            theData.insertLine(0,1,"","premise",false,false,false,0,[-1])
                            proofModel.updateLines();
                            listView.currentIndex = 0;
                            cConnector.evalText = "Evaluate Proof";
                        }
                    }
                    Action{
                        text: "Add Conclusion"
                        onTriggered: {
                            theData.insertLine(index + 1,index+2,"","choose",model.sub,false,false,model.ind,[-1]);
                            proofModel.updateLines();
                            listView.currentIndex = index + 1;
                            cConnector.evalText = "Evaluate Proof";
                        }
                    }

                    Action{
                        text: "Start Subproof"
                        onTriggered:{
                            theData.insertLine(index + 1,index+2,"","subproof",true,true,false,model.ind + 20,[-1]);
                            proofModel.updateLines();
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
                                theData.insertLine(index + 1,index+2,"","sub-concl",(model.ind >= 20)? true: false,false,true,model.ind -20,[-1]);
                                proofModel.updateLines();
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
            width: parent.width
            color: "wheat"
            radius: 10
        }
    }

}

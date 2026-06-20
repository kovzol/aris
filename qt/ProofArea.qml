
/* The proof area containing textfields, labels, rule combo boxes etc.

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
import proof.model 1.0
import "RuleAbbreviations.js" as Abbrevs

Item {
    id: rootProofArea

    property var selectedIndices: []
    property int lastSelectedIndex: -1

    // Returns a human-readable string listing every conclusion line whose
    // refs array contains `lineNum` (1-based).  Empty string = no incoming refs.
    function findIncomingRefs(lineNum) {
        var holders = []
        var n = proofModel.rowCount()
        for (var i = 0; i < n; i++) {
            var refs = proofModel.data(proofModel.index(i, 0), 263)  // RefsRole
            if (refs && Array.from(refs).indexOf(lineNum) !== -1)
                holders.push(proofModel.data(proofModel.index(i, 0), 256) + 1)  // LineRole (1-based display)
        }
        return holders.length === 0 ? "" : holders.join(", ")
    }

    // Edge-case conversion: move premise at myIdx to the boundary, then convert.
    // Called when the line is NOT the last premise (requires a physical move).
    function doConvertPremise(myIdx) {
        var pText   = proofModel.data(proofModel.index(myIdx, 0), 257)  // TextRole
        var pSub    = proofModel.data(proofModel.index(myIdx, 0), 259)  // SubRole
        var pSubSt  = proofModel.data(proofModel.index(myIdx, 0), 260)  // SubStartRole
        var pSubEnd = proofModel.data(proofModel.index(myIdx, 0), 261)  // SubEndRole
        var pInd    = proofModel.data(proofModel.index(myIdx, 0), 262)  // IndentRole

        theData.removeLineAt(myIdx)
        proofModel.updateLines()
        proofModel.updateRefs(myIdx, false)

        // After removal the boundary shifts to premiseCount - 1.
        var insertAt = proofModel.premiseCount - 1
        theData.insertLine(insertAt, insertAt + 1, pText, "choose",
                           pSub, pSubSt, pSubEnd, pInd, [-1])
        proofModel.updateLines()
        proofModel.updateRefs(insertAt, true)
        listView.currentIndex = insertAt
        proofModel.premiseCount = proofModel.premiseCount - 1

        fileModified = true
        cConnector.evalText = "Evaluate Proof"
        proofModel.clearErrors()
    }

    // Edge-case conversion: move conclusion at myIdx to the boundary, then convert.
    // Called when the line is NOT the first conclusion (requires a physical move).
    function doConvertConclusion(myIdx) {
        var cText = proofModel.data(proofModel.index(myIdx, 0), 257)  // TextRole

        theData.removeLineAt(myIdx)
        proofModel.updateLines()
        proofModel.updateRefs(myIdx, false)

        var insertAt2 = proofModel.premiseCount
        theData.insertLine(insertAt2, insertAt2 + 1, cText, "premise",
                           false, false, false, 0, [-1])
        proofModel.updateLines()
        proofModel.updateRefs(insertAt2, true)
        listView.currentIndex = insertAt2
        proofModel.premiseCount = proofModel.premiseCount + 1

        fileModified = true
        cConnector.evalText = "Evaluate Proof"
        proofModel.clearErrors()
    }

    property var combo2: [
        [qsTr("Modus Ponens"), qsTr("Addition"), qsTr("Simplification"), qsTr("Conjunction"), qsTr("Hypothetical Syllogism"), qsTr("Disjunctive Syllogism"), qsTr("Excluded middle"), qsTr("Constructive Dilemma"), qsTr("XOR Introduction"), qsTr("XOR Elimination")],
        [qsTr("Implication"), qsTr("DeMorgan"), qsTr("Association"), qsTr("Commutativity"), qsTr("Idempotence"), qsTr("Distribution"), qsTr("Equivalence"), qsTr("Double Negation"), qsTr("Exportation"), qsTr("Subsumption"), qsTr("Contrapositive")],
        [qsTr("Universal Generalization"), qsTr("Universal Instantiation"), qsTr("Existential Generalization"), qsTr("Existential Instantiation"), qsTr("Bound Variable Substitution"), qsTr("Null Quantifier"), qsTr("Prenex"), qsTr("Identity"), qsTr("Free Variable Substitution")],
        [qsTr("Lemma"), qsTr("Subproof"), qsTr("Sequence"), qsTr("Induction")],
        [qsTr("Identity "), qsTr("Negation"), qsTr("Dominance"), qsTr("Symbol Negation")]
    ]
    anchors.fill: parent

    Shortcut {
        sequences: ["Ctrl+A", "Meta+A"]
        context: Qt.ApplicationShortcut
        onActivated: {
            var all = []
            for (var i = 0; i < listView.count; i++) {
                all.push(i)
            }
            rootProofArea.selectedIndices = all
            rootProofArea.lastSelectedIndex = listView.count > 0 ? listView.count - 1 : -1
            
            // Defocus any active text field so the selection is visually obvious
            rootProofArea.forceActiveFocus()
        }
    }

    Shortcut {
        sequences: [ StandardKey.Cancel ] // Maps to Escape
        onActivated: {
            if (rootProofArea.selectedIndices.length > 0) {
                rootProofArea.selectedIndices = []
                rootProofArea.lastSelectedIndex = -1
                rootProofArea.forceActiveFocus()
            }
        }
    }

    // Right-click context menu (shared, one instance) 

    // Target row is stored in contextMenuTargetIdx before popup() is called.
    property int contextMenuTargetIdx: -1

    Menu {
        id: lineContextMenuID

        palette {
            base: darkMode ? "#1F1A24" : "white"
            text: darkMode ? "white" : "black"
        }

        //  Convert 

        Action {
            id: ctxConvertAction

            text: {
                if (rootProofArea.contextMenuTargetIdx < 0) return qsTr("Convert")
                var t = proofModel.data(proofModel.index(rootProofArea.contextMenuTargetIdx, 0), 258)
                return t === "premise" ? qsTr("Convert to Conclusion") : qsTr("Convert to Premise")
            }

            enabled: {
                var idx = rootProofArea.contextMenuTargetIdx
                if (idx < 0) return false
                var t = proofModel.data(proofModel.index(idx, 0), 258)
                if (t === "sf" || t === "subproof") return false
                if (t === "premise") return proofModel.premiseCount > 1
                return true  // conclusions can always be converted
            }

            onTriggered: {
                var myIdx  = rootProofArea.contextMenuTargetIdx
                var myType = proofModel.data(proofModel.index(myIdx, 0), 258)
                var myLine = proofModel.data(proofModel.index(myIdx, 0), 256)  // pLine is already 1-based

                if (myType === "premise") {
                    // Always physically move to the first-conclusion slot.
                    var holders = rootProofArea.findIncomingRefs(myLine)
                    if (holders !== "") {
                        convertWarningID.pendingIdx  = myIdx
                        convertWarningID.pendingType = myType
                        convertWarningID.refHolders  = holders
                        convertWarningID.open()
                    } else {
                        rootProofArea.doConvertPremise(myIdx)
                    }
                } else {
                    // Always physically move to just after the last premise.
                    rootProofArea.doConvertConclusion(myIdx)
                }
            }
        }

        MenuSeparator {}

        // Add Premise above 
        Action {
            text: qsTr("Add Premise Above")
            onTriggered: {
                var myIdx = rootProofArea.contextMenuTargetIdx
                if (myIdx < 0) return
                var insertIndex = (myIdx < proofModel.premiseCount) ? myIdx : proofModel.premiseCount
                theData.insertLine(insertIndex, insertIndex + 1, "", "premise",
                                   false, false, false, 0, [-1])
                proofModel.updateLines()
                proofModel.updateRefs(insertIndex, true)
                listView.currentIndex = insertIndex
                proofModel.premiseCount = proofModel.premiseCount + 1
                fileModified = true
                cConnector.evalText = "Evaluate Proof"
                proofModel.clearErrors()
            }
        }

        // Add Conclusion below 
        Action {
            text: qsTr("Add Conclusion Below")
            onTriggered: {
                var myIdx = rootProofArea.contextMenuTargetIdx
                if (myIdx < 0) return
                var curSub = proofModel.data(proofModel.index(myIdx, 0), 259)
                var curInd = proofModel.data(proofModel.index(myIdx, 0), 262)
                theData.insertLine(myIdx + 1, myIdx + 2, "", "choose",
                                   curSub, false, false, curInd, [-1])
                proofModel.updateLines()
                proofModel.updateRefs(myIdx + 1, true)
                listView.currentIndex = myIdx + 1
                fileModified = true
                cConnector.evalText = "Evaluate Proof"
                proofModel.clearErrors()
            }
        }

        MenuSeparator {}

        // Remove line 
        Action {
            text: "Remove This Line"
            onTriggered: {
                var myIdx  = rootProofArea.contextMenuTargetIdx
                if (myIdx < 0) return
                var myType = proofModel.data(proofModel.index(myIdx, 0), 258)

                if (listView.count > 1) {
                    if (myType === "premise")
                        proofModel.premiseCount = proofModel.premiseCount - 1
                    theData.removeLineAt(myIdx)
                    proofModel.updateLines()
                    proofModel.updateRefs(myIdx, false)
                    listView.currentIndex = Math.min(myIdx, listView.count - 1)
                } else {
                    theData.removeLineAt(0)
                    theData.insertLine(0, 1, "", "premise", false, false, false, 0, [-1])
                    proofModel.premiseCount = 1
                    proofModel.updateLines()
                    listView.currentIndex = 0
                }
                fileModified = true
                cConnector.evalText = "Evaluate Proof"
                proofModel.clearErrors()
            }
        }
    }

    function resetViewState() {
        listView.currentIndex = 0
        listView.positionViewAtBeginning()
    }


    // Shared warning dialog for the edge/move conversion case 
    // lives here (one instance) rather than inside each per line delegate.
    // styled to match importBehaviorID in main.qml.
    Dialog {
        id: convertWarningID

        property int    pendingIdx:  -1
        property string pendingType: ""
        property string refHolders:  ""

        width: Math.min(rootID.width * 0.42, 440)
        anchors.centerIn: parent

        parent: Overlay.overlay
        modal: true
        closePolicy: Popup.CloseOnEscape
        padding: 20

        Overlay.modal: Rectangle {
            color: darkMode ? "#66121212" : "#66CFCFCF"
        }

        background: Rectangle {
            radius: 12
            color: darkMode ? "#1F1B24" : "white"
            border.width: 1
            border.color: darkMode ? "#50485A" : "#D9D9D9"
        }

        contentItem: ColumnLayout {
            width: convertWarningID.availableWidth
            spacing: 20

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                font.bold: true
                color: darkMode ? "white" : "black"
                text: qsTr("Confirm Conversion")
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                color: darkMode ? "white" : "black"
                text: "Line(s) " + convertWarningID.refHolders
                      + " reference this line.\nMoving it to the boundary will remove those references."
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 14

                Button {
                    text: qsTr("Cancel")
                    Layout.fillWidth: true

                    palette {
                        button: darkMode ? "#2A2631" : "white"
                        buttonText: darkMode ? "white" : "black"
                    }

                    onClicked: convertWarningID.close()
                }

                Button {
                    text: qsTr("Convert Anyway")
                    Layout.fillWidth: true

                    palette {
                        button: darkMode ? "#2A2631" : "white"
                        buttonText: darkMode ? "white" : "black"
                    }

                    onClicked: {
                        convertWarningID.close()
                        if (convertWarningID.pendingType === "premise")
                            rootProofArea.doConvertPremise(convertWarningID.pendingIdx)
                        else
                            rootProofArea.doConvertConclusion(convertWarningID.pendingIdx)
                    }
                }
            }
        }
    }

    MouseArea {
        anchors.fill: proofAreaID
        z: -1
        onClicked: {
            if (rootProofArea.selectedIndices.length > 0) {
                rootProofArea.selectedIndices = []
                rootProofArea.lastSelectedIndex = -1
                rootProofArea.forceActiveFocus()
            }
        }
    }

    ColumnLayout {
        id: proofAreaID

        anchors {
            fill: parent
            leftMargin: keyboardID.width + scaledSpacing * 2
            topMargin: scaledSpacing * 2
            rightMargin: scaledSpacing * 2
        }

        ListView {
            id: listView

            model: proofModel
            delegate: proofLineID
            highlight: highlightID

            currentIndex: -1

            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: scaledSpacing
            ScrollBar.vertical: ScrollBar {}

            onCurrentItemChanged: {
                if (currentItem)
                    currentItem.children[1].forceActiveFocus()
            }
        }
    }

    Component {
        id: proofLineID

        Column {
            id: outerColumn
            width: parent ? parent.width : 0
            spacing: 0

            // Properties here so ALL descendants (RowLayout + Text) can access by bare name
            property bool editCombos: (!isExtFile || type === "choose")
            property var arr: model.refs
            property string type: model.type
            property int indexx: model.index
            property bool vis: type === "premise" || type === "subproof"
                               || type === "sf"
            property string textFieldColor: {
                if (rootProofArea.selectedIndices.includes(indexx)) {
                    return darkMode ? "#5C469C" : "#E6E6FA"
                }
                if (listView.currentIndex !== -1) {
                    var currentRefs = proofModel.data(proofModel.index(listView.currentIndex, 0), 263)
                    return (currentRefs && currentRefs.includes(model.line)) ? (darkMode ? "brown" : "yellow") : (darkMode ? "#1F1A24" : "white")
                }
                return darkMode ? "#332940" : "lightgrey"
            }

            // Function to refresh the line color after selecting/de-selecting references
            function refreshTextFieldColor() {
                var temp = listView.currentIndex
                listView.currentIndex = -1
                listView.currentIndex = temp
            }

            RowLayout {
                id: root_delegate
                spacing: scaledSpacing
                width: parent.width
                Layout.fillWidth: true

            // Line Number Button
            Button {
                id: lineNumberID

                Layout.preferredHeight: theTextID.height
                // Content-aware width: at least as tall as it is wide (square),
                // but expands for 2-digit line numbers at high zoom.
                Layout.preferredWidth: Math.max(height, lineNumTextID.implicitWidth + scaledSpacing)
                palette {
                    button: darkMode ? "#1F1A24" : "white"
                }

                Text {
                    id: lineNumTextID
                    anchors.centerIn: parent
                    font.italic: true
                    font.pointSize: scaledFontSize
                    text: model.line
                    color: theTextID.color
                }

                // Add this button's line to the current line's references
                onClicked: {
                    if (listView.currentIndex <= index)
                        console.log("Invalid Operation : Can only reference to smaller line numbers")
                    else if (proofModel.data(proofModel.index(
                                                 listView.currentIndex, 0),
                                             257) === "premise")
                        console.log("Invalid Operation: Current Line is a premise")
                    else if (proofModel.data(proofModel.index(
                                                 listView.currentIndex, 0),
                                             260) === true)
                        //|| proofModel.data(proofModel.index(listView.currentIndex,0),261) === true)
                        console.log("Invalid Operation: Subproof beginning")
                    else if (proofModel.data(
                                 proofModel.index(listView.currentIndex, 0),
                                 262) < model.ind && proofModel.data(
                                 proofModel.index(listView.currentIndex, 0),
                                 261) === false)
                        console.log("Invalid Operation: Invalid reference to subproof")
                    else {
                        cConnector.evalText = "Evaluate Proof"
                            proofModel.clearErrors()
                        var array = Array.from(proofModel.data(
                                                   proofModel.index(
                                                       listView.currentIndex,
                                                       0), 263))
                        for (var i = 0; i < array.length; i++) {
                            if (array[i] === model.line) {
                                array.splice(i, 1)
                                proofModel.setData(proofModel.index(
                                                       listView.currentIndex,
                                                       0), array, 263)
                                refreshTextFieldColor()
                                return
                            }
                        }
                        array.push(model.line)
                        proofModel.setData(proofModel.index(
                                               listView.currentIndex, 0),
                                           array, 263)
                        refreshTextFieldColor()
                    }
                }
            }

            // The Typing Area
            TextField {
                id: theTextID

                color: darkMode ? "white" : "black"
                height: scaledFontSize + scaledSpacing
                font.pointSize: scaledFontSize
                Layout.leftMargin: model.ind
                Layout.fillWidth: true

                Keys.onPressed: (event) => {
                    if ((event.modifiers & Qt.ControlModifier || event.modifiers & Qt.MetaModifier) && event.key === Qt.Key_A) {
                        var all = []
                        for (var i = 0; i < listView.count; i++) {
                            all.push(i)
                        }
                        rootProofArea.selectedIndices = all
                        rootProofArea.lastSelectedIndex = listView.count > 0 ? listView.count - 1 : -1
                        rootProofArea.forceActiveFocus()
                        event.accepted = true
                    }
                }

                background: Rectangle {
                    id: backRectID
                    border.width: 1
                    border.color: {
                        if (cConnector.evalText === "Evaluate Proof")
                            return darkMode ? "white" : "black"
                        if (model.errMsg !== "")
                            return "red"
                        return "springgreen"
                    }
                    color: textFieldColor
                }

                //placeholderText: indexx === 0 ? qsTr("Start Typing here..."): ""
                text: model.lText

                MouseArea {

                    anchors.fill: parent
                    
                    property int dragStartIndex: -1
                    
                    onPressed: (mouse) => {
                        if (mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier || mouse.modifiers & Qt.MetaModifier) {
                            dragStartIndex = -1
                        } else {
                            dragStartIndex = indexx
                        }
                    }
                    
                    onPositionChanged: (mouse) => {
                        if (dragStartIndex !== -1) {
                            var mappedPos = mapToItem(listView.contentItem, mouse.x, mouse.y)
                            var currentIndexHovered = listView.indexAt(mappedPos.x, mappedPos.y)

                            if (currentIndexHovered !== -1) {
                                var start = Math.min(dragStartIndex, currentIndexHovered)
                                var end = Math.max(dragStartIndex, currentIndexHovered)

                                var newSel = []
                                for (var i = start; i <= end; i++) {
                                    newSel.push(i)
                                }

                                if (rootProofArea.lastSelectedIndex !== currentIndexHovered || rootProofArea.selectedIndices.length !== newSel.length) {
                                    rootProofArea.selectedIndices = newSel
                                    rootProofArea.lastSelectedIndex = currentIndexHovered
                                }
                            }
                        }
                    }

                    onReleased: (mouse) => {
                        dragStartIndex = -1
                    }
                    
                    //propagateComposedEvents: true
                    onClicked: (mouse) => {
                        if (mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.MetaModifier) {
                            var sel = Array.from(rootProofArea.selectedIndices)
                            var pos = sel.indexOf(indexx)
                            if (pos !== -1) {
                                sel.splice(pos, 1)
                            } else {
                                sel.push(indexx)
                            }
                            rootProofArea.selectedIndices = sel
                            rootProofArea.lastSelectedIndex = indexx
                            parent.forceActiveFocus()
                        } else if (mouse.modifiers & Qt.ShiftModifier) {
                            var start = rootProofArea.lastSelectedIndex !== -1 ? rootProofArea.lastSelectedIndex : 0
                            var end = indexx
                            if (start > end) {
                                var temp = start; start = end; end = temp;
                            }
                            var sel2 = Array.from(rootProofArea.selectedIndices)
                            for (var i = start; i <= end; i++) {
                                if (sel2.indexOf(i) === -1) sel2.push(i)
                            }
                            rootProofArea.selectedIndices = sel2
                            rootProofArea.lastSelectedIndex = indexx
                            parent.forceActiveFocus()
                        } else {
                            rootProofArea.selectedIndices = []
                            rootProofArea.lastSelectedIndex = indexx
                            listView.currentIndex = index
                            parent.forceActiveFocus()
                            parent.cursorPosition = parent.positionAt(mouse.x, mouse.y)
                        }
                    }
                }

                // Click the +/- button on pressing Enter
                onAccepted: plusID.clicked()

                // Implementation for Keyboard Macros
                onTextChanged: {

                    // TODO: Improve implementation later
                    if (theTextID.length >= 2) {
                        const last_two = text.slice(cursorPosition - 2,
                                                    cursorPosition)
                        if (last_two.includes('/\\')) {
                            theTextID.remove(cursorPosition - 2, cursorPosition)
                            theTextID.insert(cursorPosition, "\u2227")
                        } else if (last_two.includes('\\/')) {
                            theTextID.remove(cursorPosition - 2, cursorPosition)
                            theTextID.insert(cursorPosition, "\u2228")
                        } else if (last_two.includes('->')) {
                            theTextID.remove(cursorPosition - 2, cursorPosition)
                            theTextID.insert(cursorPosition, "\u2192")
                        } else if (last_two.includes('<' + "\u2192")) {
                            theTextID.remove(cursorPosition - 2, cursorPosition)
                            theTextID.insert(cursorPosition, "\u2194")
                        }
                    }
                }

                // Save Text inside Model
                onEditingFinished: {
                    if (model.lText !== text) {
                        fileModified = true
                    }
                    model.lText = text
                }
            }

            // Label for premise, subproofs
            Label {
                id: ruleID

                height: theTextID.height
                // Cap width so it never squeezes the TextField;
                // shorter  text at high zoom via the text binding below.
                width: Math.min(implicitWidth + 8, 80 * Math.min(zoomFactor, 1.5))
                visible: vis
                clip: true

                font.italic: true
                font.pointSize: scaledFontSize
                // Short form when zoomed in (>150%) so label stays compact
                text: {
                    if (zoomFactor > 1.5) {
                        if (model.type === "premise")  return "P"
                        if (model.type === "subproof") return "SP"
                        if (model.type === "sf")       return "SF"
                    }
                    if (model.type === "premise")  return qsTr("premise")
                    if (model.type === "subproof") return qsTr("subproof")
                    if (model.type === "sf")       return qsTr("sf")
                    return model.type
                }
                color: darkMode ? "white" : "black"
                opacity: darkMode ? 0.87 : 1

                // Tooltip shows full word when abbreviated
                ToolTip.visible: zoomFactor > 1.5 && ruleHoverID.containsMouse
                ToolTip.text: model.type
                MouseArea { id: ruleHoverID; anchors.fill: parent; hoverEnabled: true }
            }

            // First ComboBox to select rule
            ComboBox {
                id: chooseID

                palette {
                    button: darkMode ? "#CF6679" : "white"
                    buttonText: "black"
                    window: darkMode ? "#CF6679" : "white"
                    base: darkMode ? "#CF6679" : "white"
                    text: "black"
                }

                visible: !vis
                Layout.preferredHeight: theTextID.height
                // Width tracks the scaled font; stays compact via capped font above 150%
                Layout.preferredWidth: implicitWidth
                font.pointSize: zoomFactor > 1.5
                                ? Math.round(scaledFontSize * 0.8)
                                : scaledFontSize

                // Short display labels above 150% zoom; full names still in the popup
                displayText: {
                    if (zoomFactor > 1.5) {
                        var shorts1 = ["Inf", "Eq", "Pred", "Misc", "Bool"]
                        return shorts1[currentIndex] ?? currentText
                    }
                    return currentText
                }

                hoverEnabled: true
                ToolTip.visible: zoomFactor > 1.5 && hovered
                ToolTip.text: currentText

                onActivated: {
                    editCombos = true
                    proofModel.setData(proofModel.index(indexx, 0),
                                       conclusionRuleID.currentText, 258)
                    asteriskID.visible = false
                }

                model: [qsTr("Inference"), qsTr("Equivalence"), qsTr("Predicate"), qsTr("Miscellaneous"), qsTr("Boolean")]

                Component.onCompleted: {
                    if (!editCombos) {
                        if (combo2[0].includes(type))
                            currentIndex = 0
                        else if (combo2[1].includes(type))
                            currentIndex = 1
                        else if (combo2[2].includes(type))
                            currentIndex = 2
                        else if (combo2[3].includes(type))
                            currentIndex = 3
                        else
                            currentIndex = 4
                    }
                }
            }

            // Second ComboBox to select rule
            ComboBox {
                id: conclusionRuleID

                palette {
                    button: darkMode ? "#CF6679" : "white"
                    buttonText: "black"
                    window: darkMode ? "#CF6679" : "white"
                    base: darkMode ? "#CF6679" : "white"
                    text: "black"
                }

                // TODO: Fix width maybe
                visible: !vis
                Layout.preferredHeight: theTextID.height
                // Width tracks the scaled font; stays compact via capped font above 150%
                Layout.preferredWidth: implicitWidth
                font.pointSize: zoomFactor > 1.5
                                ? Math.round(scaledFontSize * 0.8)
                                : scaledFontSize

                // Short display labels above 150% zoom; full names still in the popup.
                // Abbreviations are sourced from RuleAbbreviations.js (locale-invariant).
                displayText: {
                    if (zoomFactor > 1.5) {
                        var abbr = Abbrevs.get(chooseID.currentIndex, conclusionRuleID.currentIndex)
                        return abbr || currentText
                    }
                    return currentText
                }

                hoverEnabled: true
                ToolTip.visible: hovered && currentText !== ""
                ToolTip.text: currentText

                onActivated: {
                    editCombos = true
                    proofModel.setData(proofModel.index(indexx, 0),
                                       currentText, 258)
                    asteriskID.visible = false
                }

                onModelChanged: {
                    if (!editCombos) {
                        currentIndex = model.indexOf(type)
                    }
                }

                model: combo2[chooseID.currentIndex]
            }

            // Display Asterisk next to ComboBox if rule not chosen
            Label {
                id: asteriskID

                property string toolTipText: "Rule Not Chosen"

                text: "*"
                font.bold: true
                font.pointSize: scaledFontSize

                height: theTextID.height
                width: implicitWidth + 8
                visible: !vis && editCombos

                ToolTip.visible: toolTipText ? mID.containsMouse : false
                ToolTip.text: toolTipText

                MouseArea {
                    id: mID
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }

            // Row of references
            Row {
                id: refID

                Repeater {
                    id: repID
                    model: arr

                    onModelChanged: {
                        outerColumn.refreshTextFieldColor()
                    }

                    Button {
                        // Box height and width both scale with zoom
                        height: theTextID.height
                        width: Math.max(height, refNumTextID.implicitWidth + scaledSpacing)
                        visible: (modelData === -1) ? false : true

                        palette {
                            button: darkMode ? "#1F1A24" : "white"
                        }

                        onClicked: {
                            var ar = Array.from(arr)
                            ar.splice(index, 1)
                            var ok = parent.parent.parent
                            proofModel.setData(proofModel.index(
                                                   listView.currentIndex,
                                                   0), ar, 263)
                            cConnector.evalText = "Evaluate Proof"
                            proofModel.clearErrors()
                        }

                        Text {
                            id: refNumTextID
                            anchors.centerIn: parent
                            font.italic: true
                            font.pointSize: scaledFontSize
                            text: modelData
                            color: darkMode ? "white" : "black"
                        }
                    }
                }
            }

            // The +/- Button
            Button {
                id: plusID

                Layout.preferredHeight: theTextID.height
                // Extra padding so the "/" never clips at max zoom
                Layout.preferredWidth: plusTextID.implicitWidth + scaledSpacing * 4
                palette {
                    button: darkMode ? "#1F1A24" : "white"
                }

                onClicked: {
                    if (computePremise) {
                        proofModel.recomputePremiseCount()
                        computePremise = false
                    }
                    optionsID.open()
                }

                Text {
                    id: plusTextID
                    anchors.centerIn: parent
                    text: "+ / \u2013"
                    font.pointSize: scaledFontSize
                    color: theTextID.color
                }

                Menu {
                    id: optionsID

                    palette {
                        base: darkMode ? "#1F1A24" : "white"
                        text: darkMode ? "white" : "black"
                    }

                    Action {
                        text: qsTr("Add Premise")
                        onTriggered: {
                            var insertIndex = (index < proofModel.premiseCount) ? index + 1 : proofModel.premiseCount
                            theData.insertLine(insertIndex, insertIndex + 1,
                                               "", "premise", false, false,
                                               false, 0, [-1])
                            proofModel.updateLines()
                            proofModel.updateRefs(insertIndex, true)
                            listView.currentIndex = insertIndex
                            cConnector.evalText = "Evaluate Proof"
                            proofModel.clearErrors()
                            proofModel.premiseCount = proofModel.premiseCount + 1
                        }
                    }
                    Action {
                        text: qsTr("Add Conclusion")
                        enabled: index + 1 >= proofModel.premiseCount

                        onTriggered: {
                            theData.insertLine(index + 1, index + 2, "",
                                               "choose", model.sub, false,
                                               false, model.ind, [-1])
                            proofModel.updateLines()
                            proofModel.updateRefs(index + 1, true)
                            listView.currentIndex = index + 1
                            cConnector.evalText = "Evaluate Proof"
                            proofModel.clearErrors()
                        }
                    }

                    Action {
                        text: type === "premise" ? qsTr("Convert to Conclusion") : qsTr("Convert to Premise")

                        // Disabled for subproof structural lines and the
                        // last remaining premise (a proof must keep at least one).
                        enabled: {
                            if (type === "sf" || type === "subproof") return false
                            if (type === "premise") return proofModel.premiseCount > 1
                            return true  // conclusions can always be converted
                        }

                        onTriggered: {
                            var myIdx  = indexx
                            var myType = type
                            var myLine = model.line  // already 1-based

                            if (myType === "premise") {
                                // Always physically move to the first-conclusion slot.
                                // Show the warning dialog if any line references this one.
                                var holders = rootProofArea.findIncomingRefs(myLine)
                                if (holders !== "") {
                                    convertWarningID.pendingIdx  = myIdx
                                    convertWarningID.pendingType = myType
                                    convertWarningID.refHolders  = holders
                                    convertWarningID.open()
                                } else {
                                    rootProofArea.doConvertPremise(myIdx)
                                }
                            } else {
                                // Always physically move to just after the last premise.
                                rootProofArea.doConvertConclusion(myIdx)
                            }
                        }
                    }


                    Action {
                        text: qsTr("Start Subproof")
                        onTriggered: {
                            theData.insertLine(index + 1, index + 2, "", "sf",
                                               true, true, false,
                                               model.ind + 20, [-1])
                            proofModel.updateLines()
                            proofModel.updateRefs(index + 1, true)
                            listView.currentIndex = index + 1
                            cConnector.evalText = "Evaluate Proof"
                            proofModel.clearErrors()
                        }
                    }

                    Action {
                        text: qsTr("End Subproof")
                        enabled: model.sub

                        onTriggered: {

                            theData.insertLine(
                                        index + 1, index + 2, "", "subproof",
                                        (model.ind >= 20) ? true : false,
                                        false, true, model.ind - 20, [-1])
                            proofModel.updateLines()
                            proofModel.updateRefs(index + 1, true)
                            listView.currentIndex = index + 1
                            cConnector.evalText = "Evaluate Proof"
                            proofModel.clearErrors()
                        }
                    }

                    Action {
                        text: qsTr("Remove this Line")
                        enabled: true

                        onTriggered: {
                            cConnector.evalText = "Evaluate Proof"
                            proofModel.clearErrors()

                            if (listView.count > 1) {
                                if (type === "premise")
                                    proofModel.premiseCount = proofModel.premiseCount - 1

                                var i = index
                                theData.removeLineAt(index)
                                proofModel.updateLines()
                                proofModel.updateRefs(i, false)
                            } else {
                                theData.removeLineAt(0)
                                theData.insertLine(0, 1, "", "premise", false, false, false, 0, [-1])
                                proofModel.premiseCount = 1
                                proofModel.updateLines()
                                listView.currentIndex = 0
                                console.log("Goal 3: Last line reset to prevent blank screen crash.")
                            }
                        }
                    }
                }
                // Right-click overlay — lives inside RowLayout so it doesn't add
                // height to the outer Column.  No Layout.* properties = pure overlay.
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    z: 20

                    onClicked: (mouse) => {
                        if (mouse.button === Qt.RightButton) {
                            rootProofArea.contextMenuTargetIdx = indexx
                            listView.currentIndex = indexx
                            lineContextMenuID.popup()
                        }
                    }
                }
            } // end menu button
            } // end RowLayout

            // Inline error — slides in when errMsg is non-empty
            Text {
                id: errorDetailID
                visible: model.errMsg !== ""
                text: model.errMsg
                color: "#FF6B6B"
                font.pointSize: Math.max(10, scaledFontSize)
                font.italic: true
                leftPadding: scaledSpacing * 2
                wrapMode: Text.WordWrap
                width: outerColumn.width
                opacity: 0.0
                onVisibleChanged: opacity = visible ? 1.0 : 0.0
                Behavior on opacity { NumberAnimation { duration: 180 } }
            }
        } // end Column
    } // end Component

    // Background to highlight current line
    Component {
        id: highlightID

        Rectangle {
            width: (parent) ? parent.width : 0
            color: darkMode ? "#3700B3" : "lightblue"
            radius: 10
        }
    }
}

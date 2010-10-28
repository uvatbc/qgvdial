////////////////////////////////////////////////////////////////////////////////
// All helper function sit in this file
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// These functions calculates the height/width of the current component
function calcFlowChildHeight () {
    if (parent.height > parent.width) {
        return ((parent.height - parent.spacing) / 2);
    } else {
        return (parent.height - parent.spacing);
    }
}
function calcFlowChildWidth () {
    if (parent.width > parent.height) {
        return ((parent.width - parent.spacing) / 2);
    } else {
        return (parent.width - parent.spacing);
    }
}

////////////////////////////////////////////////////////////////////////////////
// These functions calculate the height and width of the buttons on the keypad
function calcDigitButtonWidth () {
    return (parent.width / layoutGrid.columns);
}
function calcDigitButtonHeight () {
    return (parent.height / layoutGrid.rows);
}

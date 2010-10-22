////////////////////////////////////////////////////////////////////////////////
// All helper function sit in this file
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// These functions calculates the height/width of the current component
function calcFlowChildHeight () {
    if (parent.height > parent.width) {
        console.debug("reduced height = " + (parent.height / 2));
        return (parent.height / 2);
    } else {
        console.debug("normal height = " + parent.height);
        return (parent.height);
    }
}
function calcFlowChildWidth () {
    if (parent.width > parent.height) {
        console.debug("reduced width = " + (parent.width / 2));
        return (parent.width / 2);
    } else {
        console.debug("normal width = " + parent.width);
        return (parent.width);
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

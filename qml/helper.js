/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

////////////////////////////////////////////////////////////////////////////////
// All helper functions and their global variables sit in this file
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Calculate the height/width of the current component
function calcFlowChildHeight () {
    if (parent.height > parent.width) {
        return (parent.height / 2);
    } else {
        return (parent.height);
    }
}
function calcFlowChildWidth () {
    if (parent.width > parent.height) {
        return ((parent.width - parent.spacing) / 2);
    } else {
        return (parent.width);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Calculate the font point from the minimum of the height and width of the button
function btnFontPoint () {
    var m = (height < width? height : width);
    m = (m + 1) / 2;
    m = (m < 1 ? 1 : m);
    return (m);
}

////////////////////////////////////////////////////////////////////////////////
// Calculate the subtext font point from the main font point
function btnSubTextFontPoint () {
    var m = (btnFontPoint() + 2) / 3;
    m = (m < 1 ? 1 : m);
    return (m);
}

// Deletion in the MainView
function doDel () {
    var origStart = txtNum.selectionStart;
    var sel = txtNum.selectionEnd - origStart;
    var result = txtNum.text.substr(0, origStart);
    if (sel == 0) {
        result = result.substr(0, origStart-1);
    }
    result += txtNum.text.substr(txtNum.selectionEnd);
    txtNum.text = result;

    if (origStart > result.length) {
        origStart = result.length;
    }

    txtNum.cursorPosition = origStart;
}

function doIns (strText) {
    var origStart = txtNum.selectionStart;
    var result = txtNum.text.substr(0, origStart);
    result += strText;
    result += txtNum.text.substr(txtNum.selectionEnd);
    txtNum.text = result;
    txtNum.cursorPosition = origStart + strText.length;
}

function getOriName (ori) {
    var name = "UnknownOrientation";
    if (ori == Orientation.Portrait) {
        name = "Portrait";
    } else if (ori == Orientation.Landscape) {
        name = "Landscape";
    } else if (ori == Orientation.PortraitInverted) {
        name = "PortraitInverted";
    } else if (ori == Orientation.LandscapeInverted) {
        name = "LandscapeInverted";
    }
    return name;
}

function getAngle(orientation) {
    var angle = 0;
    if (orientation == "Portrait") {
        angle = 0;
    } else if (orientation == "Landscape") {
        angle = 90;
    } else if (orientation == "PortraitInverted") {
        angle = 180;
    } else if (orientation == "LandscapeInverted") {
        angle = 270;
    }
    return angle;
}

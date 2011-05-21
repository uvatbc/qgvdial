import Qt 4.7

Rectangle {
    // height of the tab bar
    property int tabsHeight : 50

    // index of the active tab
    property int tabIndex : 0

    // the model used to build the tabs
    property VisualItemModel tabsModel

    anchors.fill: parent

    // will contain the tab views
    Rectangle {
        id: tabViewContainer
        width: parent.width

        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom

        // build all the tab views
        Repeater {
            model: tabsModel
        }
    }// Tab Content

    Component.onCompleted:
    {
        // hide all the tab views
        for(var i = 0; i < tabsModel.children.length; i++)
        {
            tabsModel.children[i].visible = false;
        }
        // select the default tab index
        tabClicked(tabIndex);
    }

    function tabClicked(index)
    {
        // unselect the currently selected tab
        tabs.children[tabIndex].color = "transparent";

        // hide the currently selected tab view
        tabsModel.children[tabIndex].visible = false;

        // change the current tab index
        tabIndex = index;

        // highlight the new tab
        tabs.children[tabIndex].color = "#30ffffff";

        // show the new tab view
        tabsModel.children[tabIndex].visible = true;
    }

    Component {
        id: tabBarItem

        Rectangle {
            height: tabs.height
            width: tabs.width / tabsModel.count

            color: "transparent"

            Image {
                source: tabsModel.children[index].icon
                anchors {
                    fill: parent
                    centerIn: parent
                }
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    tabClicked(index);
                }
            }
        }
    }//Component (tabBarItem)

    // the tab bar
    Rectangle {
        height: tabsHeight
        width: parent.width

        anchors {
            // take the whole parent width
            left: parent.left
            right: parent.right

            // attach it to the view top
            top: parent.top
        }

        id: tabBar

        gradient: Gradient {
            GradientStop {position: 0.0; color: "#666666"}
            GradientStop {position: 1.0; color: "#000000"}
        }

        // place all the tabs in a row
        Row {
            anchors.fill: parent

            id: tabs

            Repeater {
                model: tabsModel.count

                delegate: tabBarItem
            }
        }
    }
}

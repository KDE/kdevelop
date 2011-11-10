import QtQuick 1.0

StandardPage
{
    color: "#FFF3EC"
    
    Column {
        id: column1
        height: parent.height
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width/2
        anchors.margins: 30
        spacing: 50

        Link {
            text: qsTr("New Project")
            onClicked: kdev.retrieveMenuAction("project/project_new").trigger()
        }

        Link {
            text: qsTr("Import project")
            onClicked: ICore.projectController().openProject()
        }
    }

    ListView {
        anchors.left: column1.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 30

        delegate: Row {
                spacing: 10
//                 Rectangle {
//                     width: 40
//                     height: 40
//                     color: colorCode
//                 }

                Link {
                    function justName(str) {
                        var idx = str.indexOf(" [")
                        
                        return str.substr(0, idx);
                    }
                    font.pixelSize: 15
                    
                    text: justName(modelData["text"])
                    onClicked: modelData.trigger()
                }
            }

        model: kdev.recentProjects()
        
//         ListModel {
//             ListElement { name: "Grey"; colorCode: "grey" }
//             ListElement { name: "Red"; colorCode: "red" }
//             ListElement { name: "Blue"; colorCode: "blue" }
//             ListElement { name: "Green"; colorCode: "green" }
//         }
    }
}

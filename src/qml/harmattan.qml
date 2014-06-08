import QtQuick 1.1
import com.nokia.meego 1.0

PageStackWindow {
    id: appWindow

    initialPage: mainPage
    showStatusBar: true
    //showToolBar: true
    showToolBar: mainPage.fullScreen

    MainPage {
        id: mainPage
    }

    ToolBarLayout {
        id: commonTools
        visible: true
        ToolIcon { platformIconId: "icon-m-toolbar-back"; onClicked: Qt.quit(); }
        ToolIcon {
            platformIconId: "toolbar-view-menu"
            anchors.right: (parent === undefined) ? undefined : parent.right
            onClicked: (myMenu.status === DialogStatus.Closed) ? myMenu.open() : myMenu.close()
        }
    }

    Menu {
        id: myMenu
        visualParent: pageStack
        MenuLayout {
            MenuItem { text: mainPage.getActionText(1); onClicked: {fileSelector.down("file:///"+mainPage.dirMap); fileSelector.open();}  } //mainPage.runOpen(); }
            MenuItem { text: mainPage.getActionText(2); onClicked: mainPage.runSaveAs(); }
            //MenuItem { text: qsTr("Save scale") }
            //MenuItem { text: qsTr("Save position") }
            MenuItem { text: mainPage.getActionText(3); onClicked: mainPage.runSettings(); }
            MenuItem { text: mainPage.getActionText(4); onClicked: queryAddMap.open(); }
            MenuItem { text: mainPage.getActionText(5); onClicked: mainPage.runAbout(); }
        }
    }

    FileSelector {
     id: fileSelector;
     filterList: [ "*.pmz" ]
     titleText: mainPage.getActionText(1);
     onAccepted: mainPage.loadMap(selectedFile.replace("file://","")); // but its don't work at win
    }

    QueryDialog {
        id: queryFirstStart
        icon: ":/qmetro.png"
        titleText: "qMetro"
        message: mainPage.msgInstalledMaps
        acceptButtonText: qsTr("Yes")
        rejectButtonText: qsTr("No")
        onAccepted: queryAddMap.open();
    }

    QueryDialog {
        id: queryAddMap
        icon: "image://theme/icon-l-downloading"
        titleText: mainPage.getActionText(4); //qsTr("Add maps")
        message: mainPage.msgAddMap
        acceptButtonText: qsTr("OK")
        rejectButtonText: qsTr("Cancel")
        onAccepted: mainPage.runAddMap();
    }

    Component.onCompleted: if(mainPage.firstStart) queryFirstStart.open();
}

#folder_01.source = rc/qml
#folder_01.target = qml
#DEPLOYMENTFOLDERS = folder_01

DEFINES += QML_EDITION

QT += declarative
include(src/qmlviewer/qmlapplicationviewer.pri)
# qtcAddDeployment()

contains(MEEGO_EDITION,harmattan){
QT += opengl
CONFIG += qt-boostable qdeclarative-boostable
}

maemo5{
QT += opengl
}

RESOURCES += src/qml/qml.qrc
SOURCES += src/qmlmain.cpp



HEADERS += \
    src/qmlmainwindow.h

OTHER_FILES += \
    src/qml/universal.qml \
    src/qml/MainPage.qml \
    src/qml/harmattan.qml \
    src/qml/FileSelector.qml

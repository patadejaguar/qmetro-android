APPNAME = qMetro

TARGET = qmetro
TEMPLATE = app
DESTDIR = bin
VERSION = 0.7.1

# on/off alarm (now its used only in about ;) ):
#include(rc/module/alarm.pri)  # +multimedia

# on/off update (set your platform!):
include(rc/module/update.pri) # +network

# on/off NFC-reader (Symbian)
#include(rc/module/nfc.pri)    # +connectivity

# on/off OPENGL (work bad):
#DEFINES += OPENGL_ENABLE=1

# QTPLUGIN += qjpeg
# include(rc/locale.pri)


symbian: include(rc/platform/symbian.pri)
win32: include(rc/platform/win.pri)
macx: include(rc/platform/macx.pri)
unix:!macx:!android:!haiku:!symbian: include(rc/platform/nix.pri) # + Maemo, MeeGo

haiku{
 TARGET = qMetro
 LIBS += -lz
}


include(rc/module/main.pri)
contains(MEEGO_EDITION,harmattan){
 include(rc/module/main_quick.pri)
}else{
 include(rc/module/main_widget.pri)
}

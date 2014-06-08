greaterThan(QT_MAJOR_VERSION, 4): {
 QT += widgets
 QT -= network
}

DEFINES += VERSION=\"\\\"$$VERSION\\\"\" \
           APPNAME=\"\\\"$$APPNAME\\\"\"

INCLUDEPATH += src/zlib

SOURCES += src/mainwindow.cpp \
    src/zip/zipglobal.cpp \
    src/zip/zip.cpp \
    src/zip/unzip.cpp \
    src/mapview/mapview.cpp \
    src/mapview/maproute.cpp \
    src/mapview/mapgraphics.cpp \
    src/mapview/mapnode.cpp \
    src/mapview/mapedge.cpp \
    src/mapview/maplayer.cpp \
    src/mapview/mapelement.cpp \
    src/completer.cpp \
    src/settings.cpp \
    src/details.cpp \
    src/ini/inisettings.cpp \
    src/mobileui.cpp \
    src/flickcharm.cpp \
    src/swipegesturerecognizer.cpp

HEADERS  += src/mainwindow.h \
    src/zlib/zlib.h \
    src/zlib/zconf.h \
    src/zip/zipglobal.h \
    src/zip/zipentry_p.h \
    src/zip/zip_p.h \
    src/zip/zip.h \
    src/zip/unzip_p.h \
    src/zip/unzip.h \
    src/mapview/mapview.h \
    src/mapview/maproute.h \
    src/mapview/mapgraphics.h \
    src/mapview/mapnode.h \
    src/mapview/mapedge.h \
    src/mapview/maplayer.h \
    src/mapview/mapelement.h \
    src/completer.h \
    src/settings.h \
    src/details.h \
    src/ini/inisettings.h \
    src/mobileui.h \
    src/flickcharm.h \
    src/about.h \
    src/swipegesturerecognizer.h

FORMS    += ui/mainwindow.ui \
            ui/settings.ui \
            ui/details.ui

RESOURCES += rc/resource.qrc

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog \
    qtc_packaging/debian_fremantle/rules \
    qtc_packaging/debian_fremantle/qmetro.links \
    qtc_packaging/debian_fremantle/prerm \
    qtc_packaging/debian_fremantle/postinst \
    qtc_packaging/debian_fremantle/copyright \
    qtc_packaging/debian_fremantle/control \
    qtc_packaging/debian_fremantle/compat \
    qtc_packaging/debian_fremantle/changelog \
    rc/qmetro_meego.desktop \
    rc/qmetro.desktop \
    rc/locale.pri \
    rc/Info.plist \
    rc/icon.rc \
    rc/icon.ico \
    rc/locale/*.ts \
    rc/locale/ru.qm \
    rc/img/zoom-in.png \
    rc/img/zoom-fit-best.png \
    rc/img/system-help.png \
    rc/img/preferences-system-windows-actions.png \
    rc/img/preferences-system.png \
    rc/img/preferences-other.png \
    rc/img/preferences-desktop.png \
    rc/img/general_fullsize.png \
    rc/img/folder-favorites.png \
    rc/img/favorites.png \
    rc/img/edit-find.png \
    rc/img/edit-clear.png \
    rc/img/document-save.png \
    rc/img/document-open.png \
    rc/img/dialog-information.png \
    rc/img/bullet.png \
    rc/img/applications-system.png \
    rc/img/application-exit.png \
    rc/img/zoom-out.png \
    rc/icons/hicolor/64x64/apps/qmetro.png \
    rc/icons/hicolor/80x80/apps/qmetro.png \
    bin/locale/*.lng \
    bin/map/*.pmz \
    rc/menu_wince.res \
    rc/info_win.rc \
    rc/info_mac.plist \
    rc/icon_win.ico \
    rc/img/donate.png

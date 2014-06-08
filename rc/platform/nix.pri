 # its only for old MeeGo SDK
 exists($$QMAKE_INCDIR_QT"/../qmsystem2/qmkeys.h"):!contains(MEEGO_EDITION,harmattan): {
  MEEGO_VERSION_MAJOR     = 1
  MEEGO_VERSION_MINOR     = 2
  MEEGO_VERSION_PATCH     = 0
  MEEGO_EDITION           = harmattan
  DEFINES += MEEGO_EDITION_HARMATTAN
 }

 TARGET = qmetro
 isEmpty(PREFIX):PREFIX = /usr
 BINDIR = $$PREFIX/bin
 DATADIR = /tmp/$$APPNAME

 maemo5{
  #QT += opengl
  DEFINES +=  QMETRO_PLATFORM=\"\\\"Maemo\\\"\"

  BINDIR = /opt/maemo/usr/bin
  DATADIR = /home/user/tmp/$$APPNAME
 }

 contains(MEEGO_EDITION,harmattan){
  DEFINES +=  QMETRO_PLATFORM=\"\\\"MeeGo\\\"\"

  BINDIR = /opt/usr/bin
  DATADIR = /home/user/tmp/$$APPNAME
  DEFINES += MEEGO_EDITION_HARMATTAN
 }

 #MAKE INSTALL
 INSTALLS += target desktop icons locale map skin

 # maemo: /opt/maemo/usr/bin (+link: /usr/bin)
 # meego: /opt/usr/bin
 #  *nix: /usr/bin
 target.path =$$BINDIR

 # maemo: /usr/share/applications/hildon/qmetro.desktop
 # meego: /usr/share/applications/qmetro_meego.desktop
 #  *nix: /usr/share/applications/qmetro.desktop

 desktop.path = $$PREFIX/share/applications
 maemo5: desktop.path = $$PREFIX/share/applications/hildon
 desktop.files = rc/qmetro.desktop
 contains(MEEGO_EDITION,harmattan){
  desktop.files = rc/qmetro_meego.desktop
 }

 # /usr/share/icons/...
 icons.path = $$PREFIX/share/icons
 icons.files += rc/icons/*

 # *nix: ~/.local/share/data/qMetro (XDG data folder)
 # maemo & meego: /home/user/tmp/qMetro (install script)
 locale.path = $$DATADIR/locale
 locale.files += bin/locale/*.lng
 map.path = $$DATADIR/map
 map.files += bin/map/*.pmz
 skin.path = $$DATADIR/skin
 skin.files += bin/skin/*.zip

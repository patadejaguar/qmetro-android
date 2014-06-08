# QT += opengl
 TARGET = qMetro
 QMAKE_LFLAGS += -static-libgcc
 win32-g++:DEFINES += QT_NEEDS_QMAIN
 win32-borland:DEFINES += QT_NEEDS_QMAIN
 RC_FILE = rc/info_win.rc

DEFINES +=  QMETRO_PLATFORM=\"\\\"Windows\\\"\"

message(Update module is active)

# set PLATFORM for check update
DEFINES += QMETRO_PLATFORM=\"\\\"Source\\\"\"
# set URL for check update
#DEFINES += QMETRO_UPDATE_URL=\"\\\"http://qmetro.sf.net/update\"\\\"

DEFINES += QMETRO_HAVE_UPDATE=1


QT += network
symbian:TARGET.CAPABILITY += NetworkServices

HEADERS += \
    src/update/download.h \
    src/update/update.h

SOURCES += \
    src/update/download.cpp \
    src/update/update.cpp

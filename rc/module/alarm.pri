message(Alarm module is active)
android:haiku: error(Alarm module> This platform is not yet supported)

# on/off vibro with alarm (don't work):
#maemo5:include(src/alarm/rc/vibro.pri)

#!contains(MEEGO_EDITION,harmattan): QT += multimedia
#contains(MEEGO_EDITION,harmattan){
#CONFIG += mobility
#MOBILITY = multimedia
#}

QT += multimedia
CONFIG += mobility
MOBILITY = multimedia

unix:!macx:!maemo5:!contains(MEEGO_EDITION,harmattan):!symbian {
#QT -= multimedia
INCLUDEPATH += /usr/include/QtMultimediaKit \
               /usr/include/QtMobility
}


DEFINES += QMETRO_HAVE_ALARM=1

DEFINES += HAVE_CONFIG_H=1 \
           AC_APPLE_UNIVERSAL_BUILD=1 \
           MODPLUG_BASIC_SUPPORT=1 \
           NO_PACKING=1 \
           MODPLUG_NO_FILESAVE=1 \
           MODPLUG_FASTSOUNDLIB=1 \
           HAVE_SINF=1 \
           HAVE_STDINT_H=1 \
           HAVE_UNZIP_H=1

symbian:maemo5 {
 DEFINES += HAVE_SETENV=1


}




HEADERS       = \
    src/alarm/audiooutput.h \
    src/alarm/libmodplug/tables.h \
    src/alarm/libmodplug/stdafx.h \
    src/alarm/libmodplug/sndfile.h \
    src/alarm/libmodplug/modplug.h \
    src/alarm/libmodplug/it_defs.h \
    src/alarm/libmodplug/config.h

#    libmodplug/load_pat.h

SOURCES       = \
    src/alarm/audiooutput.cpp \
    src/alarm/libmodplug/sndmix.cpp \
    src/alarm/libmodplug/sndfile.cpp \
    src/alarm/libmodplug/snd_fx.cpp \
    src/alarm/libmodplug/snd_flt.cpp \
    src/alarm/libmodplug/snd_dsp.cpp \
    src/alarm/libmodplug/modplug.cpp \
    src/alarm/libmodplug/mmcmp.cpp \
    src/alarm/libmodplug/fastmix.cpp \
    src/alarm/libmodplug/load_xm.cpp \
    src/alarm/libmodplug/load_mod.cpp \
    src/alarm/libmodplug/load_wav.cpp \
    src/alarm/libmodplug/load_s3m.cpp \
    src/alarm/libmodplug/load_it.cpp

#    libmodplug/load_umx.cpp \
#    libmodplug/load_ult.cpp \
#    libmodplug/load_stm.cpp \
#    libmodplug/load_ptm.cpp \
#    libmodplug/load_psm.cpp \
#    libmodplug/load_pat.cpp \
#    libmodplug/load_okt.cpp \
#    libmodplug/load_mtm.cpp \
#    libmodplug/load_mt2.cpp \
#    libmodplug/load_mid.cpp \
#    libmodplug/load_med.cpp \
#    libmodplug/load_mdl.cpp \
#    libmodplug/load_j2b.cpp \
#    libmodplug/load_far.cpp \
#    libmodplug/load_dsm.cpp \
#    libmodplug/load_dmf.cpp \
#    libmodplug/load_dbm.cpp \
#    libmodplug/load_ams.cpp \
#    libmodplug/load_amf.cpp \
#    libmodplug/load_669.cpp

RESOURCES += \
    src/alarm/rc/alarm.qrc

#FORMS += \
#    src/alarm/ui/alarm.ui

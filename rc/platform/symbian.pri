TARGET = qMetro
DESTDIR =

DEFINES +=  QMETRO_PLATFORM=\"\\\"Symbian\\\"\"

TARGET.UID3 = 0x2006E50D
DEPLOYMENT.installer_header=0x2002CCCF
#TARGET.CAPABILITY = "NetworkServices Location"
TARGET.EPOCSTACKSIZE = 0x14000
TARGET.EPOCHEAPSIZE = 0x020000 0x4000000

#RSS_RULES ="group_name=\"Applications\";"

#    Localised Vendor name
#    Unique Vendor name

vendorinfo = \
        "%{\"Gusak Leonid\"}" \
        ":\"Gusak Leonid\""

vendor.pkg_prerules += vendorinfo

supported_platforms = \
    "[0x1028315F],0,0,0,{\"S60ProductID\"}" \ # Symbian^1
    "[0x20022E6D],0,0,0,{\"S60ProductID\"}" \ # Symbian^3
    "[0x2003A678],0,0,0,{\"S60ProductID\"}"   # Symbian Belle

default_deployment.pkg_prerules -= pkg_platform_dependencies
platform.pkg_prerules += supported_platforms

skin.sources = bin/skin/*.zip
skin.path = e:/Data/qMetro/skin
map.sources = bin/map/*.pmz
map.path = e:/Data/qMetro/map
locale.sources = bin/locale/*.lng
locale.path = e:/Data/qMetro/locale

DEPLOYMENT += vendor platform skin map locale

ICON = rc/icons/hicolor/scalable/apps/qmetro.svg

TRANSLATIONS += rc/locale/es.ts \
		rc/locale/fr.ts \
		rc/locale/cs.ts

isEmpty(QMAKE_LUPDATE) {
	win32:QMAKE_LUPDATE = $$[QT_INSTALL_BINS]\lupdate.exe
	else:QMAKE_LUPDATE = $$[QT_INSTALL_BINS]/lupdate
}
isEmpty(QMAKE_LRELEASE) {
	win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
	else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

updatets.commands = $$QMAKE_LUPDATE -locations none -noobsolete qmetro.pro
QMAKE_EXTRA_TARGETS += updatets

updateqm.depends = updatets
updateqm.input = TRANSLATIONS
updateqm.output = bin/locale/${QMAKE_FILE_BASE}.lng
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN}
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm

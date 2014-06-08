#include <QDebug>
#include <qglobal.h>
#include <QTextCodec>

#include "main.h"
#include "mainwindow.h"

//#include <QTime>
//#include <QDebug>
//QTime tTimer;


#ifdef Q_OS_WINCE
int WinMain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{

 TApplication a(argc, argv);

#ifdef Q_OS_SYMBIAN
 // Enable split screen keyboard
 QCoreApplication::setAttribute(Qt::AA_S60DisablePartialScreenInputMode, false);
#endif

#if QT_VERSION < 0x050000
 QTextCodec *codec = QTextCodec::codecForName("utf-8");
// QTextCodec::setCodecForLocale(codec);
 QTextCodec::setCodecForCStrings(codec);
#endif

 //tTimer.start();
 MainWindow w;
 //qDebug() << tTimer.elapsed(); return 0;

 int result = a.exec();

#if defined(Q_OS_WINCE)
 CloseHandle(mutex);
#endif
 return result;
}

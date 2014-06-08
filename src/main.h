#ifndef MAIN_H
#define MAIN_H

#include <QDebug>
#include <QApplication>

#ifdef Q_OS_SYMBIAN
#include <QSymbianEvent>
#include <w32std.h>
#endif

class TApplication : public QApplication
{
Q_OBJECT

public:
 TApplication( int argc, char** argv ) : QApplication( argc, argv ) {}

#ifdef Q_OS_SYMBIAN // Special for STUPID RULE of Nokia QA

protected:
 bool symbianEventFilter( const QSymbianEvent* symbianEvent )
 {
  const TWsEvent *event = symbianEvent->windowServerEvent();
  if(!event) return false;

  switch( event->Type() ) {
  case EEventFocusGained: {
   emit appFocusGained();
//   qDebug() << "Focus gained";
   break;
  }
  case EEventFocusLost: {
   emit appFocusLost();
//   qDebug() << "Focus lost";
   break;
  }
  default: break;
  }

  return false;
 }
public slots:
signals:
 void appFocusGained();
 void appFocusLost();
#endif // Q_OS_SYMBIAN
};



//#if defined(Q_OS_WINCE) // mutex - single app
// wchar_t uniqueAppID[MAX_PATH];
// GetModuleFileName(0, uniqueAppID, MAX_PATH);
// QString uid = QString::fromWCharArray(uniqueAppID).toLower().replace(QLatin1String("\\"), QLatin1String("_"));

// HANDLE mutex = CreateMutex(NULL, TRUE, (LPCWSTR)uid.utf16());
// if (mutex && ERROR_ALREADY_EXISTS == GetLastError()) {
//  CloseHandle(mutex);

//  UINT msgNo = RegisterWindowMessage((LPCWSTR)uid.utf16());
//  HWND aHwnd = FindWindow((LPCWSTR)QString::number(msgNo).utf16(), 0);
//  if (aHwnd)
//   SetForegroundWindow((HWND)(((ULONG)aHwnd) | 0x01));
//  return 0;
// }
//#endif // Q_OS_WINCE



#endif // MAIN_H

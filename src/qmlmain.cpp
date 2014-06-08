#include <QApplication>
#include <QPushButton>
#include <QDesktopWidget>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeItem>

#if defined(Q_WS_MAEMO_5) || defined(MEEGO_EDITION_HARMATTAN)
#include <QtOpenGL>
#endif

#include "qmlviewer/qmlapplicationviewer.h"
#include "qmlmainwindow.h"


Q_DECL_EXPORT int main(int argc, char *argv[])
{
 QScopedPointer<QApplication> app(createApplication(argc, argv));
 QScopedPointer<QmlApplicationViewer> viewer(QmlApplicationViewer::create());

 QString MainQML;
 qmlRegisterType<QMLMainWindow>("QMLMainWindow", 1, 0, "MainWindow");

 // For Maemo 5 and Symbian use screen resolution but for desktop use different size
//#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(MEEGO_EDITION_HARMATTAN)
// // Get screen dimensions
// QDesktopWidget *desktop = QApplication::desktop();
// const QRect screenRect = desktop->screenGeometry();
//#else
// // On desktop we use nHD
// QPoint topLeft(0,0);
// QSize size(360, 640);
// QRect screenRect(topLeft, size);
//#endif

#if defined(Q_WS_WIN)
 QDeclarativeContext* context = viewer->rootContext();
 context->setContextProperty("screenWidth", 640);
 context->setContextProperty("screenHeight", 480);
#endif

// // Set the screen size to QML context
// QDeclarativeContext* context = viewer->rootContext();
// context->setContextProperty("screenWidth", screenRect.width());
// context->setContextProperty("screenHeight", screenRect.height());

//  MainQML="harmattan";
 #if defined(MEEGO_EDITION_HARMATTAN)
  MainQML="harmattan";
 #else
  MainQML="universal";
 #endif

 viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
 viewer->setMainQmlFile("qrc:/"+MainQML+".qml");
 //viewer->setResizeMode(QDeclarativeView::SizeRootObjectToView);

// work bad...
// For Harmattan and N900 set OpenGL rendering
//#if defined(MEEGO_EDITION_HARMATTAN) //|| defined(Q_WS_MAEMO_5)
// QGLFormat fmt = QGLFormat::defaultFormat();
// fmt.setDirectRendering(1);
// fmt.setDoubleBuffer(1);
// QGLWidget *glWidget = new QGLWidget(fmt);
// viewer->setViewport(glWidget);
//#endif


#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(MEEGO_EDITION_HARMATTAN)
 viewer->showFullScreen();
#else
 viewer->show();
#endif

 return app->exec();
}

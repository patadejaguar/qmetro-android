#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>

#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDir>

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "ui_settings.h"
#include "settings.h"
#include "ui_details.h"
#include "details.h"
#include "about.h"

#include "mapview/mapview.h"
#include "mapview/maplayer.h"
#include "mapview/maproute.h"
#include "mapview/mapnode.h"
#include "completer.h"
#include "mobileui.h"
#include "flickcharm.h"

#include "swipegesturerecognizer.h"

//#ifdef MEEGO_EDITION_HARMATTAN
//#include <QX11Info>
//#include <X11/Xatom.h>
//#include <X11/Xlib.h>
//#endif

#ifdef Q_OS_SYMBIAN
 #define _DEF( x ) QString(x).remove('"') // Symbian 474 "\"TEXT\""
#else
 #define _DEF( x ) QString(x)
#endif

QString URL_APP="http://qmetro.sf.net";
QString URL_MAPS="http://pmetro.su/Maps.html";
QString URL_TRANSLATE="http://translate.google.com/translate?sl=ru&tl=%1&u=%2";

bool LITE=0;
int ICONSIZE_DEFAULT=24,ICONSIZE=ICONSIZE_DEFAULT; // begin size 100% scale
QString STYLE_MAP_DEFAULT=
  "QLabel,QLineEdit{color:black;font:bold;max-height:1.6em;border:0.1em solid rgba(128,128,128,50%);border-radius:0.5em;background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(245,245,245,90%),stop:1 rgba(255,255,255,80%));}"
  "QToolButton{border-image:url(:/img/button_inactive.png);height:%1px;width:%1px;background:transparent;padding:0.2em;margin-top:%7px;margin-left:%7px;} "
  "QToolButton:pressed{border-image:url(:/img/button_active.png);margin-top:%6px;margin-left:%6px;} "
  "QLabel{margin-left:%7px;} "
  "QLineEdit{padding-left:%6px;}/*%1%2%3%4%5%6%7*/",
  STYLE_MAP=STYLE_MAP_DEFAULT;

QString STYLE_MAIN_DEFAULT=
  "QToolBox::tab{padding-left:1em;text-transform:uppercase;background:palette(button);border-bottom:0.2em solid palette(highlight);border-bottom-left-radius:0.5em;border-bottom-right-radius:0.5em;}"
  "QToolBox::tab:selected{font:bold;border-bottom:0.3em solid palette(highlight);}/*%1%2%3%4%5%6%7*/",
        STYLE_MAIN=STYLE_MAIN_DEFAULT,
        SKIN_DEFAULT, STYLE_FONT_DEFAULT;

MainWindow *mainWnd;

FlickCharm charm;

IniSettingMap AppConfig;

QString appPath, appConfigFile, appCachePath, appMapPath;

QTranslator appTranslator;

extern QTime testTimer;

QVector <QString> IniGroupStr;
QVector <QString> IniKeyStr;

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),map(NULL),SettingsWindow(NULL),ui(new Ui::MainWindow),mUi(NULL),Completer(NULL)
{
 qApp->setApplicationName(_DEF(APPNAME));

#if defined(Q_OS_WINCE) || defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5) || defined(MEEGO_EDITION_HARMATTAN) || defined(Q_OS_ANDROID)
 MobileVersion=1;
 qApp->setOverrideCursor(Qt::BlankCursor);
#else
 MobileVersion=0;
 LITE=0;
#endif

 //MobileVersion=1;
 //setAttribute(Qt::WA_AutoOrientation);
 setAttribute(Qt::WA_AcceptTouchEvents);
 codePage = QTextCodec::codecForName("Windows-1251");
 SKIN_DEFAULT=Config.appSkinName=this->style()->objectName().toLower();
 STYLE_FONT_DEFAULT=qApp->font().toString();
 mainWnd=this;
 IniGroupStr << "MapView"<<"Main"<<"Other";
 IniKeyStr << "Skin"<<"StationCustomFont"<<"StationFont"<<"StationBack"<<"StationShadow"<<"StationBackColor"<<"StationShadowColor"<<
              "StationUpperCase"<<"StationWordWrap"<<"StationTransparency"<<
              "MapShowScrollbar"<<"WindowFullScreen"<<"MapShowTime"<<"MapShowBackground"<<"MapAntialiasing"<<"MapFastScrolling"<<
              "MapBackColor"<<"MapSimpleTransfers"<<"MapCleanCache"<<"MapLastFile"<<"MapCaching"<<"UiScale"<<"UiButtonScale"<<"SwipeRoutes";


 appPath=qApp->applicationDirPath()+"/";
 appConfigFile=appCachePath="";

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
 QString home=qgetenv("HOME");
 appPath=QString("%1/.local/share/data/%2/").arg(home).arg(APPNAME);
 appMapPath=QString("%1/.local/share/data/%2/map/").arg(home).arg(APPNAME);
 if(!home.isEmpty())
 {
  appCachePath=QString("%1/.cache/%2/").arg(home).arg(APPNAME);
  appConfigFile=QString("%1/.config/%2/config.conf").arg(home).arg(APPNAME);
 }
#endif
#if defined(Q_OS_HAIKU)
 QString home=qgetenv("HOME");
 if(!home.isEmpty())
 {
  appConfigFile=QString("%1/config/settings/%2_settings").arg(home).arg(APPNAME);
 }
#endif
#if defined(Q_WS_MAEMO_5) || defined(MEEGO_EDITION_HARMATTAN)
 #if defined(Q_WS_MAEMO_5) && !defined(QML_EDITION)
 setAttribute(Qt::WA_Maemo5AutoOrientation);
 setAttribute(Qt::WA_Maemo5StackedWindow);
 setAttribute(Qt::WA_Maemo5NonComposited);
 #else
 MobileVersion=2;
 #endif

 appPath=QString("/home/user/MyDocs/.local/share/%1/").arg(APPNAME);
 appConfigFile=QString("/home/user/.config/%1/config.conf").arg(APPNAME);
#endif
#ifdef Q_OS_ANDROID
 appPath=QString("/sdcard/Android/data/%1/").arg(APPNAME);
 MobileVersion=2;
#endif
#ifdef Q_OS_SYMBIAN
 appPath=QString("e:/data/%1/").arg(_DEF(APPNAME));
#endif

 if(appMapPath.isEmpty()) appMapPath=appPath+"map/";
 if(appConfigFile.isEmpty()) appConfigFile=appPath+"config.ini";
 if(appCachePath.isEmpty())
 {
  if(MobileVersion)
       appCachePath=appPath+".cache/";
  else appCachePath=appPath+"cache/";
 }
 Config.appIconSize=ICONSIZE;

 QSettings *ini=new QSettings(appConfigFile,QSettings::IniFormat);
 ini->setIniCodec(codePage);
 QString key;
 ini->beginGroup("Main");
 if(_DEF(VERSION)!=ini->value("Version").toString())
 {
  removeDir(appCachePath);
  if(MobileVersion) removeDir(appPath+"cache/"); // #OLD#
 }

 Config.mapKineticScrollCheck=0; // its work so bad...
 Config.locale=ini->value("Locale","").toString();
 Config.dockWidth=ini->value("DockWidth",180).toInt();
 Config.appUiScale=ini->value(IniKeyStr[ikWUiScale],qreal(1)).toReal();
 Config.appUiButtonScale=ini->value(IniKeyStr[ikWUiButtonScale],qreal(1)).toReal();

 key="Mobile";
 if(ini->contains(key)) MobileVersion=ini->value(key).toInt();
 key=IniKeyStr[ikWFullScreen];
 if(MobileVersion && !ini->contains(key)) ini->setValue(key,1);
 Config.appFullscreen=ini->value(key,0).toBool();
 key="MapZoomFactor";
 Config.mapZoomFactor=ini->value(key,1).toReal();
 Config.mapZoomFactorCheck=ini->contains(key);
 key="MapPosition";
 Config.mapPosition=ini->value(key,QPoint()).toPoint();
 Config.mapPositionCheck=ini->contains(key);


 Config.appFirstStart=!ini->contains(IniKeyStr[ikMLastFile]);
 Config.appFont.fromString(ini->value("Font",qApp->font().toString()).toString());
 if(Config.locale.isEmpty()) Config.locale=QLocale::system().name().section("_",0,0);
 if(Config.locale=="uk" || Config.locale=="be" || Config.locale=="kk" || Config.locale=="ab" || Config.locale=="ky") Config.locale="ru";

 toggleLang(Config.locale);

 ui->setupUi(this); // UI #############################


 if(MobileVersion)
 {
  ui->menuFile->removeAction(ui->mExit);
  ui->menuFile->removeAction(ui->mClear);
  removeToolBar(ui->toolBar);

  // set UI size for device
  Config.appIconSize=ICONSIZE_DEFAULT=ICONSIZE=qMin(qApp->desktop()->width(),qApp->desktop()->height()) / qreal(6.5);
//  Config.appUiScale=1;
 }
 else
 {
  restoreState(QByteArray::fromHex(ini->value("WindowState").toByteArray()));
  key="WindowGeometry";
  if(ini->contains(key))
  {
   restoreGeometry(QByteArray::fromHex(ini->value(key).toByteArray()));
   if(windowState()==Qt::WindowFullScreen) setWindowState(Qt::WindowMaximized);
  }
  else
  {
   resize(640,480);
   setWindowState(Qt::WindowMaximized);
  }
 }

 loadSkin(ini->value("Skin").toString());
 //ui->labelLoading->setText("<H1>"+ui->labelLoading->text()+"</H1>");
 showPageLoading();

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD) || defined(Q_OS_HAIKU)
 ui->menuHelp->setIcon(QIcon());
#endif
#ifdef Q_OS_SYMBIAN
 ui->menuFile->menuAction()->setVisible(0);
 ui->menuHelp->menuAction()->setVisible(0);
 menuBar()->addActions(ui->menuFile->actions());
 menuBar()->addMenu(ui->menuSetting);
 menuBar()->addActions(ui->menuHelp->actions());
#endif
#ifdef Q_OS_ANDROID
 ui->menuSetting->removeAction(ui->menuLanguage->menuAction());
 menuBar()->removeAction(ui->menuInfo->menuAction());
 menuBar()->removeAction(ui->menuView->menuAction());
 menuBar()->removeAction(ui->menuHelp->menuAction());
 menuBar()->removeAction(ui->menuFile->menuAction());
 menuBar()->removeAction(ui->menuSetting->menuAction());
 menuBar()->addActions(ui->menuFile->actions());
 menuBar()->addMenu(ui->menuSetting);
 menuBar()->addMenu(ui->menuLanguage);
 menuBar()->addActions(ui->menuHelp->actions());
#endif

#ifdef QT_NO_DEBUG
 ui->menuView->menuAction()->setVisible(0);
 ui->menuInfo->menuAction()->setVisible(0);
 //ui->menuSetting->menuAction()->setVisible(0);
 //ui->menuHelp->addAction(ui->menuLanguage->menuAction());
#endif

if(LITE)
{
 ui->menuSetting->menuAction()->setVisible(0);
 ui->mSave->setVisible(0);
}

 qApp->setFont(Config.appFont);
 setWindowTitle(windowTitle()+(MobileVersion ? QString(""):QString(" ")+_DEF(VERSION)));
 if(Config.appFullscreen) //AppConfig.value(igMain).value(ikWFullScreen).toInt()
      toFullScreen();
 else (MobileVersion ? showMaximized():show());
 qApp->processEvents();

 ui->mSaveScale->setChecked(Config.mapZoomFactorCheck);
 ui->mSavePosition->setChecked(Config.mapPositionCheck);
 if(!Config.dockWidth) Config.dockWidth=ui->dock->width();
 ui->dock->setVisible(0);
 //ui->eSearch->setText(tr("Find"));
 //ui->eSearch->installEventFilter(this);

 map = new TMapView(appPath,appCachePath);
 map->config.MobileVersion=MobileVersion;
 map->config.locale=Config.locale;
 map->config.enableZoomKeys=ini->value("MapZoomKeys",1).toBool();
 map->params.appMapPath=appMapPath;

 connect(map,SIGNAL(trackText(QString)),ui->textDescription,SLOT(setText(QString)));
 if(!MobileVersion)
 {
  connect(map,SIGNAL(trackFinded(bool)),ui->mFind,SLOT(setChecked(bool))); // show description panel
  connect(map,SIGNAL(trackShowIndex(int)),ui->cbRoute,SLOT(setCurrentIndex(int)));
 }

 connect(map,SIGNAL(delayNames(QStringList)),this,SLOT(setDelayNames(QStringList)));
 connect(map,SIGNAL(trackNames(QStringList)),this,SLOT(setTrackNames(QStringList)));

 if(!MobileVersion)
 {
  QHBoxLayout *EditLayout = new QHBoxLayout;
  ui->bClear->setCursor(Qt::ArrowCursor);
  ui->bClear->setStyleSheet("QToolButton{border:0px;background-color:transparent;}");
  EditLayout->setAlignment(Qt::AlignRight);
  EditLayout->setMargin(3);
  EditLayout->addWidget(ui->bClear);
  ui->eSearch->setLayout(EditLayout);

  QSplitter *splitter = new QSplitter();
  splitter->addWidget(ui->dock);
//  QLabel *LoadingLabel =new QLabel(this);
//  QMovie *LoadingMovie =new QMovie(":/img/loading.gif");
//  LoadingLabel->setAlignment(Qt::AlignCenter);
//  LoadingLabel->setMovie(LoadingMovie);
//  LoadingMovie->start();
  splitter->addWidget(map);
  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);
  splitter->setChildrenCollapsible(0);
  splitter->setOpaqueResize(0);
  ui->pMap->layout()->addWidget(splitter);
  //setCentralWidget(splitter);

  QLabel *label = new QLabel("");
  label->setMinimumWidth(0);
  label->setMinimumHeight(0);
  label->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  ui->toolBar->addWidget(label);
  ui->toolBar->addAction(ui->mPreference);

 }else
 {
  resize(qApp->desktop()->width(),qApp->desktop()->height());
  ui->pMap->layout()->addWidget(map);
  //centralWidget()->layout()->addWidget(map);
  //setCentralWidget(map);
  mUi = new TMobileUi(this,map);
 }


 // apply config
 map->show();

 if(windowState()==Qt::WindowMaximized) // Under linux that need for correct FitInView
  map->resize(qApp->desktop()->availableGeometry().width(),qApp->desktop()->availableGeometry().height()-(height()-map->height())-20);

 loadSettings();

 if(Config.mapZoomFactorCheck) // load map scale
 {
  map->scaleMap(Config.mapZoomFactor);
  map->centerOn(map->scene->itemsBoundingRect().center());
 }else
  map->fitMap();

 connect(ui->cbRoute,SIGNAL(currentIndexChanged(int)),map,SLOT(showTrack(int)));
 connect(ui->mNextTrack,SIGNAL(triggered()),map,SLOT(showNextTrack()));
 connect(ui->mPreviousTrack,SIGNAL(triggered()),map,SLOT(showPreviousTrack()));

 connect(ui->cbDelay,SIGNAL(currentIndexChanged(int)),map,SLOT(setDelayIndex(int)));
 connect(ui->sbDelay,SIGNAL(valueChanged(int)),map,SLOT(setDelayMetro(int)));

 connect(ui->mZoomIn,SIGNAL(triggered()),map,SLOT(zoomIn()));
 connect(ui->mZoomOut,SIGNAL(triggered()),map,SLOT(zoomOut()));
 connect(ui->mFit,SIGNAL(triggered()),map,SLOT(fitMap()));

 if(!LITE)
 {
  map->nodePopup = new QMenu(this);
  map->nodePopup->setStyleSheet(qApp->styleSheet());
  map->nodePopup->addAction(new QAction(ui->mAbout->icon(),tr("Information"),this));
  connect(map->nodePopup->actions().at(map->nodePopup->actions().count()-1), SIGNAL(triggered()),this, SLOT(nodeInformation()));
  map->nodePopup->addSeparator();
  map->nodePopup->addAction(tr("Start"));
  QIcon fakeIcon(":/img/fake64.png");
  map->nodePopup->actions().at(map->nodePopup->actions().count()-1)->setIcon(fakeIcon);
  connect(map->nodePopup->actions().at(map->nodePopup->actions().count()-1),SIGNAL(triggered()),map,SLOT(setNodeBegin()));
  map->nodePopup->addAction(tr("Finish"));
  map->nodePopup->actions().at(map->nodePopup->actions().count()-1)->setIcon(fakeIcon);
  connect(map->nodePopup->actions().at(map->nodePopup->actions().count()-1),SIGNAL(triggered()),map,SLOT(setNodeEnd()));
  map->nodePopup->addSeparator();
  map->nodePopup->addAction(tr("Default start"));
  map->nodePopup->actions().at(map->nodePopup->actions().count()-1)->setCheckable(1);
  connect(map->nodePopup->actions().at(map->nodePopup->actions().count()-1),SIGNAL(triggered(bool)),this,SLOT(setNodeDefault(bool)));
  //need add QSignalMapper because dont know index
  connect(map->nodePopup, SIGNAL(aboutToShow()),this, SLOT(checkNodeDefault()));
  map->nodePopup->show(); map->nodePopup->hide(); // for initialize position
 }
 if(map->nodeByID.count()) Completer= new TCompleter(map,ui->eSearch);

 installEventFilter(this);

 QDir localeDir=QDir(appPath+"locale","*.lng",QDir::Name,QDir::Files);
 QStringList langNames=localeDir.entryList();

 langNames.prepend("ru.lng");
 langNames.prepend("en.lng");

 QActionGroup *langGroup = new QActionGroup(ui->menuLanguage);
 QMutableStringListIterator i(langNames);
 while (i.hasNext()) {
  i.next();
  QRegExp rx("(.+)\\.lng");
  rx.indexIn(i.value());
  QLocale locale(rx.cap(1));
  QAction *langAction = new QAction(QLocale::languageToString(locale.language()), this);
  langAction->setCheckable(true);
  if(locale.name().section("_",0,0)==Config.locale) langAction->setChecked(1);
  langAction->setData(locale.name().section("_",0,0));
  ui->menuLanguage->addAction(langAction);
  langGroup->addAction(langAction);
  connect(langAction, SIGNAL(triggered()),this, SLOT(toggleLang()));
 }
 if(MobileVersion) toggleLang(Config.locale);
 loadNodeDefault();

 showPageMap();
 if(Config.appFirstStart)
 {
  ui->mSaveScale->setChecked(1);
  ui->mSavePosition->setChecked(1);
#if !defined(QML_EDITION)
  if(QMessageBox::question(this,_DEF(APPNAME),
                           getMessageInstalledMaps(),QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
  {
   on_mAddMap_triggered();
  }
#endif
 }

#ifdef Q_OS_SYMBIAN
 // Remove context menu from all widgets ("Actions")
 QWidgetList widgets=QApplication::allWidgets();
 foreach(QWidget* w,widgets) w->setContextMenuPolicy(Qt::NoContextMenu);
 widgets.clear();
#endif


 ini->endGroup();

 if(Config.mapPositionCheck) // load map position
 {
  int xOffset, yOffset;
  xOffset=yOffset=0;
#if defined(MEEGO_EDITION_HARMATTAN)
  xOffset=-75;
  yOffset=175;
#endif
  map->horizontalScrollBar()->setValue(Config.mapPosition.x()+xOffset);
  map->verticalScrollBar()->setValue(Config.mapPosition.y()+yOffset);
 }


 // SWIPE!
 QGestureRecognizer* pRecognizer = new SwipeGestureRecognizer(qMin(qApp->desktop()->width(),qApp->desktop()->height())/qreal(1.2));
 m_gestureId = QGestureRecognizer::registerRecognizer(pRecognizer);

 // UPDATE
#ifdef QMETRO_HAVE_UPDATE
 Updater = new TUpdate;
 connect(Updater,SIGNAL(doneUpdateMaps()),this,SLOT(updateMapsAndReload()));
#endif

// NFC
#ifdef QMETRO_HAVE_NFC
 NFC = new TNFCTicket();
#endif
}

MainWindow::~MainWindow()
{
 QGestureRecognizer::unregisterRecognizer(m_gestureId);
#ifdef QMETRO_HAVE_UPDATE
 delete Updater;
#endif
#ifdef QMETRO_HAVE_NFC
 delete NFC;
#endif

 delete ui;
}

void MainWindow::setNodeDefault(bool checked,TMapNode *Node)
{
 if(!Node) Node=map->nodeClick;
 if(!Node) return;
 //qDebug() << checked << map->params.mapName << Node->Route->DisplayName << Node->DisplayName;
 QSettings ini(appConfigFile,QSettings::IniFormat);
 ini.setIniCodec(codePage);
 QString key;
 ini.beginGroup("DefaultStation");
 if(checked)
 {
  Config.mapDefaultStationID=Node->ID;
  ini.setValue(map->params.mapName,Node->Route->DisplayName+","+Node->DisplayName);
  map->setNodeBegin(Node);
 }
 else
 {
  Config.mapDefaultStationID=-1;
  ini.remove(map->params.mapName);
 }
 ini.endGroup();
}
void MainWindow::loadNodeDefault()
{
 Config.mapDefaultStationID=-1;
 QSettings ini(appConfigFile,QSettings::IniFormat);
 ini.setIniCodec(codePage);
 QStringList DefaultStation=ini.value("DefaultStation/"+map->params.mapName).toString().split(",");
 //qDebug() << DefaultStation;
 if(DefaultStation.count()==2)
 {
  TMapNode *Node=NULL;
  TMapRoute *Route=map->layer.first()->findRoute(DefaultStation.first());
  if(Route) Node=Route->findNode(DefaultStation.last());
  if(Node)
  {
   Config.mapDefaultStationID=Node->ID;
   map->setNodeBegin(Node);
  }
 }
}
void MainWindow::checkNodeDefault()
{
 char first=0;
#ifdef Q_OS_SYMBIAN
 first=2;
#endif
 map->nodePopup->actions().at(first)->setEnabled(map->nodeClick->HasInfo);
 map->nodePopup->actions().at(first)->setText(map->nodeClick->DisplayName+"...");
 map->nodePopup->actions().at(map->nodePopup->actions().count()-1)->setChecked((map->nodeClick->ID==Config.mapDefaultStationID));
 qApp->processEvents();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
 //QMainWindow::event(e);
 switch (e->type()) {
 case QEvent::LanguageChange:
  {
   ui->retranslateUi(this);
   if(SettingsWindow) SettingsWindow->ui->retranslateUi(SettingsWindow);
#ifdef Q_WS_MAEMO_5
   ui->mPreference->setText(ui->menuSetting->title());
#endif
#if defined(Q_OS_SYMBIAN) || defined(Q_OS_ANDROID)
   QList<QMenu*> menus=findChildren<QMenu*>();
   foreach(QMenu *m,menus) m->setTitle(m->title().remove("&"));
   menus.clear();
#endif
   return 1;
  }
  break;
 case QEvent::Resize:
  {
   map->mapStartDragging();
   map->selectedNodesToFront(1);
   //if(MobileVersion) Completer->popup->setStyleSheet(QString("max-width:%1px;").arg(width()-Config.appIconSize*2));
   //map->fitMap();
   return 1;
  }
  break;
 case QEvent::KeyPress:
  {
   QKeyEvent* event = (QKeyEvent*)e;
   Qt::Key key = (Qt::Key)event->key();

   if(key==Qt::Key_Escape)
   {
    //if(windowState()==Qt::WindowFullScreen)
    ui->mFullScreen->setChecked(0); //showMaximized();
    map->clearTrack();
    return 1;
   }

   if(key==Qt::Key_F1) on_mAbout_triggered();
   if(key==Qt::Key_F11) ui->mFullScreen->setChecked(windowState()!=Qt::WindowFullScreen);
   QString char_key=event->text().simplified();
   //android menu:16777301
   if(!ui->mFind->isChecked() && event->modifiers()==Qt::NoModifier && !char_key.simplified().isEmpty() && key!=16777301)//((key>=Qt::Key_A && key<=Qt::Key_Z)||(key>=Qt::Key_0 && key<=Qt::Key_9)) )
   {
    ui->mFind->setChecked(1);
    ui->eSearch->setFocus();
    ui->eSearch->setText(char_key);
   }
   return 1;
  }
  break;
 default: ;
 }


 if (obj == ui->eSearch && e->type()==QEvent::FocusIn)
 {
  //qDebug("QEvent::FocusIn event intercepted");
  ((QLineEdit *)obj)->selectAll();
  return 0;
 }
 else
  if (obj == ui->eSearch && e->type()==QEvent::FocusOut)
  {
  //qDebug("QEvent::FocusOut event intercepted");
  return 0;
 }

 return 0;
}

void MainWindow::on_mExit_triggered()
{
 close();
}

QString MainWindow::getMapDefaultDir()
{
 if(map->params.mapFile.isEmpty())
       return appMapPath;
 else  return map->params.mapFile.left(map->params.mapFile.lastIndexOf("/")+1);
}

//void MainWindow::scaleAndMoveMap(qreal scaleFactor,QPointF pos)
//{
//// if(MobileVersion && ui->stackMain->currentIndex()==1)
//// {

//// }
// map->centerOn(pos);
// map->scaleMap(scaleFactor);
//}

void MainWindow::openMap(QString fileName)
{
 QFileDialog::Options options;
#ifdef Q_OS_WINCE
 options=QFileDialog::DontUseNativeDialog;
#endif
 QString MapPath=getMapDefaultDir();

// ZIPed maps will be unpacked and updated
 updateMapsFromZips();

if(fileName.isEmpty())
 fileName = QFileDialog::getOpenFileName(0,
            tr("Open file"),MapPath,QString("%1 pMetro (*.pmz)").arg(tr("Map")),0,options);

if (fileName.isEmpty()) return;

 showPageLoading();
 qApp->processEvents();

//#ifndef QT_NO_CURSOR
// QApplication::setOverrideCursor(Qt::WaitCursor);
//#endif

//testTimer.start();
 QString TempCache=fileName.section('/', -1 );
 TempCache=appCachePath+TempCache.left(TempCache.indexOf("."))+"/";
 QFile::remove(TempCache+".cache");
 QFile::remove(TempCache+".info");
 if(Completer) delete Completer;
 map->loadMap(fileName);
 map->fitMap();
 //map->viewport()->repaint();
 ui->eSearch->setText("");
 Completer= new TCompleter(map,ui->eSearch);
//qDebug() << "Load time: " << testTimer.elapsed();

//#ifndef QT_NO_CURSOR
// QApplication::restoreOverrideCursor();
//#endif
 showPageMap();
}

void MainWindow::updateMapsFromZips()
{
 QString tmpMapPath=appMapPath+".update/";
 QDir dir(appMapPath);
 if (dir.exists())
  foreach(QFileInfo info, dir.entryInfoList(QStringList()<<"*.zip",QDir::NoDotAndDotDot | QDir::Files))
  {
//   QString mapFileName=info.absolutePath()+"/"+info.baseName()+".pmz";
//   QFile::remove(mapFileName);

   removeDir(appCachePath+info.baseName());
   map->extractFiles(info.absoluteFilePath(),"*.pmz",tmpMapPath);
   QFile::remove(info.absoluteFilePath());
  }

 dir.setPath(tmpMapPath);
 foreach(QFileInfo info, dir.entryInfoList(QStringList()<<"*.pmz",QDir::NoDotAndDotDot | QDir::Files))
 {
  QString newMapFile=appMapPath+info.fileName();
  QFile::remove(newMapFile);
  QFile::rename(tmpMapPath+info.fileName(),newMapFile);

  QFile touch(newMapFile);
  touch.open(QFile::ReadWrite);
  touch.write(QString("P").toLatin1(),1); // touch for change modify date (P -> P)
  touch.close();
 }
 removeDir(tmpMapPath);
}

void MainWindow::updateMapsAndReload()
{
 updateMapsFromZips();
 reloadMap();
}
void MainWindow::reloadMap()
{
 qreal mapZoom = map->params.mapZoomFactor;
 QPoint mapPos = QPoint(map->horizontalScrollBar()->value(),map->verticalScrollBar()->value());

 map->mapStartDragging();
 map->loadMap(map->params.mapFile);
 map->fitInView(QRectF(0,0,map->mapWidth,map->mapHeight),Qt::KeepAspectRatioByExpanding);
 map->scaleMap(mapZoom);
 map->horizontalScrollBar()->setValue(mapPos.x());
 map->verticalScrollBar()->setValue(mapPos.y());
 map->mapStopDragging();
}

void MainWindow::loadSkin(QString skinName)
{
//qDebug() << 0 << skinName << SKIN_DEFAULT << Config.appSkinName;
 skinName=skinName.toLower();
 if(skinName.isEmpty()) skinName="default";

 // unpack skins
 QString skinPath=appPath+"skin/";
 QDir Dir(skinPath);
 if(Dir.exists())
 {
  foreach(QFileInfo info, Dir.entryInfoList(QStringList()<<"*",QDir::NoDotAndDotDot | QDir::Dirs))
  {
   if(info.fileName().at(0)=='.') continue;
   QFile::rename(info.absoluteFilePath(),info.absolutePath()+"/."+info.fileName());
  }
  foreach(QFileInfo info, Dir.entryInfoList(QStringList()<<"*.zip",QDir::NoDotAndDotDot | QDir::Files))
  {
    map->extractFiles(info.absoluteFilePath(),"*",skinPath);
    QFile::remove(info.absoluteFilePath());
  }
 }

 if(Config.appSkinName==skinName)
 {
  setScaleForUi();
  return;
 }

 QString fullPath=skinPath+"."+skinName+"/";
 QString styleSheet,value,url="url(",rc=":#(",args="/*%1%2%3%4%5%6%7*/";
//qDebug() << 1;

 // load default icons
 if(actionIcons.count())
 {
  int i=0;
  QList<QAction*> actions = findChildren<QAction*>();
  foreach(QAction *a,actions) if(!a->objectName().isEmpty()) a->setIcon(actionIcons.at(i++));
  actions.clear();
//qDebug() << 1.1;
 }

 // load just style
 if(!QFile::exists(fullPath))
 {
  STYLE_MAIN=STYLE_MAIN_DEFAULT;
  STYLE_MAP=STYLE_MAP_DEFAULT;
  ICONSIZE=ICONSIZE_DEFAULT;
  if(QStyleFactory::keys().contains(skinName,Qt::CaseInsensitive))
  {
   qApp->setStyle(skinName);
   Config.appSkinName=skinName;
//   qDebug() << 1.15;
  }
  Config.appFont.fromString(STYLE_FONT_DEFAULT);
  setScaleForUi();
// qDebug() << 1.2;
  return;
 }
// qDebug() << 2;

 // load skin
 Config.appSkinName=skinName;
 QFile fileStyle;
 styleSheet="";
 fileStyle.setFileName(fullPath+"main.qss");
 fileStyle.open(QFile::ReadOnly | QFile::Text);
 styleSheet=fileStyle.readAll();
 if(styleSheet.isEmpty()) styleSheet=STYLE_MAIN_DEFAULT;
 styleSheet.replace(url+":",rc,Qt::CaseInsensitive);
 styleSheet.replace(url,url+fullPath,Qt::CaseInsensitive);
 styleSheet.replace(rc,url+":",Qt::CaseInsensitive);
 STYLE_MAIN=styleSheet+args;
 fileStyle.close();
 styleSheet="";
 fileStyle.setFileName(fullPath+"map.qss");
 fileStyle.open(QFile::ReadOnly | QFile::Text);
 styleSheet=fileStyle.readAll();
 if(styleSheet.isEmpty()) styleSheet=STYLE_MAP_DEFAULT;
 styleSheet.replace(url+":",rc,Qt::CaseInsensitive);
 styleSheet.replace(url,url+fullPath,Qt::CaseInsensitive);
 styleSheet.replace(rc,url+":",Qt::CaseInsensitive);
 STYLE_MAP=styleSheet+args;
 fileStyle.close();
 QSettings ini(fullPath+"main.ini",QSettings::IniFormat);
 ini.beginGroup("Main");
 value=ini.value("Style").toString().trimmed();
 if(!value.isEmpty()) qApp->setStyle(value);
// value=ini.value("MapColor").toString().trimmed();
// if(!value.isEmpty()) map->params.mapColorBack.setNamedColor(value);
 value=ini.value("Font",STYLE_FONT_DEFAULT).toString().trimmed();

 Config.appFont.fromString(value);

 ICONSIZE=ini.value("IconSize").toInt();
 if(!ICONSIZE) ICONSIZE=ICONSIZE_DEFAULT;

 setScaleForUi();

 ini.endGroup();



 ini.beginGroup("Icons");

 if(!actionIcons.count() && ini.childKeys().count())
 {
  QList<QAction*> actions = findChildren<QAction*>();
  foreach(QAction *a,actions) if(!a->objectName().isEmpty()) actionIcons << a->icon();
 }

 QList<QAction*> actions;
 if(ini.childKeys().count()) actions = findChildren<QAction*>();
 foreach(QString key,ini.childKeys())
 {
  value=ini.value(key).toString().trimmed();
  if(value.isEmpty()) continue;
  value=fullPath+value;
  if(QFile::exists(value))
   foreach(QAction *a,actions)
    if(a->objectName().contains(key,Qt::CaseInsensitive)) { a->setIcon(QIcon(value)); break; }
 }
 actions.clear();
 ini.endGroup();
//qDebug() << 3;
}

void MainWindow::on_mOpen_triggered()
{
 openMap();
}

void MainWindow::on_mSave_triggered()
{
 map->saveToFile();
}

void MainWindow::on_mTest_triggered()
{

}

void MainWindow::on_eSearch_returnPressed()
{
 ui->eSearch->selectAll();
}

void MainWindow::on_mFind_toggled(bool checked)
{
 ui->eSearch->setFocus(); // just need
 if(!MobileVersion) // PC
 {
  map->mapStartDragging();
  if(checked)
   ui->dock->resize(QSize(Config.dockWidth,ui->dock->height()));
  else
   Config.dockWidth=ui->dock->width();
  ui->dock->setVisible(checked);
  if(checked) ui->dock->repaint();
 }
 else
 {
  //map->setUpdatesEnabled(0);
  ui->eSearch->setVisible(checked);
  QApplication::processEvents();

  // show virtual keyboard by click...
  if(checked)
  {
   QPoint pos=QPoint(ui->eSearch->width()/2,ui->eSearch->height()/2);

   QPoint global = ui->eSearch->mapToGlobal(pos);
   QCursor::setPos(global);
   QWidget *w = qobject_cast<QLineEdit*>(this->childAt(global));
//   QWidget *w = qobject_cast<QLineEdit*>(qApp->widgetAt(global));
   if(w)
   {
    //   QWidget *w = ui->eSearch->childAt(pos);
    pos = w->mapFrom(ui->eSearch,pos);
    //qDebug() << pos << global << w;
    QMouseEvent mousePress(QEvent::MouseButtonPress, pos, Qt::LeftButton , Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(w, &mousePress);
    QMouseEvent mouseRelease(QEvent::MouseButtonRelease, pos, Qt::LeftButton , Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(w, &mouseRelease);
   }
  }
  //(checked ? ui->eSearch->setMaximumWidth(maximumWidth()):ui->eSearch->setFixedWidth(0));
  //map->setUpdatesEnabled(1);
 }
 if(checked)
{
 ui->eSearch->setFocus();
 ui->eSearch->selectAll();
}else
 map->setFocus();
}

void MainWindow::on_bClear_clicked()
{
 map->clearTrack();
 ui->eSearch->clear();
 if(!MobileVersion)
 {
  ui->eSearch->setFocus();
  ui->cbDelay->setCurrentIndex(0);
 }
 else ui->mFind->setChecked(0);
}

void MainWindow::setDelayNames(QStringList list)
{
 ui->cbDelay->clear();
 ui->cbDelay->addItems(list);
 ui->cbDelay->setCurrentIndex(0);
}


void MainWindow::setTrackNames(QStringList list)
{
 if(MobileVersion) return;

 QRegExp reCount=QRegExp(" (*)",Qt::CaseInsensitive,QRegExp::Wildcard);;
 ui->cbRoute->clear();
 ui->cbRoute->addItems(list);
 if(ui->cbRoute->count())
 {
  QString routeCount=QString(" (%1)").arg(ui->cbRoute->count());
  if(ui->gRoute->title().indexOf(reCount)==-1)
       ui->gRoute->setTitle(ui->gRoute->title()+routeCount);
  else ui->gRoute->setTitle(ui->gRoute->title().replace(reCount,routeCount));
  ui->cbRoute->setCurrentIndex(0);
  ui->cbRoute->setFocus();
 }
 else ui->gRoute->setTitle(ui->gRoute->title().replace(reCount,""));
}

void MainWindow::on_mAbout_triggered()
{
 About *a=new About(0);
 if(MobileVersion)
 {
   toFullScreen();
   a->setWindowFlags(Qt::Widget);
   ui->stackMain->addWidget(a);
   ui->stackMain->setCurrentIndex(ui->stackMain->count()-1);
   //a->showFullScreen();
 }
 else a->exec();
}

void MainWindow::on_mPreference_triggered()
{
 if(!SettingsWindow)
 {
  SettingsWindow = new Settings(this);

#ifdef Q_WS_MAEMO_5
  SettingsWindow->setAttribute(Qt::WA_Maemo5StackedWindow);
  SettingsWindow->setWindowFlags(Qt::Window);
  SettingsWindow->setAttribute(Qt::WA_Maemo5AutoOrientation);
#endif
 }

 SettingsWindow->loadSettingsToUi(AppConfig);

 if(MobileVersion)
 {
#ifndef Q_WS_MAEMO_5
  connect(SettingsWindow,SIGNAL(showed()),this,SLOT(toFullScreen()));
  connect(SettingsWindow,SIGNAL(closed()),this,SLOT(showPageMap()));
  connect(SettingsWindow,SIGNAL(closed()),SettingsWindow,SLOT(close()));
  SettingsWindow->setWindowFlags(Qt::Widget);
  ui->stackMain->addWidget(SettingsWindow);
  ui->stackMain->setCurrentIndex(ui->stackMain->count()-1);
#else
  SettingsWindow->show();
#endif


 //      MobileVersion==2 ? SettingWindow->showFullScreen():SettingWindow->showMaximized();
 }
 else
  SettingsWindow->show();
}

void MainWindow::toggleLang(QString locale)
{
 QAction *action = qobject_cast<QAction *>(sender());

 qApp->removeTranslator(&appTranslator);

 if(action || locale.isEmpty()) locale=action->data().toString();

 if(locale=="ru")
      appTranslator.load("ru.qm",":/locale");
 else appTranslator.load(locale+".lng",appPath+"locale");

 qApp->installTranslator(&appTranslator);
 Config.locale=locale;
 if(map) map->config.locale=locale;
}

void infoAddTab(QWidget *widget, const QString &Caption,const QString &Text)
{
 QTextBrowser *TextEdit=new QTextBrowser();
 if(mainWnd->MobileVersion)
 {
  QFont TempFont=qApp->font();
  mainWnd->scaleFont(TempFont,0.77);
  TextEdit->setFont(TempFont);
  QPalette pal = TextEdit->palette();
  pal.setBrush(QPalette::Base, mainWnd->palette().color(QPalette::Window));
  pal.setColor(QPalette::Text,mainWnd->palette().color(QPalette::Text));
  TextEdit->setPalette(pal);
 }

#ifndef Q_WS_MAEMO_5
 FlickCharm *fcharm = new FlickCharm(TextEdit);
 fcharm->activateOn(TextEdit);
#endif
 TextEdit->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
 TextEdit->setText(Text); 
 TextEdit->setReadOnly(1);
 TextEdit->setOpenExternalLinks(1);
 //TextEdit->setMinimumHeight(200);
 // FIX IT: double scroll. need something better then that
 TextEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Ignored);

 if(widget->metaObject()->className()==QString("QTabWidget").toLatin1())
 {
  ((QTabWidget*)widget)->addTab(TextEdit,Caption);
  if(!mainWnd->MobileVersion) TextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
 }
 else
 {
  ((QToolBox*)widget)->addItem(TextEdit,QIcon(":/img/bullet.png"),Caption);

 }

}

void MainWindow::nodeInformation()
{
 if(!map->nodeClick && !map->nodeBegin) return;
 if((MobileVersion && !map->nodeClick) || qobject_cast<QToolButton*>(sender())) map->nodeClick=map->nodeBegin;
 //qDebug() << sender();
//testTimer.start();

 Details *DetailsWindow = new Details();


 if(MobileVersion)
 {
  QString style="<table width=\"100%\" style=\"COLOR:white;BACKGROUND-COLOR:%1\"><tr><td align=\"center\"><b>&nbsp;%2</b></tr></table>";
  showPageLoading();
  qApp->processEvents();
  QFont TempFont=qApp->font();
  scaleFont(TempFont,0.8);
  DetailsWindow->ui->labelNodeName->setText(style.arg(map->nodeClick->Route->color.name()).arg(map->nodeClick->DisplayName));
  DetailsWindow->ui->labelNodeName->setFont(TempFont);
  DetailsWindow->ui->labelNodeName->setVisible(1);
  DetailsWindow->setWindowTitle(map->nodeClick->DisplayName);
 }else
 {
  DetailsWindow->ui->labelNodeName->setVisible(0);
  DetailsWindow->setWindowTitle(DetailsWindow->windowTitle()+" "+map->nodeClick->DisplayName);
  DetailsWindow->resize(640,480);
 }

#ifdef Q_WS_MAEMO_5
 DetailsWindow->setAttribute(Qt::WA_Maemo5StackedWindow);
 DetailsWindow->setWindowFlags(Qt::Window);
 DetailsWindow->setAttribute(Qt::WA_Maemo5AutoOrientation);
#endif

 map->loadNodeInfo(map->nodeClick);

 TMapView *MapScheme;
 QVBoxLayout *layout;
 QTabWidget *Tabs=DetailsWindow->ui->tabs;
 QString AllText="";
 foreach(QString key,map->nodeClick->Info.keys())
 {
  TMapNode::InfoItem Info=map->nodeClick->Info.value(key);
  if(Info.Type==TMapNode::itImage)
  {
   QStringList Value=Info.Value.split("\\n");
   foreach(QString FileName,Value)
   {
    layout = new QVBoxLayout;
    layout->setMargin(2);
    MapScheme = new TMapView(appPath,appCachePath);

    MapScheme->setParent(DetailsWindow);
    MapScheme->state.mapLoaded=0;
    MapScheme->config=map->config;
    //MapScheme->setAlignment(Qt::AlignCenter);
    //MapScheme->setTransformationAnchor(AnchorUnderMouse);
    MapScheme->params.mapColorBack=Qt::white;
    MapScheme->params.mapCachePath=map->params.mapCachePath;
    MapScheme->config.enableDrawBackground=1;
    //working bad//
    MapScheme->config.enableKineticScroll=MapScheme->config.enableCaching;
    MapScheme->config.mapLastFile="";
    MapScheme->applyConfig();

    //MapScheme->setStyleSheet(TOOLBAR_STYLE.arg(Config.appIconSize));

    map->extractFiles(map->params.mapFile,FileName,map->params.mapCachePath);
    MapScheme->loadMapImageToPaintCmd(FileName,MapScheme->paintBkg);
    MapScheme->horizontalScrollBar()->setValue(MapScheme->horizontalScrollBar()->maximum()/2);
    MapScheme->verticalScrollBar()->setValue(MapScheme->verticalScrollBar()->maximum()/2);
    MapScheme->state.mapLoaded=1;

    if(map->config.enableTranslit)
    {
     map->convertToTranslit(key);
     key=key.replace("Skhema",tr("Schema"));
    }

    Tabs->addTab(MapScheme,key);
    new TMobileUi(this,MapScheme,1);
    if(!MobileVersion)
     connect(DetailsWindow,SIGNAL(resizeWindow()),MapScheme,SLOT(fitMap()));
   }
  }else
   AllText+=QString("<h3>%1</h3><br>%2<br>").arg(key).arg(Info.Value);
 }

 int TextLength=AllText.length();
 if(TextLength>0) infoAddTab(Tabs,(MobileVersion ? tr("Information"):tr("All")),AllText);

 if(TextLength>200 && !MobileVersion)
 {
  QToolBox *Cats = new QToolBox(DetailsWindow);
  QScrollArea *Scroll=new QScrollArea(DetailsWindow);
  Tabs->addTab(Scroll,tr("Categories"));
  Scroll->setWidgetResizable(1);
  Scroll->setWidget(Cats);

  foreach(QString key,map->nodeClick->Info.keys())
  {
   TMapNode::InfoItem Info=map->nodeClick->Info.value(key);
   if(Info.Type==TMapNode::itText)
        infoAddTab(Cats,key,Info.Value);
   else continue;
  }
 }

 if(!Tabs->count())
 {
  showPageMap();
  DetailsWindow->close();
  return;
 }

 if(MobileVersion)
 {
  DetailsWindow->ui->buttonBox->setVisible(0);
  connect(DetailsWindow,SIGNAL(showed()),this,SLOT(toFullScreen()));
  connect(DetailsWindow->ui->bOK,SIGNAL(clicked()),this,SLOT(showPageMap()));
  connect(DetailsWindow->ui->bOK,SIGNAL(clicked()),DetailsWindow,SLOT(close()));
  DetailsWindow->setWindowFlags(Qt::Widget);
  ui->stackMain->addWidget(DetailsWindow);
  ui->stackMain->setCurrentIndex(ui->stackMain->count()-1);
 }
 else
 {
  DetailsWindow->ui->bOK->setVisible(0);
  //connect(DetailsWindow,SIGNAL(rejected()),this,SLOT(showPageMap()));
  DetailsWindow->showMaximized();
 }

// if(Tabs->count())
// MobileVersion==2 ? DetailsWindow->showFullScreen():DetailsWindow->showMaximized();

//qDebug() << testTimer.elapsed();
}

void MainWindow::closeEvent(QCloseEvent * e)
{
 if(!isVisible()) { e->accept(); return;} // FIX: double exec dont know why
 map->state.mapLoaded=0;
 if(ui->dock->isVisible()) Config.dockWidth=ui->dock->width();

 lower();
 hide();
 qApp->processEvents();
 QSettings ini(appConfigFile,QSettings::IniFormat);
 ini.setIniCodec(codePage);
 QString key;
 ini.beginGroup("Main");
 ini.setValue("Version", _DEF(VERSION));
 if(!MobileVersion)
 {
  ini.setValue("WindowState", saveState().toHex());
  if(!MobileVersion) ini.setValue("WindowGeometry", saveGeometry().toHex());
 }
 ini.setValue(IniKeyStr[ikMLastFile],map->params.mapFile);
 key="Locale";
 if(ini.contains(key))
  ini.setValue(key,Config.locale);
 else
  if(Config.locale!=QLocale::system().name().section("_",0,0)) ini.setValue(key,Config.locale);
 ini.setValue("DockWidth",Config.dockWidth);
 key="MapZoomFactor";
 if(ui->mSaveScale->isChecked())
  ini.setValue(key,QString::number(map->params.mapZoomFactor));
 else ini.remove(key);
 key="MapPosition";
 if(ui->mSavePosition->isChecked())
      ini.setValue(key,QPoint(map->horizontalScrollBar()->value(),map->verticalScrollBar()->value()));
 else ini.remove(key);
 // -> create default skin? // if(MobileVersion) ini.setValue("Styles",MAP_STYLE);
 if(AppConfig.value(igOther).value(ikMCleanCache).toInt()) removeDir(appCachePath);
 else
 if(map->config.enableCaching && !QFile::exists(map->params.mapCachePath+".cache"))
 {
  QList<QByteArray> supportFormats;
  supportFormats << QImageWriter::supportedImageFormats();
  int supportCount=supportFormats.count();
  if(supportCount)
  {
   QByteArray format;
   format="png";
   if(!supportFormats.contains(format))
   {
    format="jpg";
    if(!supportFormats.contains(format)) format=supportFormats.at(0);
   }
   //QImage img = map->CachedMap->toImage();
   QImageWriter imgWriter(map->params.mapCachePath+".cache",format.toUpper());
   if(imgWriter.canWrite())
        imgWriter.write(map->CachedMap->toImage());
   else map->CachedMap->save(map->params.mapCachePath+".cache",format);
  }
 }
 map->clearMap();

 //setVisible(0);
 e->accept();
 qApp->quit();
}

void applySettings(const IniSettingMap &settings)
{
 TMapView *map=mainWnd->map;
// if(!settings.count()) return;

 QFont nodeFont;
 QMapIterator<IniGroup,IniKeyMap> i(settings);
 while (i.hasNext())
 {
  i.next();
  QMapIterator<IniKey,QString> j(i.value());
  while (j.hasNext())
  {
   j.next();
   QString value=j.value();
   //qDebug() << value;
   if(value.isEmpty()) continue;

   switch(j.key())
   {
//   case ikSkin: mainWnd->loadSkin(value); break;
   case ikStationCustomFont: map->config.enableCustomFont=value.toInt(); break;
   case ikStationFont: nodeFont.fromString(value); break;
   case ikStationBack: map->config.enableStationBackColor=value.toInt(); break;
   case ikStationShadow: map->config.enableStationShadowColor=value.toInt(); break;
   case ikSBackColor: map->params.nodeColorBack.setNamedColor(value); break;
   case ikSShadowColor: map->params.nodeColorShadow.setNamedColor(value); break;
   case ikSUpperCase: nodeFont.setCapitalization(( value.toInt() ? QFont::AllUppercase:QFont::MixedCase)); break;
   case ikSWordWrap: map->params.nodeFontStyle=(value.toInt() ? Qt::TextWordWrap:Qt::TextSingleLine)+Qt::TextDontClip; break;
   case ikMBackColor:
   {
    map->params.mapColorBack.setNamedColor(value);
    QPalette palette;
    palette.setColor(map->backgroundRole(), map->params.mapColorBack);
    map->setPalette(palette);
   } break;
   case ikSTransparency: map->config.valueTransparency=value.toInt(); break;
   case ikMShowScrollbar: map->config.enableShowScrollBar=value.toInt(); break;
   case ikMShowTime: map->config.enableWriteTime=value.toInt(); break;
   case ikMShowBackground: map->config.enableDrawBackground=value.toInt(); break;
   case ikMAntialiasing: map->config.enableAntialiasing=value.toInt(); break;
   case ikMFastScrolling: map->config.enableLowqDrag=value.toInt(); break;
   case ikMSimpleTransfers: map->config.enableSimpleTransfer=value.toInt(); break;
   case ikMLastFile: map->config.mapLastFile=value; break;
   case ikMCaching: map->config.enableCaching=value.toInt(); break;
   case ikMSwipeRoutes: map->config.enableSwipeRoutes=value.toInt(); break;
   case ikWUiScale: mainWnd->Config.appUiScale=qreal(value.toFloat()); break; // setScaleForUi((qreal)value.toFloat()); break;
   case ikWUiButtonScale: mainWnd->Config.appUiButtonScale=qreal(value.toFloat()); break;
   default: break;
   }
  }
 }

 mainWnd->loadSkin(settings.value(igMain).value(ikSkin)); // + ikWUiScale + ikWUiButtonScale
 map->applyConfig(); // + load last map

 // change params
 if(map->config.enableCustomFont)
      map->params.nodeFont=nodeFont;
 else map->params.nodeFont=map->params.nodeFontDefault;
 map->params.nodeTimeFont=map->params.nodeFont;
 map->params.nodeTimeFont.setPixelSize(map->params.nodeWidth*1.25);

 if(mainWnd->Config.mapKineticScrollCheck && map->config.enableCaching)
 {
  charm.activateOn(map);
  mainWnd->connect(&charm,SIGNAL(started()),map,SLOT(mapStartDragging()));
  mainWnd->connect(&charm,SIGNAL(finished()),map,SLOT(mapStopDragging()));
 }
 else
 {
  charm.deactivateFrom(map);
  mainWnd->disconnect(&charm,SIGNAL(started()),map,SLOT(mapStartDragging()));
  mainWnd->disconnect(&charm,SIGNAL(finished()),map,SLOT(mapStopDragging()));
 }

}

void saveSettings(const IniSettingMap &settings)
{
 QSettings cfg(appConfigFile,QSettings::IniFormat);
 cfg.setIniCodec(mainWnd->codePage);
 if(AppConfig.count())
 {
  QMapIterator<IniGroup,IniKeyMap> i(AppConfig);
  while (i.hasNext())
  {
   i.next();
   AppConfig.remove(i.key());
  }
  AppConfig.clear();
 }

 if(settings.count())
 {
  QMapIterator<IniGroup,IniKeyMap> i(settings);
  while (i.hasNext())
  {
   i.next();
   AppConfig.insert(i.key(),i.value());
  }
 }


 QMapIterator<IniGroup,IniKeyMap> i(AppConfig);
 while (i.hasNext())
 {
  i.next();
  cfg.beginGroup(IniGroupStr[i.key()]);
  //qDebug() << IniGroupStr[i.key()];
  QMapIterator<IniKey,QString> j(i.value());
  while (j.hasNext())
  {
   j.next();
   cfg.setValue(IniKeyStr[j.key()],j.value());
   //qDebug() << IniKeyStr[j.key()]+"="+j.value();
  }
  cfg.endGroup();
 }
}

void loadSettings()
{
 TMapView *map=mainWnd->map;
 map->defaultConfig();

 QSettings cfg(appConfigFile,QSettings::IniFormat);
 cfg.setIniCodec(mainWnd->codePage);

 foreach(QString group, cfg.childGroups())
 {
  IniKeyMap keys;
  cfg.beginGroup(group);
  //qDebug() << ">>" << group << IniGroupStr.indexOf(group);
  foreach(QString key, cfg.allKeys())
  {
//   qDebug() << key << (IniKey)IniKeyStr.indexOf(key) << cfg.value(key).toString();
   keys.insert((IniKey)IniKeyStr.indexOf(key),cfg.value(key).toString());
  }
  AppConfig.insert((IniGroup)IniGroupStr.indexOf(group),keys);
  cfg.endGroup();
 }

//////////
// if(AppConfig.value(igMapView).value(ikStationFont).isEmpty())
// {
//  IniKeyMap key=AppConfig.value(igMapView);
//  key.insert(ikStationFont,map->params.nodeFont.toString());
//  AppConfig.erase(AppConfig.find(igMapView));
//  AppConfig.insert(igMapView,key);
// }

 applySettings(AppConfig);
// map->applyConfig();
}

bool removeDir(const QString &DirName)
{
 bool Result=1;
 QDir Dir(DirName);

 if (Dir.exists(DirName))
 {
  foreach(QFileInfo info, Dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
  {
   if (info.isDir())
        Result=removeDir(info.absoluteFilePath());
   else Result=QFile::remove(info.absoluteFilePath());

   if (!Result) return Result;
  }
  Result=Dir.rmdir(DirName);
 }

 return Result;
}

qint64 sizeOfDir(const QString &DirName)
{
 qint64 Result=0;
 QDir Dir(DirName);

 if (Dir.exists())
  foreach(QFileInfo info, Dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
  {
   if (info.isDir())
        Result+=sizeOfDir(info.absoluteFilePath());
   else Result+=info.size();
  }

 return Result;
}

QString MainWindow::getMessageAddMap()
{
 QString Text=appPath+"map/";
 Text=tr("1. Wait until loaded site...\n2. Select map\n3. Save file map here:")+"\n     "+Text;
 return Text;
}

QString MainWindow::getMessageInstalledMaps()
{

 QString MapPath=appPath+"map/";
 QDir Dir=QDir(MapPath,"*.pmz",QDir::Name,QDir::Files);
 return tr("This is the first launch of the program.\n"
           "Want to download more cities?")+"\n\n"
           +tr("Installed maps:")+"\n"
           +Dir.entryList().join(", ").remove(".pmz",Qt::CaseInsensitive)+".";
}


void MainWindow::on_mAddMap_triggered()
{
 QString MapPath=appPath+"map/";
#if !defined(MEEGO_EDITION_HARMATTAN)
 if(QMessageBox::information(this,tr("Add maps"),getMessageAddMap()))
#endif
 {
     if(Config.locale!="ru")
         QDesktopServices::openUrl(QUrl(URL_TRANSLATE.arg(Config.locale).arg(URL_MAPS)));
     else QDesktopServices::openUrl(QUrl(URL_MAPS));
     MapPath=QDir::toNativeSeparators(MapPath);
 }
}

void MainWindow::toFullScreen()
{ ui->mFullScreen->setChecked(1); }

void MainWindow::showPageLoading()
{ ui->stackMain->setCurrentIndex(0); }

void MainWindow::showPageMap()
{ ui->stackMain->setCurrentIndex(1); }

void MainWindow::on_mFullScreen_toggled(bool checked)
{
 //bool b=windowState()!=Qt::WindowFullScreen;

 if(MobileVersion)
 {
  ui->mExit->setVisible(checked); // menu (MobileUI - button in mobileui)
#ifndef Q_WS_MAEMO_5
  menuBar()->setVisible(!checked);
#endif
 }
 else (checked ? ui->toolBar->addAction(ui->mExit):ui->toolBar->removeAction(ui->mExit));

#ifndef QML_EDITION
 if(checked)
       showFullScreen();
  else showMaximized();
#endif

}

void MainWindow::setScaleForUi(qreal UiScale,qreal UiButtonScale)
{
 if(UiScale<=qreal(0.0)) UiScale=Config.appUiScale;
 if(UiButtonScale<=qreal(0.0)) UiButtonScale=Config.appUiButtonScale;
 //qDebug() << UiScale;
 ICONSIZE=ICONSIZE_DEFAULT*UiButtonScale;
 Config.appIconSize=ICONSIZE*UiScale;

 if(MobileVersion)
 {
#if defined(Q_WS_MAEMO_5)
  Config.appFont.setPixelSize(ICONSIZE_DEFAULT/qreal(2.7));
#else
  Config.appFont.setPixelSize(ICONSIZE_DEFAULT/qreal(2.5));
#endif
 }

 QFont font=Config.appFont;
 scaleFont(font,UiScale);

 qApp->setFont(font);
 qApp->setStyleSheet(STYLE_MAIN.arg(Config.appIconSize)          // 1
                                     .arg(int(Config.appIconSize/2))   // 2
                                     .arg(int(Config.appIconSize/4))   // 3
                                     .arg(int(Config.appIconSize/6))   // 4
                                     .arg(int(Config.appIconSize/8))   // 5
                                     .arg(int(Config.appIconSize/10))  // 6
                                     .arg(int(Config.appIconSize/12)));// 7

 if(MobileVersion && mUi)
      mUi->setToolbarIconSize(Config.appIconSize);
 else ui->toolBar->setIconSize(QSize(Config.appIconSize,Config.appIconSize));
 Config.appUiScale=UiScale;
 Config.appUiButtonScale=UiButtonScale;
}

void MainWindow::scaleFont(QFont &Font, const qreal &Factor)
{
 if(Font.pointSizeF()==-1)
      Font.setPixelSize(Font.pixelSize()*Factor);
 else Font.setPointSizeF(Font.pointSizeF()*Factor);
}

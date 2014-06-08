#include <QDebug>
#include <QtGui>
#include "mobileui.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "flickcharm.h"

extern MainWindow *mainWnd;
extern QString STYLE_MAP,SKIN_DEFAULT;
//extern TMapView *map;

TMobileUi::TMobileUi(QWidget *parent,TMapView *map,int type) : MapView(map),TextDesc(NULL)
{Q_UNUSED(parent);
 Ui::MainWindow *ui=mainWnd->ui;
 int Spacing=qRound(mainWnd->Config.appIconSize/10.0);
 DefaultIconSize=QSize(mainWnd->Config.appIconSize,mainWnd->Config.appIconSize);

 lTop =new QHBoxLayout;
 lBottom =new QHBoxLayout;
 lLeft =new QVBoxLayout;
 lRight =new QVBoxLayout;
 lToolbar =new QGridLayout;

 if(type) // details
 {
  bZoomOut = new QToolButton(this);
  bZoomIn = new QToolButton(this);
  bFit = new QToolButton(this);
  bSave = new QToolButton(this);
  bZoomOut->setIcon(ui->mZoomOut->icon());
  bZoomIn->setIcon(ui->mZoomIn->icon());
  bFit->setIcon(ui->mFit->icon());
  bSave->setIcon(ui->mSave->icon());
  connect(bSave,SIGNAL(clicked()),map,SLOT(saveToFile()));
  connect(bZoomOut,SIGNAL(clicked()),map,SLOT(zoomOut()));
  connect(bZoomIn,SIGNAL(clicked()),map,SLOT(zoomIn()));
  connect(bFit,SIGNAL(clicked()),map,SLOT(fitMap()));
 }
 else
 {
  bZoomOut = createButtonFromAction(ui->mZoomOut);
  bZoomIn = createButtonFromAction(ui->mZoomIn);
  bFit = createButtonFromAction(ui->mFit);
  bSave = createButtonFromAction(ui->mSave);
  bNext =  createButtonFromAction(ui->mNextTrack);
  bPrevious = createButtonFromAction(ui->mPreviousTrack);
 }

 bZoomOut->setAutoRepeat(1);
 bZoomIn->setAutoRepeat(1);
 bZoomOut->setShortcut(QKeySequence::ZoomOut);
 bZoomIn->setShortcut(QKeySequence::ZoomIn);
 bSave->setVisible(ui->mSave->isVisible());

 if(!type) // MobileVersion UI
 {
  lTop->addWidget(createButtonFromAction(ui->mFind));
  lTop->addSpacing(Spacing);
  ui->eSearch->setVisible(0);
  //ui->eSearch->setMaximumWidth(maximumWidth());
  lTop->addWidget(ui->eSearch);
  lTop->addStretch();
  bExit=createButtonFromAction(ui->mExit);
  lTop->addWidget(bExit);
  bExit->setVisible(mainWnd->Config.appFullscreen);

  lLeft->addSpacing(Spacing);
  lLeft->addWidget(bZoomOut);
  lLeft->addSpacing(Spacing);
  lLeft->addWidget(bZoomIn);
  lLeft->addStretch();
  bClear=createButtonFromAction(ui->mClear);
  lRight->addWidget(bClear);
  bClear->setVisible(0);

  bInfo=createButtonFromAction(new QAction(ui->mAbout->icon(),tr("Information"),this));
  bInfo->setVisible(0);

  textInfo = new QLabel("");
  textInfo->setMinimumWidth(0);
  textInfo->setMinimumHeight(0);
  //textDescription->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
  textInfo->setAlignment(Qt::AlignCenter);
  textInfo->setVisible(0);
  textInfo->installEventFilter(this);

  QString transparentButton="QToolButton{margin-right:-0.5em;margin-left:-0.5em;padding:0em;border-image:url();background:transparent;}";
  QHBoxLayout *InfoLayout = new QHBoxLayout;
//  ui->bClear->setCursor(Qt::ArrowCursor);
  bNext->setStyleSheet(transparentButton);
  bPrevious->setStyleSheet(transparentButton);
  InfoLayout->setSpacing(0);
  InfoLayout->setMargin(0);
  InfoLayout->addWidget(bPrevious);
  InfoLayout->addStretch();
//  InfoLayout->addWidget(textInfo);
  InfoLayout->addWidget(bNext);
  textInfo->setLayout(InfoLayout);

  lBottom->addWidget(bFit);
  lBottom->addSpacing(Spacing);
  lBottom->addStretch();
  lBottom->addWidget(bInfo);
//  lBottom->addItem(InfoLayout);
  lBottom->addWidget(textInfo);
  lBottom->addStretch();
  lBottom->addSpacing(Spacing);
  bFullscreen=createButtonFromAction(ui->mFullScreen);
  lBottom->addWidget(bFullscreen);


  //### Description ###
  Description = new QWidget(this);
//  QPalette pal = palette();
//  pal.setBrush(QPalette::Window, palette().color(QPalette::Window)); // fix white background
//  Description->setPalette(pal);
//  Description->setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
  bNextDescription =  createButtonFromAction(ui->mNextTrack);
  bPreviousDescription = createButtonFromAction(ui->mPreviousTrack);

  LabelInfo = new QLabel(Description);
  LabelInfo->setAlignment(Qt::AlignHCenter);

  transparentButton="QToolButton{border-image:url();background:transparent;}";
  QHBoxLayout *InfoDescriptionLayout = new QHBoxLayout;
//  ui->bClear->setCursor(Qt::ArrowCursor);
  bNextDescription->setStyleSheet(transparentButton);
  bPreviousDescription->setStyleSheet(transparentButton);
  InfoDescriptionLayout->setSpacing(0);
  InfoDescriptionLayout->setMargin(0);
  InfoDescriptionLayout->addWidget(bPreviousDescription);
  InfoDescriptionLayout->addStretch();
  InfoDescriptionLayout->addWidget(bNextDescription);
  LabelInfo->setLayout(InfoDescriptionLayout);

//  LabelInfo->setStyleSheet("font-weight:bold;");
  TextDesc= new QTextBrowser(Description);
  QPalette pal=TextDesc->palette();
  pal.setBrush(QPalette::Base, mainWnd->palette().color(QPalette::Window));
  pal.setColor(QPalette::Text,mainWnd->palette().color(QPalette::Text));
  TextDesc->setPalette(pal);
  TextDesc->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
  TextDesc->setReadOnly(1);
  TextDesc->setFrameShape(QFrame::NoFrame);
  //bbDialog= new QDialogButtonBox(QDialogButtonBox::Close,Qt::Horizontal,Description);
  bOK= new QPushButton(tr("OK"));
  QVBoxLayout *lDescription =new QVBoxLayout(Description);
  lDescription->addWidget(LabelInfo);
  lDescription->addWidget(TextDesc);
  lDescription->addWidget(bOK);
  Description->setLayout(lDescription);
  ui->stackMain->addWidget(Description);


#if defined(Q_WS_MAEMO_5)
//  Description->setAttribute(Qt::WA_Maemo5StackedWindow);
//  Description->setWindowFlags(Qt::Window);
//  Description->setAttribute(Qt::WA_Maemo5AutoOrientation);

  //  bZoomIn->setShortcut(QKeySequence(Qt::Key_F7));
  //  bZoomOut->setShortcut(QKeySequence(Qt::Key_F8));
  if(mainWnd->Config.appSkinName==SKIN_DEFAULT)
  {
   QIcon icoFullScreen=QIcon::fromTheme(QLatin1String("general_fullsize"));
   if(!icoFullScreen.isNull()) ui->mFullScreen->setIcon(icoFullScreen);
   QIcon icoClose=QIcon::fromTheme(QLatin1String("general_close"));
   if(!icoClose.isNull()) ui->mExit->setIcon(icoClose);
  }
  ui->textDescription->setTextInteractionFlags(Qt::TextSelectableByKeyboard);
#else
 FlickCharm *fcharm = new FlickCharm(TextDesc);
 fcharm->activateOn(TextDesc);
#endif

  // Description
  connect(map,SIGNAL(trackText(QString)),LabelInfo,SLOT(setText(QString)));
  connect(map,SIGNAL(trackDescription(QString)),TextDesc,SLOT(setText(QString)));
  connect(bOK,SIGNAL(clicked()),mainWnd,SLOT(showPageMap()));

//  connect(bOK,SIGNAL(clicked()),mainWnd,SLOT(toFullScreen()));
//  connect(bOK,SIGNAL(clicked()),Description,SLOT(close()));
//  connect(bbDialog,SIGNAL(rejected()),mainWnd,SLOT(toFullScreen()));
//  connect(bbDialog,SIGNAL(rejected()),Description,SLOT(close()));

  // UI
  connect(map,SIGNAL(nodeHasInfo(bool)),bInfo,SLOT(setVisible(bool)));
  connect(bInfo,SIGNAL(clicked()),mainWnd,SLOT(nodeInformation()));
  connect(map,SIGNAL(trackFinded(bool)),textInfo,SLOT(setVisible(bool)));
  connect(map,SIGNAL(trackNames(QStringList)),this,SLOT(setTrackListVisible(QStringList)));

  connect(map,SIGNAL(trackInfo(QString)),textInfo,SLOT(setText(QString)));
  connect(bFullscreen,SIGNAL(toggled(bool)),bExit,SLOT(setVisible(bool)));
  connect(ui->mFind,SIGNAL(toggled(bool)),map->scene,SLOT(update()));
  connect(map,SIGNAL(trackFinded(bool)),this,SLOT(hideSearch(bool)));
  connect(map,SIGNAL(nodeClicked(bool)),bClear,SLOT(setVisible(bool)));

 }
 if(type) // Details window
 {
  lTop->addWidget(bZoomOut);
  lTop->addStretch();
  lTop->addWidget(bSave);

  //lLeft->addSpacing(Spacing);
  lLeft->addWidget(bZoomIn);
  lLeft->addStretch();

  lBottom->addWidget(bFit);
  lBottom->addStretch();
 }

 lToolbar->setMargin(2);
 lToolbar->addItem(lTop,0,0,1,3);
 lToolbar->addItem(lLeft,1,0);
 lToolbar->addItem(lRight,1,2);
 lToolbar->addItem(lBottom,2,0,1,3);
 map->setLayout(lToolbar);
 setToolbarIconSize(mainWnd->Config.appIconSize);

//bFit->setObjectName(ui->mFit->objectName());
//bExit->setObjectName(ui->mExit->objectName());
//bFullscreen->setObjectName(ui->mFullScreen->objectName());
//bClear->setObjectName(ui->mClear->objectName());
//bInfo->setObjectName(ui->mAbout->objectName());
//bZoomOut->setObjectName(ui->mZoomOut->objectName());
//bZoomIn->setObjectName(ui->mZoomIn->objectName());

}
TMobileUi::~TMobileUi()
{

}

QToolButton* TMobileUi::createButtonFromAction(QAction *Action,bool AutoRepeat)
{
 QToolButton *button=new QToolButton(this);
 button->setDefaultAction(Action);
 button->setIconSize(DefaultIconSize);
 button->setAutoRepeat(AutoRepeat);
 return button;
}

void TMobileUi::setToolbarIconSize(int Size)
{
 if(!bZoomOut) return;
 QSize IconSize=QSize(Size,Size);
 MapView->setStyleSheet(STYLE_MAP.arg(Size)          // 1
                                     .arg(int(Size/2))   // 2
                                     .arg(int(Size/4))   // 3
                                     .arg(int(Size/6))   // 4
                                     .arg(int(Size/8))   // 5
                                     .arg(int(Size/10))  // 6
                                     .arg(int(Size/12)));// 7
 QList<QToolButton *> buttons =  MapView->findChildren<QToolButton *>();
 foreach(QToolButton *b,buttons)
 {
  b->setIconSize(IconSize);
 }

 QFont TempFont=qApp->font();
 mainWnd->scaleFont(TempFont,0.8);

 QList<QLabel *> labels =  MapView->findChildren<QLabel *>();
 foreach(QLabel *l,labels)
 {
  l->setFont(TempFont);
 }

 // LabelInfo->setFont(TempFont);
 if(TextDesc) TextDesc->setFont(TempFont);
}

void TMobileUi::hideSearch(bool state)
{
 if(state) mainWnd->ui->mFind->setChecked(0);
}

bool TMobileUi::eventFilter(QObject *obj, QEvent *event)
{
 if(obj==textInfo && event->type() == QEvent::MouseButtonPress)
 {
//  mainWnd->ui->mFullScreen->setChecked(0);
  LabelInfo->setText("<b>"+LabelInfo->text()+"</b>");
  LabelInfo->update();
  textInfo->update();
//  Description->showFullScreen();
  mainWnd->ui->stackMain->setCurrentIndex(2);
  return true;
 }
 else return QObject::eventFilter(obj, event);

}

void TMobileUi::setTrackListVisible(QStringList trackList)
{
 bool visible=trackList.count()>1;
 bNext->setVisible(visible);
 bPrevious->setVisible(visible);
}

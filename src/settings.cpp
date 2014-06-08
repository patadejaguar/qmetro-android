#include <QDebug>
#include <QFile>
#include <QDir>
#include <QKeyEvent>
#include <QColorDialog>
#include <QFontDialog>
#include <QSettings>
#include <QShowEvent>
#include <QPushButton>
#include <QBitmap>
#include <QStyleFactory>

#include "settings.h"
#include "mainwindow.h"
#include "ui_settings.h"
#include "flickcharm.h"

extern MainWindow *mainWnd;
extern IniSettingMap AppConfig;
extern QString appPath,appConfigFile,appCachePath,SKIN_DEFAULT;

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings),SBackColor(Qt::white),SShadowColor(Qt::white),MBackColor(Qt::white),flagReloadMap(0)
{
 bool MobileVersion=mainWnd->MobileVersion;
 setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
 ui->setupUi(this);
 if(!MobileVersion)
 {
  QFontMetrics fmWidth(font());
  QList<QCheckBox *> checks =  findChildren<QCheckBox *>();
  int WindowWidth=0;
  foreach(QCheckBox *chk,checks)
  {
   int width=fmWidth.width(chk->text())+ui->bMEmptyExit->height();
   if(width>WindowWidth) WindowWidth=width;
  }
  checks.clear();

  WindowWidth+=5;
  setMinimumWidth(WindowWidth+50);
  setMaximumWidth(WindowWidth+100);
  resize(WindowWidth,WindowWidth*1.75);


  //  QList<QGroupBox *> groups =  findChildren<QGroupBox *>();
  //  foreach(QGroupBox *gr,groups)
  //  {
  //   gr->setAttribute(Qt::WA_TransparentForMouseEvents);
  ////   QList<QWidget *> widgets =  gr->findChildren<QWidget *>();
  ////   foreach(QWidget *w,widgets) w->setAttribute(Qt::WA_TransparentForMouseEvents,0);
  //  }
 } else // mobile
 {
  QLabel *label;
  QFrame *frameUi=(QFrame*)((QWidget*)ui->checkSCustomFont)->parent();
  QFrame *framePerfomance=(QFrame*)((QWidget*)ui->checkMCaching)->parent();
  QFrame *frameOther=(QFrame*)((QWidget*)ui->gbCache)->parent();

  ((QWidget*)ui->toolBox->children().at(1))->hide();
  ((QWidget*)ui->toolBox->children().at(3))->hide();
  ((QWidget*)ui->toolBox->children().at(5))->hide();

  QVBoxLayout *VLayout=(QVBoxLayout*)((QWidget*)frameUi->parent())->layout();
  QString tabStyleSheet=qApp->styleSheet().section("::tab{",1).section("}",0,0)+"margin:1ex;";
  label=new QLabel(ui->toolBox->itemText(0),this); // Ui
  label->setStyleSheet(tabStyleSheet);
  VLayout->addWidget(label);
  VLayout->addWidget(frameUi);
  label=new QLabel(ui->toolBox->itemText(1),this);// Perfomance
  label->setStyleSheet(tabStyleSheet);
  VLayout->addWidget(label);
  VLayout->addWidget(framePerfomance);
  label=new QLabel(ui->toolBox->itemText(2),this);// Other
  label->setStyleSheet(tabStyleSheet);
  VLayout->addWidget(label);
  VLayout->addWidget(frameOther);
  VLayout->setContentsMargins(2,2,mainWnd->Config.appIconSize/2,2);  // scroll area
 }

#ifdef QMETRO_HAVE_UPDATE
 QGroupBox *gbUpdate = new QGroupBox(this);
 QVBoxLayout *VLayout = new QVBoxLayout;
 QHBoxLayout *HLayout1 = new QHBoxLayout;
 QHBoxLayout *HLayout2 = new QHBoxLayout;
 checkUpdate = new QCheckBox(gbUpdate);
 spinDays = new QSpinBox(gbUpdate);
 QPushButton *bCheckNow = new QPushButton(gbUpdate);
 QLabel *labelLastCheckStr = new QLabel(gbUpdate);
 labelLastCheckDate = new QLabel(gbUpdate);
 ui->lgbOther->addWidget(gbUpdate);
 gbUpdate->setTitle(tr("Update when starts"));
 checkUpdate->setText(tr("Interval for check:"));
 spinDays->setSuffix(QString(" %1").arg(tr("Days")));
 spinDays->setEnabled(0);
 spinDays->setMaximum(365);
 spinDays->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
 bCheckNow->setText(tr("Check Now"));
 bCheckNow->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
 labelLastCheckStr->setText(tr("Last check:"));
 labelLastCheckStr->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
 labelLastCheckDate->setAlignment(Qt::AlignCenter);
 labelLastCheckDate->setWordWrap(1);

 QFont font = checkUpdate->font();
 mainWnd->scaleFont(font,0.85);
 labelLastCheckStr->setFont(font);
 labelLastCheckDate->setFont(font);
 spinDays->setFont(font);

 VLayout->setContentsMargins(5,2,6,2);
 gbUpdate->setLayout(VLayout);
 VLayout->addItem(HLayout1);
 VLayout->addItem(HLayout2);
 HLayout1->addWidget(checkUpdate);
 HLayout1->addWidget(spinDays);
 HLayout2->addWidget(labelLastCheckStr);
 HLayout2->addWidget(labelLastCheckDate);
 HLayout2->addWidget(bCheckNow);

 connect(checkUpdate,SIGNAL(toggled(bool)),spinDays,SLOT(setEnabled(bool)));
 connect(bCheckNow,SIGNAL(clicked()),mainWnd->Updater,SLOT(checkNow()));
 connect(mainWnd->Updater,SIGNAL(onCheckNow(QString)),labelLastCheckDate,SLOT(setText(QString)));
#endif

#ifdef Q_OS_SYMBIAN // QDialogButtonBox not supported - replaced QPushButton
 ui->buttonBox->hide();
 QHBoxLayout *HLayout = new QHBoxLayout;
 QWidget *SoftKeysBar = new QWidget(this);
 HLayout->setContentsMargins(0,10,0,0);
 QPushButton *aSave = new QPushButton(ui->buttonBox->buttons().first()->text());
 connect(aSave, SIGNAL(clicked()),this, SLOT(on_buttonBox_accepted()));
 connect(aSave, SIGNAL(clicked()),this, SLOT(accept()));

 QPushButton *aCancel = new QPushButton(ui->buttonBox->buttons().last()->text());
 connect(aCancel, SIGNAL(clicked()),this, SLOT(on_buttonBox_rejected()));
 connect(aCancel, SIGNAL(clicked()),this, SLOT(reject()));

 HLayout->addWidget(aSave);
 HLayout->addWidget(aCancel);
 SoftKeysBar->setLayout(HLayout);
 layout()->addWidget(SoftKeysBar);
#endif

#ifndef Q_WS_MAEMO_5 // maemo has kinetic
 QObjectList l = ui->toolBox->children();
 for (QObjectList::iterator it = l.begin(); it < l.end(); it++)
  if (QScrollArea *scroll = dynamic_cast<QScrollArea *>(*it))
  {
   FlickCharm *fcharm = new FlickCharm(scroll);
   fcharm->activateOn(scroll);
   if(!MobileVersion) scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  }
 l.clear();
#endif

 QFontMetrics fm(ui->toolBox->font());
 QString ToolStyle="QToolBox{icon-size:%1px;}"+ui->toolBox->styleSheet();
 ui->toolBox->setStyleSheet(ToolStyle.arg(fm.height()+1));
 ui->toolBox->setCurrentIndex(0);
 ui->gbFontStation->setVisible(0);
 ui->gbFontStation->setAttribute(Qt::WA_NoSystemBackground);
 updateLabelCacheSize();
 QString TextColor=tr("Color...");
 ui->bSBackColor->setText(TextColor);
 ui->bSShadowColor->setText(TextColor);
 ui->bMBackColor->setText(TextColor);
 ui->buttonBox->setFocus();

 //if(!QFile::exists(appConfigFile)) saveSettings(changeSettings());

 QDir skinDir=QDir(appPath+"skin/","*",QDir::Name,QDir::Dirs|QDir::NoDotAndDotDot|QDir::Hidden);
 foreach(QString skinName,skinDir.entryList())
 {
  if(skinName.at(0)=='.') skinName.remove(0,1);
  ui->cbSkin->addItem(QIcon(":/img/bullet.png"),skinName);
 }

 //foreach(QString style,QStyleFactory::keys())  ui->cbSkin->addItem(QIcon(""),style);
 ui->cbSkin->addItems(QStyleFactory::keys());
// qDebug() << mainWnd->Config.appSkinName;
 ui->cbSkin->setCurrentIndex(ui->cbSkin->findText(mainWnd->Config.appSkinName,Qt::MatchFixedString));

 //installEventFilter(this);
 DefaultConfig=changeSettings();
 loadSettings();
 loadSettingsToUi(AppConfig);

 if(!MobileVersion)
 {
  connect(ui->bSFont,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->checkSCustomFont,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->checkSBackColor,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->checkSShadow,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->bSBackColor,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->bSShadowColor,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->spinSTransparency,SIGNAL(valueChanged(int)),this,SLOT(changeSettings()));
  connect(ui->checkSUpperCase,SIGNAL(clicked()),this,SLOT(changeSettings()));
  //connect(ui->checkSWordWrap,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->checkMShowScrollbar,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->checkMShowTime,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->checkMShowBackground,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->checkMAntialiasing,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->checkMFastScrolling,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->bMBackColor,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->checkMCaching,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->checkMSwipeRoutes,SIGNAL(clicked()),this,SLOT(changeSettings()));

  //connect(ui->checkMSimpleTransfers,SIGNAL(clicked()),this,SLOT(changeSettings()));

  connect(ui->checkWFullScreen,SIGNAL(clicked()),this,SLOT(changeSettings()));
  connect(ui->spinUiScale,SIGNAL(valueChanged(int)),mainWnd,SLOT(setScaleForUi(int)));
  connect(ui->spinUiButtonScale,SIGNAL(valueChanged(int)),mainWnd,SLOT(setScaleForUiButtons(int)));

  connect(ui->cbSkin,SIGNAL(activated(QString)),this,SLOT(changeSettings()));
 }else // mobile
 {
//  ui->checkMShowScrollbar->setChecked(0);
  ui->checkMShowScrollbar->setVisible(0);
#if defined(MEEGO_EDITION_HARMATTAN)
  ui->checkWFullScreen->setVisible(0);
#endif
  setAttribute(Qt::WA_DeleteOnClose);
 }

// connect(ui->checkSCustomFont,SIGNAL(clicked()),this,SLOT(setFlagReloadMap()));
// connect(ui->checkSWordWrap,SIGNAL(clicked()),this,SLOT(setFlagReloadMap()));
 connect(ui->checkMSimpleTransfers,SIGNAL(clicked()),this,SLOT(setFlagReloadMap()));
 connect(ui->bMBackColor,SIGNAL(clicked()),this,SLOT(setFlagReloadMap()));
}

void Settings::showEvent(QShowEvent *e)
{Q_UNUSED(e);
 emit showed();
}

void Settings::closeEvent(QCloseEvent *e)
{Q_UNUSED(e);
 applySettings(AppConfig);
 //delete this;
 //deleteLater();
}

Settings::~Settings()
{
 mainWnd->SettingsWindow=NULL;
 delete ui;
}

void Settings::keyPressEvent(QKeyEvent *e)
{
 if(e->key()==Qt::Key_Escape) close();
}

void getColorFromDialog(QPushButton *button,QColor &color)
{
 QColorDialog Dialog;
 Dialog.setCurrentColor(color);
 Dialog.exec();
 if(Dialog.result()==1)
 {
  color=Dialog.selectedColor();
  button->setStyleSheet(QString("background: %1").arg(color.name()));
 }
}

void Settings::on_bSBackColor_clicked()
{ getColorFromDialog(ui->bSBackColor,SBackColor); }

void Settings::on_bSShadowColor_clicked()
{ getColorFromDialog(ui->bSShadowColor,SShadowColor); }

void Settings::on_bMBackColor_clicked()
{ getColorFromDialog(ui->bMBackColor,MBackColor); }

QFont Settings::getFontFromLabel(QLabel *Label)
{
 QFont Font;
 if(Label->text().isEmpty())
 {
  setFontToLabel(Label,mainWnd->map->params.nodeFontDefault);
  ui->checkSUpperCase->setChecked(mainWnd->map->params.nodeUpperCase);
  ui->checkSWordWrap->setChecked(mainWnd->map->params.nodeWordWrap);
//  ui->checkSBackColor->setChecked(mainWnd->map->config.enableStationBackColor);
//  ui->checkSShadow->setChecked(mainWnd->map->config.enableStationShadowColor);
  return mainWnd->map->params.nodeFontDefault;
 }
 else Font=Label->font();

 qreal Size=Label->text().section(",",-1).trimmed().toFloat();
 if(Font.pointSizeF()==-1)
      Font.setPixelSize(Size);
 else Font.setPointSizeF(Size);

 return Font;
}

void Settings::setFontToLabel(QLabel *Label,QFont Font)
{
 qreal Size=(Font.pointSizeF()==-1 ? Font.pixelSize():Font.pointSizeF());
 if(Label->font().pointSizeF()==-1)
      Font.setPixelSize(Label->font().pixelSize());
 else Font.setPointSizeF(Label->font().pointSizeF());
 Label->setFont(Font);
 Label->setText(QString("%1, %2").arg(Font.family()).arg(Size));
}

void Settings::on_bSFont_clicked()
{
 bool ok;
 QFont NewFont;
 NewFont=QFontDialog::getFont(&ok,getFontFromLabel(ui->labelSFont),0);
 if(ok)
  setFontToLabel(ui->labelSFont,NewFont);
}

void Settings::on_buttonBox_accepted()
{
  saveSettings(changeSettings());
#ifdef QMETRO_HAVE_UPDATE
 QSettings ini(appConfigFile,QSettings::IniFormat);
 ini.beginGroup("Update");
 ini.setValue("Enable",int(checkUpdate->isChecked()));
 ini.setValue("RefreshDay",spinDays->value());
#endif

 if(flagReloadMap) mainWnd->reloadMap();

 emit closed();
}

void Settings::on_buttonBox_rejected()
{
 applySettings(AppConfig);
 emit closed();
}

void Settings::on_bDefaultConfig_clicked()
{
 loadSettingsToUi(DefaultConfig);
 ui->spinUiScale->setValue(100);
 ui->spinUiButtonScale->setValue(100);
 ui->checkWFullScreen->setChecked(mainWnd->MobileVersion);
 ui->checkMCaching->setChecked(mainWnd->MobileVersion);
 ui->checkMSwipeRoutes->setChecked(mainWnd->MobileVersion);
 ui->cbSkin->setCurrentIndex(ui->cbSkin->findText(SKIN_DEFAULT,Qt::MatchFixedString));

#ifdef QMETRO_HAVE_UPDATE
 checkUpdate->setChecked(1);
 spinDays->setValue(28);
#endif

 // apply
 if(!mainWnd->MobileVersion) changeSettings();
}

IniSettingMap Settings::changeSettings()
{
 IniSettingMap Setting;
 //if(!isVisible()) return Setting;

 // [MapView]
 IniKeyMap MapKeys;
 MapKeys.insert(ikStationCustomFont,QString::number(ui->checkSCustomFont->isChecked()));
 MapKeys.insert(ikStationFont,getFontFromLabel(ui->labelSFont).toString());
 MapKeys.insert(ikStationBack,QString::number(ui->checkSBackColor->isChecked()));
 MapKeys.insert(ikStationShadow,QString::number(ui->checkSShadow->isChecked()));
 MapKeys.insert(ikSBackColor,SBackColor.name());
 MapKeys.insert(ikSShadowColor,SShadowColor.name());
 MapKeys.insert(ikSUpperCase,QString::number(ui->checkSUpperCase->isChecked()));
 MapKeys.insert(ikSWordWrap,QString::number(ui->checkSWordWrap->isChecked()));

 MapKeys.insert(ikSTransparency,QString::number(ui->spinSTransparency->value()));
 MapKeys.insert(ikMShowScrollbar,QString::number(ui->checkMShowScrollbar->isChecked()));
 MapKeys.insert(ikMShowTime,QString::number(ui->checkMShowTime->isChecked()));
 MapKeys.insert(ikMShowBackground,QString::number(ui->checkMShowBackground->isChecked()));
 MapKeys.insert(ikMAntialiasing,QString::number(ui->checkMAntialiasing->isChecked()));
 MapKeys.insert(ikMFastScrolling,QString::number(ui->checkMFastScrolling->isChecked()));
 MapKeys.insert(ikMSimpleTransfers,QString::number(ui->checkMSimpleTransfers->isChecked()));
 MapKeys.insert(ikMBackColor,MBackColor.name());
 MapKeys.insert(ikMCaching,QString::number(ui->checkMCaching->isChecked()));
 Setting.insert(igMapView,MapKeys);

 // [Main]
 IniKeyMap MainKeys;
 MainKeys.insert(ikSkin,ui->cbSkin->currentText());
 MainKeys.insert(ikWFullScreen,QString::number(ui->checkWFullScreen->isChecked()));
 MainKeys.insert(ikWUiScale,QString::number(ui->spinUiScale->value()/qreal(100.0)));
 MainKeys.insert(ikWUiButtonScale,QString::number(ui->spinUiButtonScale->value()/qreal(100.0)));
 Setting.insert(igMain,MainKeys);

 // [Other]
 IniKeyMap OtherKeys;
 OtherKeys.insert(ikMCleanCache,QString::number(ui->checkMCleanCache->isChecked()));
 OtherKeys.insert(ikMSwipeRoutes,QString::number(ui->checkMSwipeRoutes->isChecked()));
 Setting.insert(igOther,OtherKeys);

 applySettings(Setting);


 return Setting;
}

void Settings::loadSettingsToUi(const IniSettingMap &set)
{
 IniKeyMap key;
 QString value;

 setUpdatesEnabled(0);
 blockSignals(1);

 QFont font;
 value=set.value(igMapView).value(ikStationFont);
 if(!value.isEmpty())
      font.fromString(value);
 else font=mainWnd->map->params.nodeFontDefault; //map->font(); //mainWnd->Config.appFont;
// qDebug() << font;
 setFontToLabel(ui->labelSFont,font);

 // [MapView]
 key=set.value(igMapView);
 value=key.value(ikStationCustomFont);
 if(!value.isEmpty()) ui->checkSCustomFont->setChecked(value.toInt());
 value=key.value(ikStationBack);
 if(!value.isEmpty()) ui->checkSBackColor->setChecked(value.toInt());
 value=key.value(ikStationShadow);
 ui->checkSShadow->setChecked(value.toInt());
 value=key.value(ikSBackColor);
 if(!value.isEmpty())
 {
  SBackColor.setNamedColor(value);
  ui->bSBackColor->setStyleSheet(QString("background: %1").arg(value));
 }
 value=key.value(ikSShadowColor);
 if(!value.isEmpty())
 {
  SShadowColor.setNamedColor(value);
  ui->bSShadowColor->setStyleSheet(QString("background: %1").arg(value));
 }
 value=key.value(ikMBackColor);
 if(!value.isEmpty())
 {
  MBackColor.setNamedColor(value);
  ui->bMBackColor->setStyleSheet(QString("background: %1").arg(value));
 }
 value=key.value(ikSUpperCase);
 if(!value.isEmpty()) ui->checkSUpperCase->setChecked(value.toInt());
 value=key.value(ikSWordWrap);
 if(!value.isEmpty()) ui->checkSWordWrap->setChecked(value.toInt());
 value=key.value(ikSTransparency);
 if(!value.isEmpty()) ui->spinSTransparency->setValue(value.toInt());
 value=key.value(ikMShowScrollbar);
 if(!value.isEmpty()) ui->checkMShowScrollbar->setChecked(value.toInt());
 value=key.value(ikMShowTime);
 if(!value.isEmpty()) ui->checkMShowTime->setChecked(value.toInt());
 value=key.value(ikMShowBackground);
 if(!value.isEmpty()) ui->checkMShowBackground->setChecked(value.toInt());
 value=key.value(ikMAntialiasing);
 if(!value.isEmpty()) ui->checkMAntialiasing->setChecked(value.toInt());
 value=key.value(ikMFastScrolling);
 if(!value.isEmpty()) ui->checkMFastScrolling->setChecked(value.toInt());
 value=key.value(ikMSimpleTransfers);
 if(!value.isEmpty()) ui->checkMSimpleTransfers->setChecked(value.toInt());
 value=key.value(ikMCaching);
 if(!value.isEmpty()) ui->checkMCaching->setChecked(value.toInt());
 else ui->checkMCaching->setChecked(mainWnd->map->config.enableCaching);

 // [Main]
 key=set.value(igMain);
 value=key.value(ikSkin);
 if(!value.isEmpty()) ui->cbSkin->setCurrentIndex(ui->cbSkin->findText(value,Qt::MatchFixedString));
 value=key.value(ikWFullScreen);
 if(!value.isEmpty()) ui->checkWFullScreen->setChecked(value.toInt());
 else ui->checkWFullScreen->setChecked(mainWnd->Config.appFullscreen);
 //value=key.value(ikWUiScale);
 //if(!value.isEmpty())
 ui->spinUiScale->setValue(qRound(mainWnd->Config.appUiScale*100));
 ui->spinUiButtonScale->setValue(qRound(mainWnd->Config.appUiButtonScale*100));

 // [Other]
 value=set.value(igOther).value(ikMCleanCache);
 if(!value.isEmpty()) ui->checkMCleanCache->setChecked(value.toInt());
 value=key.value(ikMSwipeRoutes);
 if(!value.isEmpty()) ui->checkMSwipeRoutes->setChecked(value.toInt());
 else ui->checkMSwipeRoutes->setChecked(mainWnd->map->config.enableSwipeRoutes);
 //set.value(igMapView).value().toInt()

#ifdef QMETRO_HAVE_UPDATE
 QSettings ini(appConfigFile,QSettings::IniFormat);
 ini.beginGroup("Update");
 checkUpdate->setChecked(ini.value("Enable",1).toBool());
 spinDays->setValue(ini.value("RefreshDay",28).toInt());
 labelLastCheckDate->setText(ini.value("Date").toDateTime().toString(Qt::SystemLocaleShortDate).remove(QRegExp(":\\d\\d($|\\s)")));
 if(labelLastCheckDate->text().isEmpty()) labelLastCheckDate->setText(tr("never"));
#endif

 flagReloadMap=0;

 setUpdatesEnabled(1);
 blockSignals(0);
}

void Settings::on_bMEmptyExit_clicked()
{
 removeDir(appCachePath);
 updateLabelCacheSize();
}

void Settings::updateLabelCacheSize()
{
 qreal Size=sizeOfDir(appCachePath)/1024;

 QString Ext="Kb";

 if(Size>1024) {Size=Size/1024; Ext="Mb";}
 if(Size>1024) {Size=Size/1024; Ext="Gb";}
 if(Size>1024) {Size=Size/1024; Ext="Tb";}

 ui->labelCacheSize->setText(QString("%1 %2 ").arg((Size-int(Size)>0 ? QString::number(Size,'f',2):QString::number(Size))).arg(Ext));
}



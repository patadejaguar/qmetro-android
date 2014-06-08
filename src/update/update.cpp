#include <QSettings>
#include <QApplication>
#include <QDesktopServices>
#include <QStringList>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QDir>

#include "update.h"
#include "download.h"

#ifdef Q_OS_SYMBIAN
 #define _DEF( x ) QString(x).remove('"') // Symbian 474 "\"TEXT\""
#else
 #define _DEF( x ) QString(x)
#endif

#ifndef QMETRO_UPDATE_URL
 #define QMETRO_UPDATE_URL "http://qmetro.sf.net/update"
#endif

extern QString appConfigFile,appMapPath,appCachePath;

TUpdate::TUpdate(QObject *parent) : QObject(parent), JobCount(0),isChecking(0)
{
 connect(this,SIGNAL(doneInfoList()),this,SLOT(checkMaps()));

 QSettings ini(appConfigFile,QSettings::IniFormat);
 ini.beginGroup("Update");
 if(!ini.value("Enable",1).toBool()) return;

 QDateTime dateUpdate;
 QString key;
 key="Date";
 dateUpdate=QDateTime::fromString(ini.value(key).toString(),Qt::ISODate);

 if(!dateUpdate.isValid() || dateUpdate.daysTo(QDateTime::currentDateTime()) >= ini.value("RefreshDay",28).toInt())
 {
  // check app
  checkNow();
 }
}

void TUpdate::checkNow()
{
 if(isChecking) return;
 isChecking=1;
 QSettings ini(appConfigFile,QSettings::IniFormat);
 ini.beginGroup("Update");
 UpdateURL=ini.value("URL",_DEF(QMETRO_UPDATE_URL)).toString();

 downloadFile(UpdateURL,appCachePath);
 connect(Download,SIGNAL(doneDownload(QString)),this,SLOT(checkUpdate(QString)));
 connect(Download,SIGNAL(error(QString)),this,SLOT(checkError(QString)));
 connect(Download,SIGNAL(done()),this,SLOT(checked()));
 Download->start();
}

void TUpdate::checkUpdate(QString UpdateFile)
{
 if(!QFile::exists(UpdateFile)) return;
 isChecking=1;

 QSettings cfg(appConfigFile,QSettings::IniFormat);
 cfg.setValue("Update/Date",QDateTime::currentDateTime().toString(Qt::ISODate));
 emit onCheckNow(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate).remove(QRegExp(":\\d\\d($|\\s)")));

 QSettings ini(UpdateFile,QSettings::IniFormat);
 if(ini.childGroups().contains(_DEF(QMETRO_PLATFORM),Qt::CaseInsensitive))
      ini.beginGroup(_DEF(QMETRO_PLATFORM));
 else ini.beginGroup("Source");

 QString split="::",
         value=ini.value("URL").toString();
 QString urlUpdate=value.section(split,0,0);
 QString msg=value.section(split,1,1);

 if(versionToReal(ini.value("Version").toString()) > versionToReal(_DEF(VERSION)))
  if(QMessageBox::question(QApplication::activeWindow(),_DEF(APPNAME)+" ("+_DEF(QMETRO_PLATFORM)+")",
         tr("Updated version of qMetro is available for download at:")+
         QString("<br><a href=\"%1\">%1</a><br>").arg(urlUpdate)+msg+
         QString("<table><tr><td>%1:<td>&nbsp;%3<tr><td><b>%2:</b><td><b>&nbsp;%4</b></table><br><br>")
         .arg(tr("Installed")).arg(tr("Available")).arg(_DEF(VERSION)).arg(ini.value("Version").toString())+
         tr("Update?"), QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
  {
   QDesktopServices::openUrl(QUrl(urlUpdate));
  }
 ini.endGroup();

 // check maps
 ini.beginGroup("Maps");
 MapURL=ini.value("FolderURL").toString();
 if(MapURL.isEmpty()) { checked(); return; }
 isChecking=1;

 QStringList MapFiles;
 QDir Dir(appMapPath);
 if (Dir.exists())
  foreach(QFileInfo info, Dir.entryInfoList(QStringList()<<"*.pmz",QDir::NoDotAndDotDot | QDir::Files))
  {
   MapFiles << info.baseName()+".zip";
  }

 getInfoForFiles(MapURL,MapFiles);

 ///!!!
 QFile::remove(UpdateFile);
}

void TUpdate::checkMaps()
{
 QStringList MapNames;
 qint64 DownloadSize=0;

 QDir Dir(appMapPath);
 if (Dir.exists())
  foreach(QFileInfo mapInfo, Dir.entryInfoList(QStringList()<<"*.pmz",QDir::NoDotAndDotDot | QDir::Files))
  {
   foreach (TDownload::THeadInfo headInfo, InfoList)
   {
      if(mapInfo.baseName().toLower()==headInfo.fileName.toLower().remove(".zip")
         && mapInfo.lastModified() < headInfo.modified )
      {
       MapNames << mapInfo.baseName();
       DownloadSize+=headInfo.size;
      }
   }
  }

 MapNames.sort();

 if(DownloadSize)
 {
  if(QMessageBox::question(QApplication::activeWindow(),_DEF(APPNAME)+" ("+_DEF(QMETRO_PLATFORM)+")",
         tr("Updated maps available for download:")+
         QString("<table><tr><td><b>%1:</b><td>%3<tr><td><b>%2:</b><td>%4</table><br><br>")
         .arg(tr("Maps")).arg(tr("Size")).arg(MapNames.join(", ")).arg(convertSizeToString(DownloadSize))+
         tr("Download and install?"), QMessageBox::Yes|QMessageBox::No)==QMessageBox::No)
         { checked(); return;}
 } else  { checked(); return;}

 foreach(QString fileName, MapNames)
 {
  downloadFile(MapURL+fileName+".zip",appMapPath);
  connect(Download,SIGNAL(done()),this,SLOT(doneMapDownload()));
  Download->start();
 }
}

qreal TUpdate::versionToReal(QString Version)
{
 if(Version.split('.').count()==3) Version.remove(Version.lastIndexOf('.'),1);
 Version.remove('-');
 return qreal(Version.toFloat());
}


/// Download task
///---------------

// download in thread
TDownload* TUpdate::downloadFile(QString URL,QString ToDirectory)
{
 if(!JobCount) InfoList.clear();

 Download = new TDownload(URL,ToDirectory);
 Download->Job = new QThread;
 Download->moveToThread(Download->Job);

 connect(Download->Job,SIGNAL(started()),Download,SLOT(start()));
 connect(Download->Job,SIGNAL(started()),this,SLOT(jobInc())); // JobCount++
 connect(Download,SIGNAL(done()),this,SLOT(jobDec()));         // JobCount--
 connect(Download,SIGNAL(done()),Download->Job,SLOT(quit()));
 connect(Download,SIGNAL(done()),Download,SLOT(deleteLater()));
 connect(Download->Job,SIGNAL(finished()),Download->Job,SLOT(deleteLater()));

 connect(Download,SIGNAL(headInfo(TDownload::THeadInfo)),this,SLOT(addInfoToList(TDownload::THeadInfo)));

 return Download;
}

// check files at http folder
void TUpdate::getInfoForFiles(QString HttpFolderURL, QStringList FileNames)
{
 existInfoList=0;
 foreach(QString fileName, FileNames)
 {
//  qDebug() << "checkFile:" << HttpFolderURL+fileName;
  downloadFile(HttpFolderURL+fileName)->start(); // only head return: headInfo(HeadInfo)
 }
}

// collect info from http head
void TUpdate::addInfoToList(TDownload::THeadInfo HeadInfo)
{
 InfoList << HeadInfo;
// qDebug() << "Jobs:" << JobCount << HeadInfo.fileName;
 if(!JobCount && !existInfoList)
 {
  existInfoList=1;
  emit doneInfoList();
 }
}

QString TUpdate::convertSizeToString(qreal Size)
{
 QString Ext="Bytes";

 if(Size>1024) {Size=Size/1024; Ext="Kb";}
 if(Size>1024) {Size=Size/1024; Ext="Mb";}
 if(Size>1024) {Size=Size/1024; Ext="Gb";}

 return QString("%1 %2").arg((Size-int(Size)>0 ? QString::number(Size,'f',2):QString::number(Size))).arg(Ext);
}

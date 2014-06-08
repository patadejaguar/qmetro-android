#ifndef UPDATE_H
#define UPDATE_H

#include <QDebug>
#include <QObject>
#include <QThread>
#include <QDateTime>
#include <QStringList>

#include "download.h"

//class TDownload;
//class THeadInfo;

class TUpdate : public QObject
{
 Q_OBJECT

public:
 explicit TUpdate(QObject *parent=0);
 TDownload* downloadFile(QString URL,QString ToDirectory="");

 void getInfoForFiles(QString HttpFolderURL, QStringList FileNames);

 QString UpdateURL;
 QString MapURL;

 QList<TDownload::THeadInfo> InfoList;

public slots:
 void checkNow();
 void checkError(QString Error) { qDebug() << Error; emit onCheckNow(tr("error")+QString("<br><small>%1</small>").arg(Error)); }
 void checked() {isChecking=0;} // antispam

 void checkUpdate(QString UpdateFile);
 void checkMaps();
 void doneMapDownload() { if(!JobCount) checked(); emit doneUpdateMaps(); }
 void jobInc() { JobCount++; }
 void jobDec() { JobCount--; }
 qreal versionToReal(QString Version);

 void addInfoToList(TDownload::THeadInfo HeadInfo);
 QString convertSizeToString(qreal Size);

signals:
 void doneUpdateMaps();
 void onCheckNow(QString);

 void doneInfoList();


private:
 TDownload *Download;
 int JobCount;
 bool isChecking;

 bool existInfoList;

};

#endif // UPDATE_H

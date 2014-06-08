#include <QtNetwork>

#include "download.h"
//#include "mainwindow.h"

extern QString URL_APP;

TDownload::TDownload(QString link,QString toDir) : Job(0), qnam(0)
{
 qRegisterMetaType<TDownload::THeadInfo>("TDownload::THeadInfo");

 url = link;
 dirDestination=toDir;
 netRequest.setRawHeader("User-Agent", QString("%1 %2 (%3)").arg(qAppName()).arg(QString(VERSION)).arg(QString(QMETRO_PLATFORM)).toLatin1());
}

void TDownload::start()
{
 if(Job && !Job->isRunning()) { Job->start(); return; }

 qnam = new QNetworkAccessManager;
#ifndef QT_NO_OPENSSL
 connect(qnam,SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),this,SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif
 httpRequestAborted=0;
 startRequest(url);
}

void TDownload::startRequest(QUrl url)
{
// qDebug() << "start" << url;
 netRequest.setUrl(QUrl(url));
 headReply=qnam->head(netRequest);
 connect(headReply,SIGNAL(finished()),this,SLOT(headFinished()));
}

void TDownload::headFinished()
{
 // HEAD
 QVariant redirectionTarget = headReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
 if(headReply->error())
 {
//  qDebug() << "Error:" << headReply->errorString();
  emit error(headReply->errorString());
  headReply->deleteLater();
  headReply=0;
  emit done();
  return;
 }
 else
 if(!redirectionTarget.isNull())
 {
  QUrl newUrl = url.resolved(redirectionTarget.toUrl());
  url = newUrl;
  headReply->deleteLater();
  startRequest(url);
  return;
 }



 QFileInfo fileInfo(url.path());
 QString fileName = fileInfo.fileName();

 HeadInfo.fileName = fileName;
 HeadInfo.size = headReply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
 HeadInfo.modified = headReply->header(QNetworkRequest::LastModifiedHeader).toDateTime();


 // only head info: httpModified, httpSize
// qDebug() << "HEAD:" << HeadInfo.fileName << HeadInfo.size << HeadInfo.modified;
 if(dirDestination.isEmpty())
 {
  emit done();
  emit headInfo(HeadInfo);
  return;
 }


 if(fileName.isEmpty() || !HeadInfo.size) {emit done(); return;} //  index.html;
 fileName=dirDestination+fileName;


 // GET
 if(QFile::exists(fileName))
 {
  QFileInfo fi(fileName);

  if(fi.lastModified()<HeadInfo.modified /* || fi.size()!=httpSize*/)
       QFile::remove(fileName);
  else {emit done(); return;}
 }

 file = new QFile(fileName);
 if(!file->open(QIODevice::WriteOnly))
 {
  delete file;
  file=0;
  emit done();
  return;
 }
 netRequest.setUrl(QUrl(url));
 getReply=qnam->get(netRequest);
 connect(getReply,SIGNAL(finished()),this,SLOT(getFinished()));
 connect(getReply,SIGNAL(readyRead()),this,SLOT(getReadyRead()));
 connect(getReply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(updateDataReadProgress(qint64,qint64)));
}
void TDownload::getFinished()
{
 if(httpRequestAborted)
 {
  if(file)
  {
   file->close();
   file->remove();
   delete file;
   file=0;
  }
  getReply->deleteLater();
  emit done();
  return;
 }

 file->flush();
 file->close();

 QVariant redirectionTarget = getReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
 if(getReply->error())
 {
  file->remove();
 }
 else
 if(!redirectionTarget.isNull())
 {
  QUrl newUrl = url.resolved(redirectionTarget.toUrl());
  url = newUrl;
  getReply->deleteLater();
  file->open(QIODevice::WriteOnly);
  file->resize(0);
  startRequest(url);
  return;
 }

 emit doneDownload(file->fileName());
 getReply->deleteLater();
 getReply = 0;
 delete file;
 file = 0;
 emit done();
}

void TDownload::getReadyRead()
{
 if(file) file->write(getReply->readAll());
// qDebug() << getReply->readAll();
}

void TDownload::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
 Q_UNUSED(bytesRead);
 Q_UNUSED(totalBytes);

 if (httpRequestAborted) emit done();
}

void TDownload::cancelDownload()
{
 httpRequestAborted=1;
 getReply->abort();
}

#ifndef QT_NO_OPENSSL
void TDownload::sslErrors(QNetworkReply*,const QList<QSslError> &errors)
{
 emit error(getReply->errorString());
 getReply->ignoreSslErrors();
}
#endif

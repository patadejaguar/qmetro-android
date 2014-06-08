#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QDebug>
#include <QObject>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QThread>

QT_BEGIN_NAMESPACE
class QFile;
class QSslError;
class QNetworkReply;
QT_END_NAMESPACE

class TDownload : public QObject
{
 Q_OBJECT
public:
 TDownload(QString link,QString toDir="");
 ~TDownload(){ if(qnam) delete qnam; }

 void startRequest(QUrl url);
 void cancelDownload();

 QThread *Job;

 struct THeadInfo
 {
  QString fileName;
  qint64 size;
  QDateTime modified;
 }HeadInfo;

public slots:
 void start();
// void startJob() { if(Job && !Job->isRunning()) Job->start(); }
// void test(QString FileName){ qDebug() << 777 << FileName; }

signals:
 void headInfo(TDownload::THeadInfo HeadInfo);
 void doneDownload(QString FileName);
 void started();
 void error(QString Error);
 void done();

private:
 void run();

 QUrl url;
 QNetworkAccessManager *qnam;
 QNetworkRequest netRequest;
 QNetworkReply *getReply,*headReply;
 QFile *file;
 QString dirDestination;
 bool httpRequestAborted;

private slots:
 void headFinished();
 void getFinished();
 void getReadyRead();
 void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
#ifndef QT_NO_OPENSSL
 void sslErrors(QNetworkReply*,const QList<QSslError> &errors);
#endif

};

#endif // DOWNLOAD_H

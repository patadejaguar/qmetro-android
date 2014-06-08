#include <qnearfieldmanager.h>
#include <qnearfieldtarget.h>
#include <QNearFieldTagType2>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>

#include "nfcticket.h"
#include "ui_ticket.h"



TNFCTicket::TNFCTicket(QObject *parent) :
  QObject(parent), ui(new Ui::uiTicketWidget),NFC(new QNearFieldManager(this)),LastTarget(0),enableResearch(0)
{
 TicketWidget = new QWidget();
 ui->setupUi(TicketWidget);


 ui->labelCount->setText("");
 ui->labelInfo->setText("");

 connect(NFC, SIGNAL(targetDetected(QNearFieldTarget*)), this, SLOT(targetDetected(QNearFieldTarget*)));
 connect(NFC, SIGNAL(targetLost(QNearFieldTarget*)),this, SLOT(targetLost(QNearFieldTarget*)));

 NFC->setTargetAccessModes(QNearFieldManager::TagTypeSpecificTargetAccess);

 connect(this,SIGNAL(nfcStatusError(QString)),this,SLOT(msgInformation(QString)));
 nfcStart();
}

void TNFCTicket::nfcStart()
{
 if(checkNfcStatus())
      NFC->startTargetDetection();
 else QTimer::singleShot(2000, this, SLOT(nfcStart()));
}

void TNFCTicket::msgInformation(QString Message)
{
// QMessageBox::information(QApplication::activeWindow(),qAppName(), Message,QMessageBox::Ok);
 qDebug() << Message;
}

void TNFCTicket::targetDetected(QNearFieldTarget *target)
{
 if(target->type() != QNearFieldTarget::NfcTagType2)
 {
  emit nfcStatusError(QString("UID: %1<br>").arg(QString(target->uid().toHex()))+tr("This ticket is not supported"));
  return;
 }
 emit nfcStatusUpdate("");

 if(LastTarget) delete LastTarget;
 LastTarget = target;

 QNearFieldTarget::RequestId rid;
 QByteArray data;

 QNearFieldTagType2* targetSpecific = qobject_cast<QNearFieldTagType2 *>(target);

 // number ticket
 int32_t numberTicket=0;
 rid = targetSpecific->readBlock(4);

 if(targetSpecific->waitForRequestCompleted(rid))
 {
  data.append(targetSpecific->requestResponse(rid).toByteArray().at(2));
  data.append(targetSpecific->requestResponse(rid).toByteArray().at(3));
 }
 rid = targetSpecific->readBlock(5);
 if(targetSpecific->waitForRequestCompleted(rid))
 {
  data.append(targetSpecific->requestResponse(rid).toByteArray().at(0));
  data.append(targetSpecific->requestResponse(rid).toByteArray().at(1));
  data.append(targetSpecific->requestResponse(rid).toByteArray().at(2));
  numberTicket |= (data.at(0)&15)<<28;
  numberTicket |= data.at(1)<<20;
  numberTicket |= data.at(2)<<12;
  numberTicket |= data.at(3)<<4;
  numberTicket |= (data.at(4)>>4)&15;
  data.clear();
 }

 // valid date
 QString validDate;
 rid = targetSpecific->readBlock(6);
 if(targetSpecific->waitForRequestCompleted(rid))
 {
  int16_t days=0;
  data.append(targetSpecific->requestResponse(rid).toByteArray().at(0));
    data.append(targetSpecific->requestResponse(rid).toByteArray().at(1));
  days |= data.at(0)<<8;
  days |= data.at(1);
  data.clear();
  validDate = QDate(1992,1,1).addDays(days-1).toString("(ddd) dd.MM.yyyy").toLower();
 }

 // buy date
 QString buyDate;
 rid = targetSpecific->readBlock(8);
 if(targetSpecific->waitForRequestCompleted(rid))
 {
  unsigned int days=0;
  data.append(targetSpecific->requestResponse(rid).toByteArray().at(0));
  data.append(targetSpecific->requestResponse(rid).toByteArray().at(1));
  days |= data.at(0)<<8;
  days |= data.at(1);
  data.clear();
  buyDate = QDate(1992,1,1).addDays(days-1).toString("(ddd) dd.MM.yyyy").toLower();

  // type days
  data.append(targetSpecific->requestResponse(rid).toByteArray().at(2));
 }

 int typeDays;
 typeDays=data.at(0);
 data.clear();

 // count ticket
 int countTickets;
 rid = targetSpecific->readBlock(9);
 if(targetSpecific->waitForRequestCompleted(rid))
  countTickets=targetSpecific->requestResponse(rid).toByteArray().at(1);

// QTime lastTime;
// rid = targetSpecific->readBlock(11);
// if(targetSpecific->waitForRequestCompleted(rid))
// {
//  QByteArray b = targetSpecific->requestResponse(rid).toByteArray();
//  char b0=b.at(1),b1=b.at(2);
//  lastTime.setHMS((b1>>3)&31, ((b1&7) << 3)|(b0>>5)&7, (b0&31)*2); // DOSTIME to QTime
// }

 ui->labelCount->setText(QString::number(countTickets));
 ui->labelInfo->setText(QString("<font face=monospace>№ %1 UID: %2<br>").arg(numberTicket).arg(QString(targetSpecific->uid().toHex()))+
                        QString("%1 &mdash; %2 (%3)</font>").arg(buyDate).arg(validDate).arg(typeDays));

 TicketWidget->setAttribute(Qt::WA_LockPortraitOrientation, 1);

 if(qApp->activeWindow()->windowState()==Qt::WindowFullScreen)
      TicketWidget->showFullScreen();
 else TicketWidget->showMaximized();


 bool enableFullResearch=0;
 if(enableResearch)
 {
//  qDebug() << "MEMORY:" << targetSpecific->memorySize() << "uid:" << targetSpecific->uid().toHex();

  QString path="e:/data/qTicket/";
  QString tmpFile =path+targetSpecific->uid().toHex();

  QDir dir;  dir.mkpath(path);
  QString timeStamp=QDateTime::currentDateTime().toString(".yyyyMMddhhmmss");
  QFile fileAll(tmpFile+timeStamp);
  QFile fileNFC(tmpFile+".DATA"+timeStamp);
  if(enableFullResearch) fileAll.open(QFile::WriteOnly);
  fileNFC.open(QFile::WriteOnly);
  QDataStream outAll(&fileAll);
  QDataStream outNFC(&fileNFC);
  for(int i=0;i<16;i++) // 16 blocks of data
  {
   rid = targetSpecific->readBlock(i);
   if(!targetSpecific->waitForRequestCompleted(rid)) {
    qDebug() << i << "Error";
   } else {
    QByteArray data=targetSpecific->requestResponse(rid).toByteArray();

    if(enableFullResearch)
     outAll.writeRawData(data,data.length()); // Full data
    else
     outNFC.writeRawData(data,4); // actual 4 bytes of data but real response: 16 bytes + 2 bytes checksum
    //   qDebug() << "Data:" << i << ": " << data.toHex();
   }
  }
//  qDebug() << tmpFile;
  fileAll.close();
  fileNFC.close();
 }


}

void TNFCTicket::targetLost(QNearFieldTarget *target)
{
 TicketWidget->setAttribute(Qt::WA_LockPortraitOrientation, 0);
 TicketWidget->hide();
}




/****************************************************************************
**
** Copyright (C) 2012-2013 Andreas Jakl.
** All rights reserved.
** Contact: Andreas Jakl (andreas.jakl@mopius.com)
**
** This file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
****************************************************************************/

bool TNFCTicket::checkNfcStatus()
{
#ifdef Q_OS_SYMBIAN
    // Construct a new instance.
    nfcSettings = new NfcSettings(this);

    // Retrieve the NFC feature support information.
    NfcSettings::NfcFeature nfcFeature = nfcSettings->nfcFeature();

    if (nfcFeature == NfcSettings::NfcFeatureSupported) {
        // Connect signals for receiving mode change and error notifications.
        connect(nfcSettings, SIGNAL(nfcModeChanged(NfcSettings::NfcMode)), SLOT(handleNfcModeChange(NfcSettings::NfcMode)));
        connect(nfcSettings, SIGNAL(nfcErrorOccurred(NfcSettings::NfcError, int)), SLOT(handleNfcError(NfcSettings::NfcError, int)));

        // Retrieve the initial value of the NFC mode setting.
        NfcSettings::NfcMode nfcMode = nfcSettings->nfcMode();

        if (nfcMode != NfcSettings::NfcModeOn) {
            // NFC is supported but not switched on, prompt the user to enable it.
            emit nfcStatusError(tr("NFC hardware is available but currently switched off"));
            return false;
        } else {
            emit nfcStatusUpdate(tr("NFC is supported and switched on"));
            return true;
        }
    }
    else if (nfcFeature == NfcSettings::NfcFeatureSupportedViaFirmwareUpdate) {
        // Display message to user to update device firmware
        emit nfcStatusError(tr("Update device firmware to enable NFC support"));
        return false;
    } else {
        // Display message informing the user that NFC is not supported by this device.
        emit nfcStatusError(tr("NFC not supported by this device"));
        return false;
    }
#endif
    return false;
}

#ifdef Q_OS_SYMBIAN
/*!
  \brief Slot will be executed by the nfcsettings component whenever
  the NFC availability changes in the phone.

  This can also happen at runtime, for example when the user switches
  on NFC through the settings app. Emits an nfcStatusError or nfcStatusUpdate
  signal with the new information.
  */
void TNFCTicket::handleNfcModeChange(NfcSettings::NfcMode nfcMode)
{
    switch (nfcMode) {
    case NfcSettings::NfcModeNotSupported:
        // NFC is not currently supported. It is not possible to distinguish
        // whether a firmware update could enable NFC features based solely
        // on the value of the nfcMode parameter. The return value of
        // NfcSettings::nfcFeature() indicates whether a firmware update is
        // applicable to this device.
        emit nfcStatusError(tr("NFC is not currently supported"));
        break;
    case NfcSettings::NfcModeUnknown:
        // NFC is supported, but the current mode is unknown at this time.
        emit nfcStatusError(tr("NFC is supported, but the current mode is unknown at this time"));
        break;
    case NfcSettings::NfcModeOn:
        // NFC is supported and switched on.
        emit nfcStatusUpdate(tr("NFC is supported and switched on"));
        break;
    case NfcSettings::NfcModeCardOnly:
        // NFC hardware is available and currently in card emulation mode.
        emit nfcStatusError(tr("NFC hardware is available and currently in card emulation mode"));
        break;
    case NfcSettings::NfcModeOff:
        // NFC hardware is available but currently switched off.
        emit nfcStatusError(tr("NFC hardware is available but currently switched off"));
        break;
    default:
        break;
    }
}

/*!
  \brief Handle any error that might have occured when checking
  the NFC support on the phone.
  */
void TNFCTicket::handleNfcError(NfcSettings::NfcError nfcError, int error)
{
    // The platform specific error code is ignored here.
    Q_UNUSED(error)

    switch (nfcError) {
    case NfcSettings::NfcErrorFeatureSupportQuery:
        // Unable to query NFC feature support.
        emit nfcStatusError(tr("Unable to query NFC feature support"));
        break;
    case NfcSettings::NfcErrorSoftwareVersionQuery:
        // Unable to query device software version.
        emit nfcStatusError(tr("Unable to query device software version"));
        break;
    case NfcSettings::NfcErrorModeChangeNotificationRequest:
        // Unable to request NFC mode change notifications.
        emit nfcStatusError(tr("Unable to request NFC mode change notifications"));
        break;
    case NfcSettings::NfcErrorModeChangeNotification:
        // NFC mode change notification was received, but caused an error.
        emit nfcStatusError(tr("NFC mode change notification was received, but caused an error"));
        break;
    case NfcSettings::NfcErrorModeRetrieval:
        // Unable to retrieve current NFC mode.
        emit nfcStatusError(tr("Unable to retrieve current NFC mode"));
        break;
    default:
        break;
    }
}

#endif

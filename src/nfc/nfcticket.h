#ifndef NFCTICKET_H
#define NFCTICKET_H

#include <QObject>
#include <QWidget>

#include <qmobilityglobal.h>
#include <qnearfieldtarget.h>

#ifdef Q_OS_SYMBIAN
#include "nfcsettings/nfcsettings.h"
#endif

namespace Ui {
    class uiTicketWidget;
}

QTM_BEGIN_NAMESPACE
class QNearFieldManager;
class QNdefMessage;
QTM_END_NAMESPACE

QTM_USE_NAMESPACE

class TNFCTicket : public QObject
{
 Q_OBJECT
public:
 explicit TNFCTicket(QObject *parent = 0);
 ~TNFCTicket(){ delete ui; delete TicketWidget; if(LastTarget) delete LastTarget; }
 bool checkNfcStatus();

 Ui::uiTicketWidget *ui;
 QWidget *TicketWidget;

 bool enableResearch;

signals:
 void onTargetLost(QNearFieldTarget *target);

private slots:
 void targetDetected(QNearFieldTarget *target);
 void targetLost(QNearFieldTarget *target);
 void msgInformation(QString Message);
 void nfcStart();


private:
QNearFieldManager *NFC;
QNearFieldTarget *LastTarget;

#ifdef Q_OS_SYMBIAN

private slots:
    // Check for NFC Support
    void handleNfcError(NfcSettings::NfcError nfcError, int error);
    void handleNfcModeChange(NfcSettings::NfcMode nfcMode);
private:
    NfcSettings* nfcSettings;

signals:
    void nfcStatusError(QString);
    void nfcStatusUpdate(QString);
#endif

};

#endif // NFCTICKET_H

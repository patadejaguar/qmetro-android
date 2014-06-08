#ifndef MOBILEUI_H
#define MOBILEUI_H

#include <qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "mainwindow.h"

class TMapView;

class TMobileUi : public QWidget
{Q_OBJECT

public:
 explicit TMobileUi(QWidget *parent,TMapView *map,int type=0);
 ~TMobileUi();

 QSize DefaultIconSize;

public slots:
 void setToolbarIconSize(int Size);

private slots:
 void hideSearch(bool state);
 void setTrackListVisible(QStringList trackList);

private:
 QToolButton* createButtonFromAction(QAction *Action,bool AutoRepeat=0);

 QGridLayout *lToolbar;
 QHBoxLayout *lTop;
 QHBoxLayout *lBottom;
 QVBoxLayout *lLeft;
 QVBoxLayout *lRight;

 QLabel *textInfo;

 QToolButton *bExit;
 QToolButton *bFullscreen;
 QToolButton *bSave;
 QToolButton *bZoomOut;
 QToolButton *bZoomIn;
 QToolButton *bFit;
 QToolButton *bClear;
 QToolButton *bInfo;
 QToolButton *bNext;
 QToolButton *bPrevious;
 QToolButton *bNextDescription;
 QToolButton *bPreviousDescription;
 TMapView *MapView;

 QWidget *Description;
 QLabel *LabelInfo;
 QTextBrowser *TextDesc;
 QPushButton *bOK;

protected:
bool eventFilter(QObject *obj, QEvent *event);

};

#endif // MOBILEUI_H

#ifndef QMLMAINWINDOW_H
#define QMLMAINWINDOW_H

#include <QDeclarativeItem>
#include <QGraphicsProxyWidget>

#include "mainwindow.h"
#include "ui_mainwindow.h"

class QMLMainWindow : public QDeclarativeItem
{Q_OBJECT
 Q_PROPERTY(QString dirMap READ getMapDefaultDir)
 Q_PROPERTY(QString msgInstalledMaps READ getMessageInstalledMaps)
 Q_PROPERTY(QString msgAddMap READ getMessageAddMap)
 Q_PROPERTY(bool firstStart READ firstStart)

 Q_PROPERTY(bool fullScreen READ fullScreen WRITE setFullScreen NOTIFY fullScreenChanged)
 Q_PROPERTY(int width READ width WRITE setWidth NOTIFY sizeChanged)
 Q_PROPERTY(int height READ height WRITE setHeight NOTIFY sizeChanged)

 Q_PROPERTY(int searchWidth READ searchWidth)
 Q_PROPERTY(int searchHeight READ searchHeight)
 Q_PROPERTY(int searchX READ searchX)
 Q_PROPERTY(int searchY READ searchY)
 Q_PROPERTY(bool searchVisible READ searchVisible)
 Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchChanged)


public:
 QMLMainWindow(QDeclarativeItem *parent =0): QDeclarativeItem(parent), window(new MainWindow())
 {
  //window->setStyleSheet(STYLE);
  //window->ui->mExit->disconnect();
  window->ui->mFullScreen->disconnect();
  connect(window->ui->mFullScreen,SIGNAL(toggled(bool)),this,SLOT(setFullScreen(bool)));
  connect(window->ui->mFind,SIGNAL(toggled(bool)),this,SLOT(toggleSearch(bool)));

  //connect(window->ui->mFullScreen,SIGNAL(toggled(bool)),this,SLOT(showMenu(bool)));
#if defined(MEEGO_EDITION_HARMATTAN)
  window->menuBar()->setVisible(0);
#endif
  proxy = new QGraphicsProxyWidget(this);
  proxy->setWidget(window);
 }

 const QString getMapDefaultDir() const { return window->getMapDefaultDir(); }
 const QString getMessageInstalledMaps() const { return window->getMessageInstalledMaps(); }
 const QString getMessageAddMap() const { return window->getMessageAddMap(); }
 const bool &firstStart() const { return window->Config.appFirstStart; }

 const bool &fullScreen() const { return !window->ui->mFullScreen->isChecked(); }

 const QString searchText() const { return window->ui->eSearch->text(); }
 void setSearchText(const QString text) { window->ui->eSearch->setText(text); emit searchChanged(); }
 const bool searchVisible() const { return window->ui->eSearch->isVisible(); }
// const int searchX() const { return window->mapToGlobal(QPoint(window->ui->eSearch->geometry().left(),0)).x(); }
// const int searchY() const { return window->mapToGlobal(QPoint(0,window->ui->eSearch->geometry().top())).y(); }
 const int searchX() const { return window->ui->eSearch->geometry().left(); }
 const int searchY() const
 { return window->ui->eSearch->geometry().bottom(); }

 const int searchWidth() const { return window->ui->eSearch->geometry().width(); }
 const int searchHeight() const { return window->ui->eSearch->geometry().height(); }

 const int width() const { return window->width(); }
 void setWidth(const int width)
 {
  window->resize(width,window->height());
  emit sizeChanged();
 }
 const int height() const { return window->height(); }
 void setHeight(const int height)
 {
  window->resize(window->width(),height);
  emit sizeChanged();
 }

public slots:

 void close() { window->close(); }
 void loadMap(const QString fileName) { window->openMap(fileName); }
 QString getActionText(const int index)
 {
  switch(index){
  case 1: return window->ui->mOpen->text().remove("&");
  case 2: return window->ui->mSave->text().remove("&");
  case 3: return window->ui->menuSetting->title().remove("&");
  case 4: return window->ui->mAddMap->text().remove("&");
  case 5: return window->ui->mAbout->text().remove("&");
  default: return "";
  }
 }

 void runOpen() { window->on_mOpen_triggered(); }
 void runSaveAs() { window->on_mSave_triggered(); }
 void runSettings() { window->on_mPreference_triggered(); }
 void runAddMap() { window->on_mAddMap_triggered(); }
 void runAbout() { window->on_mAbout_triggered(); }

 void setFullScreen(const bool &IsFull){ window->ui->mFullScreen->setChecked(IsFull);  emit fullScreenChanged(); }
 void toggleSearch(const bool &Toggled){ emit toggledSearch(); }
 void showMenu(const bool &IsHide){ window->menuBar()->setVisible(!IsHide); }

 void scaleMap(qreal scale) { window->scaleMap(scale); }

signals:
 void fullScreenChanged();
 void sizeChanged();
 void searchChanged();
 void toggledSearch();
 //void mapScaled(qreal scaleFactor, QPointF pos);


private:
 MainWindow *window;
 QGraphicsProxyWidget *proxy;
 //QDeclarativeContext *qmlWindow;

protected:
 virtual void geometryChanged(const QRectF &newGeometry, const QRectF & oldGeometry)
 {Q_UNUSED(oldGeometry)
   proxy->setGeometry(newGeometry);
 }

};

#endif // QMLMAINWINDOW_H

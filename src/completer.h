#ifndef COMPLETER_H
#define COMPLETER_H

#include <QtGui>
#include <QObject>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTimer;
class QTreeWidget;
QT_END_NAMESPACE

class TMapView;

class TCompleter : public QObject
{
    Q_OBJECT

public:
    TCompleter(TMapView *mapView,QLineEdit *parent = 0);
    ~TCompleter();
    bool eventFilter(QObject *obj, QEvent *ev);
    void showCompletion(QList< QPair<QString,QColor> > list);

signals:
  void doneCompletionByID(int);
  void doneSuggestByID(QList<int>);

public slots:

    void doneCompletion();
    void preventSuggest();
    void autoSuggest();
    QList< QPair<QString,QColor> > filterList
      (QList< QPair<QString,QColor> > list, QString filter);

private:
    QLineEdit *editor;
    QTreeWidget *popup;
    QTimer *timer;

    QList< QPair<QString,QColor> > list;
    QList<int> idFiltred;

    bool MobileVersion;
    TMapView *map;

    char indName,indColor;
};

#endif


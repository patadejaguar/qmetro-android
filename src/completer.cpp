//#include <QDebug>
#include <qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "completer.h"
#include "mapview/mapview.h"
//#include "flickcharm.h"

TCompleter::TCompleter(TMapView *mapView,QLineEdit *parent)
 : editor(parent)
{
 map=mapView;
 list=map->getNamesAndColors();
 MobileVersion=map->config.MobileVersion;

 indName=0;
 indColor=1;
 if(MobileVersion) qSwap(indName,indColor);

 popup = new QTreeWidget((QWidget*)parent);
 popup->setStyleSheet("QTreeWidget{show-decoration-selected:1;}"+qApp->styleSheet());
 popup->setWindowFlags(Qt::Popup);
 popup->setFocusPolicy(Qt::NoFocus);
 popup->setMouseTracking(true);

 popup->setColumnCount(2);
 popup->setUniformRowHeights(true);
 popup->setRootIsDecorated(false);
 popup->setEditTriggers(QTreeWidget::NoEditTriggers);
 popup->setSelectionBehavior(QTreeWidget::SelectItems);
 popup->setFrameStyle(QFrame::Box | QFrame::Plain);
 popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
 popup->header()->hide();
 popup->installEventFilter(this);
// if(MobileVersion)
// {
//  popup->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//  //popup->setColumnWidth(1,editor->width());
// }
// else
   popup->setFocusProxy(parent);


 //FlickCharm *fcharm = new FlickCharm(popup);
 //fcharm->activateOn(popup);

 connect(popup, SIGNAL(itemClicked(QTreeWidgetItem*,int)),SLOT(doneCompletion()));

 timer = new QTimer(this);
 timer->setSingleShot(true);
 timer->setInterval(500);
 connect(timer, SIGNAL(timeout()), SLOT(autoSuggest()));
 connect(editor, SIGNAL(textEdited(QString)), timer, SLOT(start()));
 connect(this, SIGNAL(doneCompletionByID(int)),map, SLOT(findTrackByID(int)));
 connect(this, SIGNAL(doneSuggestByID(QList<int>)),map, SLOT(findedNodesToFront(QList<int>)));
}

TCompleter::~TCompleter()
{
 delete popup;
 list.clear();
 idFiltred.clear();
}

bool TCompleter::eventFilter(QObject *obj, QEvent *ev)
{
 if(obj != popup) return false;

 if (ev->type() == QEvent::MouseButtonPress)
 {
  popup->hide();
  editor->setFocus();
  return true;
 }

 if (ev->type() == QEvent::KeyPress)
 {
  bool consumed = false;
  int key = static_cast<QKeyEvent*>(ev)->key();
  switch (key) {
  case Qt::Key_Enter:
  case Qt::Key_Return:
   doneCompletion();
   consumed = true;

  case Qt::Key_Escape:
   editor->setFocus();
   popup->hide();
   consumed = true;

  case Qt::Key_Up:
  case Qt::Key_Down:
  case Qt::Key_Home:
  case Qt::Key_End:
  case Qt::Key_PageUp:
  case Qt::Key_PageDown:
   break;

  default:
   editor->setFocus();
   editor->event(ev);
   popup->hide();
   break;
  }

  return consumed;
 }

 return false;
}

void TCompleter::showCompletion(QList< QPair<QString,QColor> > list)
{
 if (list.isEmpty()) return;

// const QPalette &pal = editor->palette();
// QColor color = pal.color(QPalette::Disabled, QPalette::WindowText);
 popup->setUpdatesEnabled(false);
 popup->clear();
 for (int i = 0; i < list.count(); ++i)
 {
  QTreeWidgetItem * item;
  item = new QTreeWidgetItem(popup);
  item->setText(indName, list.at(i).first);
#ifdef Q_WS_MAEMO_5
  item->setText(indColor,"=");
  item->setFont(indColor,QFont("",QFont().pointSize()*2,87));
  item->setTextColor(indColor, list.at(i).second);
#endif
  item->setBackgroundColor(indColor,list.at(i).second);
  //item->setIcon(indColor,); //icon with transparent
  item->setTextAlignment(indColor, Qt::AlignCenter);
 }
 popup->setCurrentItem(popup->topLevelItem(0),(MobileVersion ? 1:0));
 popup->resizeColumnToContents(indName);
 popup->adjustSize();
 popup->setColumnWidth(indColor,editor->height()/(MobileVersion ? 2:1));
 popup->setUpdatesEnabled(true);

 int h = popup->sizeHintForRow(0) * qMin((MobileVersion ? 4:7), list.count()) + 3;
 //when with style// popup->move(editor->mapToGlobal(QPoint(editor->height()/2, editor->height()-1)));
 popup->move(editor->mapToGlobal(QPoint(0, editor->height())));
 if(!MobileVersion) popup->setFocus();
 popup->show();
 popup->resize(popup->columnWidth(indName)+editor->height()+( popup->verticalScrollBar()->isVisible() ? popup->verticalScrollBar()->width():0), h);
 popup->repaint();
 if(MobileVersion)
 {
  editor->hide();
  editor->setFocus();
  editor->show();
 }
}

void TCompleter::doneCompletion()
{
 timer->stop();
 popup->hide();
 editor->setFocus();
 QTreeWidgetItem *item = popup->currentItem();
 if (item)
 { 
  int id=idFiltred.at(popup->currentIndex().row());
  editor->setText(item->text(indName));
  QMetaObject::invokeMethod(editor, "returnPressed");
  emit doneCompletionByID(id);
  idFiltred.clear();
  idFiltred << id;
  map->findedNodesToFront(idFiltred);
 }

}

void TCompleter::autoSuggest()
{
 showCompletion(filterList(list,editor->text()));
 emit doneSuggestByID(idFiltred);
}

void TCompleter::preventSuggest()
{
 timer->stop();
}

QList< QPair<QString,QColor> >
TCompleter::filterList(QList< QPair<QString,QColor> > list, QString filter)
{
 QList< QPair<QString,QColor> > filtred;
 if(filter.trimmed().length()<2) return filtred;
 filter.replace(" ","*");
 QRegExp regExp=QRegExp(filter,Qt::CaseInsensitive,QRegExp::Wildcard);
 idFiltred.clear();
 for(int i=0;i<list.count();i++)
  if(list.at(i).first.contains(regExp))
  {
   filtred << list.at(i);
   idFiltred << i;
  }

 filter=filter.section("*",0,0).toLower();
 for(int i=0;i<filtred.count();i++)
  if(filtred.at(i).first.left(filter.length()).toLower()==filter)
  {
   filtred.move(i,0);
   idFiltred.move(i,0);
  }
 return filtred;
}


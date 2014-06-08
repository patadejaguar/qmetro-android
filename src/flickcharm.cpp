/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Graphics Dojo project on Qt Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 or 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "flickcharm.h"

#include <qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <QAbstractScrollArea>
#include <QApplication>
#include <QBasicTimer>
#include <QEvent>
#include <QHash>
#include <QList>
#include <QMouseEvent>
#include <QScrollBar>
#include <QGraphicsView>
#include <QGraphicsScene>
 #include <QGraphicsSceneMouseEvent>
//#include <QWebFrame>
//#include <QWebView>

#include <QDebug>

struct FlickData {
    typedef enum { Steady, Pressed, ManualScroll, AutoScroll, Stop } State;
    State state;
    QWidget *widget;
    QPoint pressPos;
    QPoint offset;
    QPoint dragPos;
    QPoint speed;
    QList<QEvent*> ignored;
};

class FlickCharmPrivate
{
public:
    QHash<QWidget*, FlickData*> flickData;
    QBasicTimer ticker;
};

FlickCharm::FlickCharm(QObject *parent): QObject(parent)
{
    d = new FlickCharmPrivate;
}

FlickCharm::~FlickCharm()
{
    delete d;
}

void FlickCharm::activateOn(QWidget *widget)
{
    QAbstractScrollArea *scrollArea = dynamic_cast<QAbstractScrollArea*>(widget);
    if (scrollArea) {
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        QWidget *viewport = scrollArea->viewport();
//        viewport->grabGesture(Qt::PinchGesture);
//        viewport->setAttribute(Qt::WA_AcceptTouchEvents);
        viewport->installEventFilter(this);
        scrollArea->installEventFilter(this);

        d->flickData.remove(viewport);
        d->flickData[viewport] = new FlickData;
        d->flickData[viewport]->widget = widget;
        d->flickData[viewport]->state = FlickData::Steady;

        return;
    }

//    QWebView *webView = dynamic_cast<QWebView*>(widget);
//    if (webView) {
//        QWebFrame *frame = webView->page()->mainFrame();
//        frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
//        frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);

//        webView->installEventFilter(this);

//        d->flickData.remove(webView);
//        d->flickData[webView] = new FlickData;
//        d->flickData[webView]->widget = webView;
//        d->flickData[webView]->state = FlickData::Steady;

//        return;
//    }

    qWarning() << "FlickCharm only works on QAbstractScrollArea (and derived classes)";
    qWarning() << "or QWebView (and derived classes)";
}

void FlickCharm::deactivateFrom(QWidget *widget)
{
    QAbstractScrollArea *scrollArea = dynamic_cast<QAbstractScrollArea*>(widget);
    if (scrollArea) {
        QWidget *viewport = scrollArea->viewport();

        viewport->removeEventFilter(this);
        scrollArea->removeEventFilter(this);

        delete d->flickData[viewport];
        d->flickData.remove(viewport);

        return;
    }

//    QWebView *webView = dynamic_cast<QWebView*>(widget);
//    if (webView) {
//        webView->removeEventFilter(this);

//        delete d->flickData[webView];
//        d->flickData.remove(webView);

//        return;
//    }
}

static QPoint scrollOffset(QWidget *widget)
{
    int x = 0, y = 0;

    QAbstractScrollArea *scrollArea = dynamic_cast<QAbstractScrollArea*>(widget);
    if (scrollArea) {
        x = scrollArea->horizontalScrollBar()->value();
        y = scrollArea->verticalScrollBar()->value();
    }

//    QWebView *webView = dynamic_cast<QWebView*>(widget);
//    if (webView) {
//        QWebFrame *frame = webView->page()->mainFrame();
//        x = frame->evaluateJavaScript("window.scrollX").toInt();
//        y = frame->evaluateJavaScript("window.scrollY").toInt();
//    }

    return QPoint(x, y);
}

static void setScrollOffset(QWidget *widget, const QPoint &p)
{
    QAbstractScrollArea *scrollArea = dynamic_cast<QAbstractScrollArea*>(widget);
    if (scrollArea) {
        scrollArea->horizontalScrollBar()->setValue(p.x());
        scrollArea->verticalScrollBar()->setValue(p.y());
    }

//    QWebView *webView = dynamic_cast<QWebView*>(widget);
//    QWebFrame *frame = webView ? webView->page()->mainFrame() : 0;
//    if (frame)
//        frame->evaluateJavaScript(QString("window.scrollTo(%1,%2);").arg(p.x()).arg(p.y()));
}

static QPoint deaccelerate(const QPoint &speed, int a = 1, int max = 64)
{
    int x = qBound(-max, speed.x(), max);
    int y = qBound(-max, speed.y(), max);
    x = (x == 0) ? x : (x > 0) ? qMax(0, x - a) : qMin(0, x + a);
    y = (y == 0) ? y : (y > 0) ? qMax(0, y - a) : qMin(0, y + a);
    return QPoint(x, y);
}

bool FlickCharm::eventFilter(QObject *object, QEvent *event)
{
 if (!object->isWidgetType()) return false;
 QEvent::Type type = event->type();

 if(type!=QEvent::MouseButtonPress &&
    type!=QEvent::MouseButtonRelease &&
    type!=QEvent::MouseMove &&

    type!=QEvent::Gesture &&
    type!=QEvent::TouchBegin &&
    type!=QEvent::TouchUpdate &&
    type!=QEvent::TouchEnd)  return false;

 QList<QTouchEvent::TouchPoint> touchPoints;
 if(type==QEvent::Gesture)
 {
  QTouchEvent *touchEvent=dynamic_cast<QTouchEvent*>(event);
  if(touchEvent) touchPoints = touchEvent->touchPoints();
 }

 QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
 QPoint mpos=(touchPoints.count()>0 ? touchPoints.first().pos().toPoint():mouseEvent->pos());

 //if(!mouseEvent || mouseEvent->modifiers() != Qt::NoModifier) return false;
 if(mouseEvent->modifiers() == Qt::ControlModifier) return false;

    QWidget *viewport = dynamic_cast<QWidget*>(object);
    FlickData *data = d->flickData.value(viewport);
    if (!viewport || !data || data->ignored.removeAll(event))
        return false;

    if(type==QEvent::Gesture )
    if(static_cast<QGestureEvent*>(event)->gesture(Qt::PinchGesture))
    {
     data->state = FlickData::Steady;
     data->speed = QPoint(0,0);
     return 0;
    }

    bool consumed = false;
    switch (data->state) {

    case FlickData::Steady:
        if (mouseEvent->type() == QEvent::MouseButtonPress)
            if (mouseEvent->buttons() == Qt::LeftButton) {
                consumed = true;
                data->state = FlickData::Pressed;
                data->pressPos = mpos;
                data->offset = scrollOffset(data->widget);
            }
        break;

    case FlickData::Pressed:
        if (mouseEvent->type() == QEvent::MouseButtonRelease) {
            consumed = true;
            data->state = FlickData::Steady;
//            if(viewport->parent()->metaObject()->className()==QString("TMapView").toLatin1())
//            {
//             qDebug() << 1;
//             QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
//             QGraphicsSceneMouseEvent release(QEvent::GraphicsSceneMouseRelease);
////             QGraphicsSceneMouseEvent *sceneEvent=(QGraphicsSceneMouseEvent*)event;
//             qDebug() << 2;
//             press.setScenePos(mpos);
//             press.setScreenPos(QCursor::pos());
//             press.setButton(Qt::LeftButton);
//             press.setPos(mpos);
//qDebug() << 2.2;
//             release.setScenePos(mpos);
//             release.setScreenPos(QCursor::pos());
//             release.setButton(Qt::LeftButton);
//             release.setPos(mpos);

//             QGraphicsView *view = (QGraphicsView*)viewport->parent();
//            qDebug() << 2.5;
//             QGraphicsScene *scene = view->scene();
//             qDebug() << 2.6;
////             QList<QGraphicsItem*> items = scene->items();

////             Q_FOREACH(QGraphicsItem * item, items )
//             {
//              //QGraphicsItem * widget = static_cast<QGraphicsWidget*>(item);
//QGraphicsItem * item=scene->itemAt(mpos);
//              // Send event to the receivers

//QMouseEvent *event1 = new QMouseEvent(QEvent::MouseButtonPress,mpos,Qt::LeftButton,Qt::LeftButton, Qt::NoModifier);
//QMouseEvent *event2 = new QMouseEvent(*mouseEvent);
//data->ignored << event1;
//data->ignored << event2;
//              scene->sendEvent(item, event1);
//              scene->sendEvent(item, event2);
//              qDebug() << item;
//             }
//            }
//            else
            {

             QMouseEvent *event1 = new QMouseEvent(QEvent::MouseButtonPress,mpos,Qt::LeftButton,Qt::LeftButton, Qt::NoModifier);
             QMouseEvent *event2 = new QMouseEvent(*mouseEvent);
             data->ignored << event1;
             data->ignored << event2;
             QApplication::postEvent(object, event1);
             QApplication::postEvent(object, event2);
            }

        }
        if (mouseEvent->type() == QEvent::MouseMove) {
            consumed = true;
            data->state = FlickData::ManualScroll;
            data->dragPos = QCursor::pos();
            if (!d->ticker.isActive())
                d->ticker.start(20, this);
        }
        break;

    case FlickData::ManualScroll:
        if (mouseEvent->type() == QEvent::MouseMove) {
            consumed = true;
            QPoint delta = mpos - data->pressPos;
            setScrollOffset(data->widget, data->offset - delta);
        }
        if (mouseEvent->type() == QEvent::MouseButtonRelease) {
            consumed = true;
            data->state = FlickData::AutoScroll;
        }
        break;

    case FlickData::AutoScroll:
        if (mouseEvent->type() == QEvent::MouseButtonPress) {
            consumed = true;
            data->state = FlickData::Stop;
            data->speed = QPoint(0, 0);
            data->pressPos = mpos;
            data->offset = scrollOffset(data->widget);
        }
        if (mouseEvent->type() == QEvent::MouseButtonRelease) {
            consumed = true;
            data->state = FlickData::Steady;
            data->speed = QPoint(0, 0);
        }
        break;

    case FlickData::Stop:
        if (mouseEvent->type() == QEvent::MouseButtonRelease) {
            consumed = true;
            data->state = FlickData::Steady;
        }
        if (mouseEvent->type() == QEvent::MouseMove) {
            consumed = true;
            data->state = FlickData::ManualScroll;
            data->dragPos = QCursor::pos();
            if (!d->ticker.isActive())
                d->ticker.start(20, this);
        }
        break;

    default:
        break;
    }

    return consumed;
}

void FlickCharm::timerEvent(QTimerEvent *event)
{
    int count = 0;
    QHashIterator<QWidget*, FlickData*> item(d->flickData);
    while (item.hasNext()) {
        item.next();
        FlickData *data = item.value();

        if (data->state == FlickData::ManualScroll) {
            count++;
            data->speed = QCursor::pos() - data->dragPos;
            data->dragPos = QCursor::pos();
        }

        if (data->state == FlickData::AutoScroll) {
            count++;
            data->speed = deaccelerate(data->speed);
            QAbstractScrollArea *scrollArea = dynamic_cast<QAbstractScrollArea*>(data->widget);

            QPoint p = QPoint(scrollArea->horizontalScrollBar()->value(),scrollArea->verticalScrollBar()->value()); //scrollOffset(data->widget);
            setScrollOffset(data->widget, p - data->speed);
            p = QPoint(scrollArea->horizontalScrollBar()->value(),scrollArea->verticalScrollBar()->value());
            if (data->speed == QPoint(0, 0) ||
                p.x() < 0 || p.y() < 0 ||
                (p.x() && p.x() >= scrollArea->horizontalScrollBar()->maximum()) ||
                (p.y() && p.y() >= scrollArea->verticalScrollBar()->maximum()) )
            {
             data->state = FlickData::Steady;
             data->speed = QPoint(0,0);
             d->ticker.stop();
             emit finished();
            }

        }
        if(data->speed.x() || data->speed.y()) emit started();
    }

    if (!count) {
        d->ticker.stop();
        emit finished();
    }

    QObject::timerEvent(event);
}

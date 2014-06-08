#include <QGesture>
#include <QMouseEvent>
#include <math.h>
#include "swipegesturerecognizer.h"

bool
SwipeGestureRecognizer::IsValidMove(int dx, int dy)
{
 // The moved distance is to small to count as not just a glitch.
 if ((qAbs(dx) < MINIMUM_DISTANCE) && (qAbs(dy) < MINIMUM_DISTANCE)) {
  return false;
 }

 return true;
}


// virtual
QGesture* SwipeGestureRecognizer::create(QObject* pTarget)
{
// qDebug("SwipeGestureRecognizer::create() called");
 QGesture *pGesture = new QSwipeGesture(pTarget);
 TimerSwipe.setInterval(400);
 TimerSwipe.setSingleShot(1);
 return pGesture;
}


// virtual
QGestureRecognizer::Result SwipeGestureRecognizer::recognize(QGesture* pGesture, QObject *pWatched, QEvent *pEvent)
{
 QGestureRecognizer::Result result = QGestureRecognizer::Ignore;
 QSwipeGesture *pSwipe = static_cast<QSwipeGesture*>(pGesture);

 switch(pEvent->type()) {
 case QEvent::MouseButtonPress: {
  QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);
  pSwipe->setProperty("startPoint", pMouseEvent->pos());
  result = QGestureRecognizer::MayBeGesture;
  TimerSwipe.start();
//  qDebug("Swipe gesture started");
 }
  break;
 case QEvent::MouseButtonRelease: {
  QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);
  const QVariant& propValue = pSwipe->property("startPoint");
  QPointF startPoint = propValue.toPointF();
  QPointF endPoint = pMouseEvent->pos();

  // process distance and direction
  int dx = endPoint.x() - startPoint.x();
  int dy = endPoint.y() - startPoint.y();

  if (!IsValidMove(dx, dy) || !TimerSwipe.isActive()) {
   // Just a click, so no gesture.
   result = QGestureRecognizer::CancelGesture;
//   qDebug("Swipe gesture canceled");
  } else {
   // Compute the angle.
   qreal angle = ComputeAngle(dx, dy);
   pSwipe->setSwipeAngle(angle);
   result = QGestureRecognizer::FinishGesture;
//   qDebug("Swipe gesture finished");
  }
 }
  break;
 default:
  break;
 }

 return result;
}

void SwipeGestureRecognizer::reset(QGesture *pGesture)
{
 pGesture->setProperty("startPoint", QVariant(QVariant::Invalid));
 parent::reset(pGesture);
}

qreal SwipeGestureRecognizer::ComputeAngle(int dx, int dy)
{
 qreal PI = 3.14159265;

 // Need to convert from screen coordinates direction
 // into classical coordinates direction.
 dy = -dy;

 qreal result = atan2((qreal)dy, (qreal)dx) ;
 result = (result * 180) / PI;

 // Always return positive angle.
 if (result < 0) {
  result += 360;
 }

 return result;
}

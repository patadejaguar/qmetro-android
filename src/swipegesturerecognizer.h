#include <QGestureRecognizer>
#include <QSwipeGesture>
#include <QTimer>

class SwipeGestureUtil: public QObject
{
public:
 static QSwipeGesture::SwipeDirection GetHorizontalDirection(QSwipeGesture *pSwipeGesture)
 {
  qreal angle = pSwipeGesture->swipeAngle();
  if (0 <= angle && angle <= 45) {
   return QSwipeGesture::Right;
  }

  if (135 <= angle && angle <= 225) {
   return QSwipeGesture::Left;
  }

  if (315 <= angle && angle <= 360) {
   return QSwipeGesture::Right;
  }

  return QSwipeGesture::NoDirection;
 }

 static QSwipeGesture::SwipeDirection GetVerticalDirection(QSwipeGesture *pSwipeGesture)
 {
  qreal angle = pSwipeGesture->swipeAngle();

  if (45 < angle && angle < 135) {
   return QSwipeGesture::Up;
  }

  if (225 < angle && angle < 315) {
   return QSwipeGesture::Down;
  }

  return QSwipeGesture::NoDirection;
 }
};

class SwipeGestureRecognizer : public QGestureRecognizer
{
public:
 SwipeGestureRecognizer(int Distance=240){MINIMUM_DISTANCE=Distance;}

  QTimer TimerSwipe;

private:
 int MINIMUM_DISTANCE;
 typedef QGestureRecognizer parent;

 bool IsValidMove(int dx, int dy);

 qreal ComputeAngle(int dx, int dy);

 virtual QGesture* create(QObject* pTarget);

 virtual QGestureRecognizer::Result recognize(QGesture* pGesture, QObject *pWatched, QEvent *pEvent);

 void reset (QGesture *pGesture);
};

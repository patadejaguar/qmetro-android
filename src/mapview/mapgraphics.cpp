#include <QDebug>
#include <QPainter>

#include "mapgraphics.h"
#include "math.h"

bool buildNaturalSpline( int size, qreal* t, qreal* p, qreal* a, qreal* b, qreal* c )
{
 int i;
 //  set up tridiagonal equation system; use coefficient
 //  vectors as temporary buffers
 qreal* h = new qreal[size-1];
 for (i = 0; i < size - 1; i++)
 {
  h[i] = t[i+1] - t[i];
  if (h[i] <= 0) {
   delete[] h;
   return false;
  }
 }

 qreal* d = new qreal[size-1];
 qreal dy1 = (p[1] - p[0]) / h[0];
 for (i = 1; i < size - 1; i++)
 {
  b[i] = c[i] = h[i];
  a[i] = qreal(2.0) * (h[i-1] + h[i]);

  const qreal dy2 = (p[i+1] - p[i]) / h[i];
  d[i] = qreal(6.0) * ( dy1 - dy2);
  dy1 = dy2;
 }

 //
 // solve it
 //

 // L-U Factorization
 for(i = 1; i < size - 2;i++)
 {
  c[i] /= a[i];
  a[i+1] -= b[i] * c[i];
 }

 // forward elimination
 qreal* s = new qreal[size];
 s[1] = d[1];
 for ( i = 2; i < size - 1; i++)
  s[i] = d[i] - c[i-1] * s[i-1];

 // backward elimination
 s[size - 2] = - s[size - 2] / a[size - 2];
 for (i = size -3; i > 0; i--)
  s[i] = - (s[i] + b[i] * s[i+1]) / a[i];
 s[size - 1] = s[0] = qreal(0);

 //
 // Finally, determine the spline coefficients
 //
 for (i = 0; i < size - 1; i++)
 {
  a[i] = ( s[i+1] - s[i] ) / ( qreal(6) * h[i]);
  b[i] = qreal(0.5) * s[i];
  c[i] = ( p[i+1] - p[i] ) / h[i]
         - (s[i+1] + qreal(2) * s[i] ) * h[i] / qreal(6);
 }

 delete[] s;
 delete[] d;
 delete[] h;
 return true;
}

bool buildPeriodicSpline( int size, qreal* t, qreal* p, qreal* a, qreal* b, qreal* c )
{
 int i;

 //
 //  setup equation system; use coefficient
 //  vectors as temporary buffers
 //
 qreal* h = new qreal[size-1];
 for (i = 0; i < size - 1; i++)
 {
  h[i] = t[i+1] - t[i];
  if (h[i] <= 0) {
   delete[] h;
   return false;
  }
 }

 qreal* d = new qreal[size-1];
 const int imax = size - 2;
 qreal htmp = h[imax];
 qreal dy1 = (p[0] - p[imax]) / htmp;
 for (i = 0; i <= imax; i++)
 {
  b[i] = c[i] = h[i];
  a[i] = qreal(2) * (htmp + h[i]);
  const qreal dy2 = (p[i+1] - p[i]) / h[i];
  d[i] = qreal(6) * ( dy1 - dy2);
  dy1 = dy2;
  htmp = h[i];
 }

 //
 // solve it
 //

 // L-U Factorization
 a[0] = sqrt(a[0]);
 c[0] = h[imax] / a[0];
 qreal sum = 0;

 for( i = 0; i < imax - 1; i++)
 {
  b[i] /= a[i];
  if (i > 0)
   c[i] = - c[i-1] * b[i-1] / a[i];
  a[i+1] = sqrt( a[i+1] - b[i]*b[i] );
  sum += c[i]*c[i];
 }
 b[imax-1] = (b[imax-1] - c[imax-2] * b[imax-2]) / a[imax-1];
 a[imax] = sqrt(a[imax] - b[imax-1]*b[imax-1] - sum);


 // forward elimination
 qreal* s = new qreal[size];
 s[0] = d[0] / a[0];
 sum = 0;
 for( i = 1; i < imax; i++)
 {
  s[i] = (d[i] - b[i-1] * s[i-1]) / a[i];
  sum += c[i-1] * s[i-1];
 }
 s[imax] = (d[imax] - b[imax-1] * s[imax-1] - sum) / a[imax];


 // backward elimination
 s[imax] = - s[imax] / a[imax];
 s[imax-1] = -(s[imax-1] + b[imax-1] * s[imax]) / a[imax-1];
 for (i= imax - 2; i >= 0; i--)
  s[i] = - (s[i] + b[i] * s[i+1] + c[i] * s[imax]) / a[i];

 //
 // Finally, determine the spline coefficients
 //
 s[size-1] = s[0];
 for ( i=0; i < size-1; i++)
 {
  a[i] = ( s[i+1] - s[i] ) / ( qreal(6) * h[i]);
  b[i] = qreal(0.5) * s[i];
  c[i] = ( p[i+1] - p[i] )
         / h[i] - (s[i+1] + qreal(2) * s[i] ) * h[i] / qreal(6);
 }

 delete[] s;
 delete[] d;
 delete[] h;
 return true;
}

QPainterPath splinePath(const QPolygonF &points)
{
 QPainterPath path;

 int i;
 int n = points.size();
 if ( n < 2 )
  return path;

 qreal* t = new qreal[n];

 qreal* px = new qreal[n];
 qreal* ax = new qreal[n];
 qreal* bx = new qreal[n];
 qreal* cx = new qreal[n];

 qreal* py = new qreal[n];
 qreal* ay = new qreal[n];
 qreal* by = new qreal[n];
 qreal* cy = new qreal[n];

 const QPointF* data = points.data();
 for (i=0; i < n; i++) {
  t[i] = i;
  px[i] = data[i].x();
  py[i] = data[i].y();
 }

 if (points.isClosed()) {
  buildPeriodicSpline( n, t, px, ax, bx, cx );
  buildPeriodicSpline( n, t, py, ay, by, cy );
 } else {
  buildNaturalSpline( n, t, px, ax, bx, cx );
  buildNaturalSpline( n, t, py, ay, by, cy );
 }

 path.moveTo( points[0] );
 for (i=0; i < n - 1; i++) {
  QPointF c1 = QPointF(px[i] + cx[i] / 3, py[i] + cy[i] / 3);
  QPointF c2 = QPointF(px[i] + cx[i] * 2/qreal(3) + bx[i] / qreal(3), py[i] + cy[i] * 2/qreal(3) + by[i] / qreal(3));
  path.cubicTo( c1, c2, points[i+1] );
 }

 delete[] cy;
 delete[] by;
 delete[] ay;
 delete[] py;

 delete[] cx;
 delete[] bx;
 delete[] ax;
 delete[] px;

 delete[] t;

 return path;
}

QPainterPath arrowPath(const QPoint &Point1,const QPoint &Point2)
{
 QPainterPath path;
 const int dy = 2;
 const int dx1 = 6;
 int x1,y1,x2,y2;
 x1=Point1.x(); y1=Point1.y(); x2=Point2.x(); y2=Point2.y();
 int DeltaX = x2 - x1;
 int DeltaY = y2 - y1;
 qreal L = sqrt((double)(DeltaX*DeltaX + DeltaY*DeltaY));

 path.moveTo(x1, y1);
 path.lineTo(x2, y2);
 path.lineTo(x2 - DeltaY * dy / L  - DeltaX * dx1 / L,y2 + DeltaX * dy / L -  DeltaY * dx1 / L);
 path.moveTo(x2, y2);
 path.lineTo(x2 + DeltaY * dy / L - DeltaX * dx1 / L, y2 - DeltaX * dy / L - DeltaY * dx1 / L);
 return path;
}

QPainterPath stairsPath(const QPolygonF &points)
{
 QPainterPath path;
 QPointF a(0,0),b(0,0),c(0,0),d(0,0),inc(0,0);

 a+=points.at(0).toPoint ();
 b+=points.at(1).toPoint ();
 c+=points.at(2).toPoint ();

 inc+=c;
 inc-=a;
 inc/=QLineF(a,c).length()/qreal(4);

 for(d+=b;  ;d+=inc,a+=inc)
 {
  QPointF tmp(0,0);
  tmp+=d;
  tmp-=inc;
  if(QLineF(d,c).length()>QLineF(tmp,c).length()-qreal(0.1)) break;

  path.moveTo(a);
  path.lineTo(d);
 }

 return path;
}

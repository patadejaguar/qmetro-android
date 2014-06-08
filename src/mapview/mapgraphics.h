#ifndef MAPGRAPHICS_H
#define MAPGRAPHICS_H
#include <QPainter>

QPainterPath splinePath(const QPolygonF &points);
QPainterPath arrowPath(const QPoint &Point1,const QPoint &Point2);
QPainterPath stairsPath(const QPolygonF &points);

#endif

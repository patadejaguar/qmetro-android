#ifndef MAPELEMENT_H
#define MAPELEMENT_H

#include <QGraphicsItem>

//#include "mapview.h"

enum ElementType{elFade,elControl};

class TMapElement : public QGraphicsItem
{
public:

 TMapElement(ElementType type,QRectF rect=QRectF(0,0,0,0));
 void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
 QRectF boundingRect() const;
 int type() const;
 qreal FadeOpacity;

 //void drawPaintCmd(QPainter *painter, QList<TPaintCmd> &paintCmd);
// void mousePressEvent(QGraphicsSceneMouseEvent *event);

 QRectF eRect;

protected:

 ElementType eType;

};

#endif // MAPELEMENT_H

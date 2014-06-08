#ifndef MAPROUTE_H
#define MAPROUTE_H

#include <QGraphicsView>
#include <QList>
#include "mapnode.h"

class TMapEdge;

class TMapRoute
{
public:
    TMapRoute();
    TMapNode* findNode(const QString &nodeName,const QString &nodePairName="");


    int ID;
    QString Name,Alias,DisplayName;
    QString mapFile;
    QColor color;        // color
    QColor blinkColor;   // ?
    QColor labelsColor;  // label color
    QColor labelsBColor; // label background
    qreal width;         // individ width
    QRect rect;
    QList<int> delay;  // day/rush/night
    QList<TMapNode*> node; // nodes and stations
    QList<TMapEdge*> edge; // edges, underconstruct

    QList<QString> temp; // temp data: name,cost

    QPainterPath pathRoute;

    //for nodes
    QPen BeginPen,EndPen,ConstructPen,TransferPen,NormalPen,
                         ConstructInfoPen,TransferInfoPen,NormalInfoPen;
    QColor ColorDark;
};

#endif // MAPROUTE_H

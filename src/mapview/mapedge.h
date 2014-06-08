#ifndef MAPEDGE_H
#define MAPEDGE_H

#include <QList>
#include <QPoint>
#include <QPolygon>

#include "mapview.h"

//class TMapRoute;
//class TMapNode;
//class QPainterPath;

class TMapEdge
{
public:
    enum EdgeType {etNormal,etTransfer,etConstruct};
    enum EdgeStyle{esNormal,esSpline,esInvisible};

    TMapEdge(TMapRoute *mapRoute, QString nodeName1, QString nodeName2,
             QString edgeCost,EdgeType edgeType=etConstruct,EdgeStyle edgeStyle=esNormal,
             TMapRoute *mapRouteTransfer=NULL);

    TMapNode *node[2];
    TMapRoute *route;
    qreal cost;       // if(!cost) cost=INF; type=etConstruct
    bool assigned;
    EdgeType type;
    EdgeStyle style;

    QPolygon addNode; // for draw additional nodes
    QPainterPath path;
};

#endif // MAPEDGE_H

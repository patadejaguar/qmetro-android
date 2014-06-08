#ifndef MAPLAYER_H
#define MAPLAYER_H

#include <QList>
#include <QString>

class TMapRoute;

class TMapLayer
{
public:
    TMapLayer();
    TMapRoute* findRoute(QString routeName);

    QString typeName;
    QList<TMapRoute*> route;
};

#endif // MAPLAYER_H

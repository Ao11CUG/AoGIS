#ifndef LAYER_H
#define LAYER_H

#include <QObject>
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qhash.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"

class Layer : public QObject
{
    Q_OBJECT

public:
    Layer(QgsMapCanvas* mMapcanvas, QObject* parent = nullptr);
    ~Layer();

    QString mAddVectorLayer(const QString& vectorFileName);
    QString mAddRasterLayer(const QString& rasterFileName);
    void removeLayer(const QString& layerId);
    void setLayerVisibility(const QString& layerId, bool visible);
    QgsMapLayer* mGetLayerById(const QString& layerId) const;

    QMap<QString, QgsMapLayer*> mLayers; // Layer ID to QgsMapLayer*
    QList<QString> mVisibleLayerIds; // Layer IDs in visible list
    QList<QString> mHiddenLayerIds;  // Layer IDs in hidden list
    QMap<QString, bool> mLayerVisibility; // Layer ID to visibility status

private:
    QgsMapCanvas* mMapCanvas;

    void updateCanvasLayers();

};


#endif


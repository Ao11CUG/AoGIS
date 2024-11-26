#include "Layer.h"
#include <QFileInfo>
#include <QDebug>
#include <QList>
#include <QMouseEvent>

Layer::Layer(QgsMapCanvas* mMapcanvas, QObject* parent)
    : QObject(parent), mMapCanvas(mMapcanvas)
{
}

Layer::~Layer() {}

// ���ʸ��ͼ�㲢��ȡͼ��id
QString Layer::mAddVectorLayer(const QString& vectorFileName)
{
    if (vectorFileName.isEmpty()) {
        qDebug() << "Error: file not found.";
        return 0;
    }

    QgsVectorLayer* vectorLayer = new QgsVectorLayer(vectorFileName, "vector layer", "ogr");
    if (!vectorLayer->isValid()) {
        qDebug() << "Error: failed to load vector layer.";
        delete vectorLayer;
        return 0;
    }

    QgsProject::instance()->addMapLayer(vectorLayer);

    QString layerId = vectorLayer->id();
    mLayers[layerId] = vectorLayer;
    mVisibleLayerIds.append(layerId);
    mLayerVisibility[layerId] = true;

    updateCanvasLayers();
    mMapCanvas->setExtent(vectorLayer->extent());
    mMapCanvas->refresh();

    return layerId;
}

// ���դ��ͼ�㲢��ȡͼ��id
QString Layer::mAddRasterLayer(const QString& rasterFileName)
{
    if (rasterFileName.isEmpty()) {
        qDebug() << "Error: file not found.";
        return 0;
    }

    QgsRasterLayer* rasterLayer = new QgsRasterLayer(rasterFileName, "raster layer");
    if (!rasterLayer->isValid()) {
        qDebug() << "Error: failed to load raster layer.";
        delete rasterLayer;
        return 0;
    }

    QgsProject::instance()->addMapLayer(rasterLayer);

    QString layerId = rasterLayer->id();
    mLayers[layerId] = rasterLayer;
    mVisibleLayerIds.append(layerId);
    mLayerVisibility[layerId] = true;

    updateCanvasLayers();
    mMapCanvas->setExtent(rasterLayer->extent());
    mMapCanvas->refresh();

    return layerId;
}

// ɾ��ͼ��
void Layer::removeLayer(const QString& layerId)
{
    if (!mLayers.contains(layerId)) {
        qDebug() << "Error: layer ID not found.";
        return;
    }

    QgsMapLayer* layer = mLayers.value(layerId);
    QgsProject::instance()->removeMapLayer(layer);

    mVisibleLayerIds.removeAll(layerId);
    mHiddenLayerIds.removeAll(layerId);
    mLayers.remove(layerId);
    mLayerVisibility.remove(layerId);

    updateCanvasLayers();
    mMapCanvas->refresh();
}

// ����ͼ��Ŀɼ���
void Layer::setLayerVisibility(const QString& layerId, bool visible)
{
    if (!mLayers.contains(layerId)) {
        qDebug() << "Error: layer ID not found.";
        return;
    }

    bool currentlyVisible = mLayerVisibility.value(layerId, false);

    if (visible != currentlyVisible) {
        if (visible) {
            if (mHiddenLayerIds.removeAll(layerId) > 0) {
                mVisibleLayerIds.append(layerId);
            }
        }
        else {
            if (mVisibleLayerIds.removeAll(layerId) > 0) {
                mHiddenLayerIds.append(layerId);
            }
        }

        mLayerVisibility[layerId] = visible;
        updateCanvasLayers();
        mMapCanvas->refresh();
    }
}

// ����ͼ��
void Layer::updateCanvasLayers()
{
    QList<QgsMapLayer*> visibleLayers;
    for (const QString& layerId : mVisibleLayerIds) {
        visibleLayers.append(mLayers.value(layerId));
    }
    mMapCanvas->setLayers(visibleLayers);
}

// ͨ��id��ȡͼ��
QgsMapLayer* Layer::mGetLayerById(const QString& layerId) const
{
    if (mLayers.contains(layerId)) {
        return mLayers.value(layerId);
    }
    else {
        qDebug() << "Error: layer ID not found.";
        return nullptr;
    }
}
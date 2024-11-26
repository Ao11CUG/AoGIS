#include "KAlgorithmAnalysis.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QgsVectorLayer.h>
#include <QgsVectorFileWriter.h>
#include <QgsField.h>
#include <QgsFeature.h>
#include <QgsGeometry.h>
#include <QgsPointXY.h>
#include <QgsFields.h>
#include <QgsVectorDataProvider.h>
#include <QgsProject.h>
#include <QgsMapLayer.h>
#include <QgsFeatureIterator.h>
#include <QVariant>
#include <QVector>
#include <QRandomGenerator>
#include <Qgis.h>
#include <QgsCoordinateReferenceSystem.h>
#include <QgsCoordinateTransformContext.h>
#include <QgsApplication.h>

KAlgorithmAnalysis::KAlgorithmAnalysis(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    // 初始化QGIS应用程序
    QgsApplication::init();
    QgsApplication::initQgis();

    setWindowTitle("Analysis of K-means clustering algorithm");
    setWindowIcon(QIcon(":/AoGIS/res/KAlgorithm.png"));

    connect(ui.openVectorFile, &QPushButton::clicked, this, &KAlgorithmAnalysis::openVectorFile);
    connect(ui.saveVectorFile, &QPushButton::clicked, this, &KAlgorithmAnalysis::saveVectorFile);
    connect(ui.begin, &QPushButton::clicked, this, &KAlgorithmAnalysis::beginAnalysis);
}

KAlgorithmAnalysis::~KAlgorithmAnalysis()
{
    QgsApplication::exitQgis();
}

void KAlgorithmAnalysis::openVectorFile() {
    mVectorFileName = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "/home",
        tr("File (*.shp)"));

    if (mVectorFileName.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(mVectorFileName);
    QString baseName = fileInfo.fileName();
    ui.vectorFileInput->setText(baseName);
}

void KAlgorithmAnalysis::saveVectorFile() {
    mSaveFileName = QFileDialog::getSaveFileName(this, tr("Save File"), "/home", tr("File (*.shp)"));
    if (mSaveFileName.isEmpty()) {
        return;
    }

    ui.vectorFileOutput->setText(mSaveFileName);
}

void KAlgorithmAnalysis::beginAnalysis() {
    if (mVectorFileName.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No file opened"));
        return;
    }

    std::unique_ptr<QgsVectorLayer> vectorLayer(new QgsVectorLayer(mVectorFileName, "InputLayer", "ogr"));
    if (!vectorLayer->isValid()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load vector file"));
        return;
    }

    QVector<QgsPointXY> points;
    QgsFeatureIterator iter = vectorLayer->getFeatures();
    QgsFeature feature;
    while (iter.nextFeature(feature)) {
        QgsGeometry geom = feature.geometry();
        if (geom.wkbType() == Qgis::WkbType::Point) {
            points.append(geom.asPoint());
        }
    }

    int numClusters = 5;
    QVector<int> labels(points.size());
    QVector<QgsPointXY> centroids(numClusters);

    //随机
    QRandomGenerator* generator = QRandomGenerator::global();
    for (int i = 0; i < numClusters; ++i) {
        int randomIndex = generator->bounded(points.size());
        centroids[i] = points[randomIndex];
    }

    bool changed;
    do {
        changed = false;

        //就近
        for (int i = 0; i < points.size(); ++i) {
            double minDist = std::numeric_limits<double>::max();
            int minIndex = 0;
            for (int j = 0; j < numClusters; ++j) {
                double dist = QgsGeometry::fromPointXY(points[i]).distance(QgsGeometry::fromPointXY(centroids[j]));
                if (dist < minDist) {
                    minDist = dist;
                    minIndex = j;
                }
            }
            if (labels[i] != minIndex) {
                labels[i] = minIndex;
                changed = true;
            }
        }

        // 更新
        QVector<QgsPointXY> newCentroids(numClusters, QgsPointXY(0, 0));
        QVector<int> counts(numClusters, 0);
        for (int i = 0; i < points.size(); ++i) {
            newCentroids[labels[i]].setX(newCentroids[labels[i]].x() + points[i].x());
            newCentroids[labels[i]].setY(newCentroids[labels[i]].y() + points[i].y());
            counts[labels[i]]++;
        }
        for (int j = 0; j < numClusters; ++j) {
            if (counts[j] != 0) {
                newCentroids[j].setX(newCentroids[j].x() / counts[j]);
                newCentroids[j].setY(newCentroids[j].y() / counts[j]);
            }
        }
        centroids = newCentroids;

    } while (changed);

    QgsFields fields = vectorLayer->fields();
    fields.append(QgsField("CLUSTER_ID", QVariant::Int));

    QgsCoordinateReferenceSystem srs = vectorLayer->crs();
    QgsCoordinateTransformContext transformContext = QgsProject::instance()->transformContext();
    std::unique_ptr<QgsVectorFileWriter> writer(new QgsVectorFileWriter(
        mSaveFileName,
        "UTF-8",
        fields,
        vectorLayer->wkbType(),
        srs,
        "ESRI Shapefile",
        QStringList(),
        QStringList(),
        nullptr,
        Qgis::FeatureSymbologyExport::NoSymbology,
        QgsFeatureSink::SinkFlags(),
        nullptr,
        transformContext,
        QgsVectorFileWriter::Original
    ));

    if (writer->hasError() != QgsVectorFileWriter::NoError) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save the result layer"));
        return;
    }

    std::unique_ptr<QgsVectorLayer> resultLayer(new QgsVectorLayer(mSaveFileName, "ClusteredLayer", "ogr"));
    if (!resultLayer->isValid()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load the result layer"));
        return;
    }

    iter = vectorLayer->getFeatures();
    while (iter.nextFeature(feature)) {
        QgsGeometry geom = feature.geometry();
        if (geom.wkbType() == Qgis::WkbType::Point) {
            QgsFeature newFeature(fields);
            newFeature.setGeometry(geom);
            newFeature.setAttributes(feature.attributes());
            newFeature.setAttribute("CLUSTER_ID", labels[points.indexOf(geom.asPoint())]);
            resultLayer->dataProvider()->addFeature(newFeature);
        }
    }

    QMessageBox::information(this, tr("Success"), tr("K-means clustering analysis completed successfully."));
}

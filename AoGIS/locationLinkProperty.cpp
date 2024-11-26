#include "locationLinkProperty.h"
#include <QFileDialog>
#include <QMessageBox>
#include <qgsapplication.h>
#include <qgsvectorlayer.h>
#include <qgsvectorfilewriter.h>
#include <qgsgeometry.h>
#include <qgspoint.h>
#include <qgsfeature.h>
#include <qgslayertreeview.h>
#include <qgslayertreelayer.h>
#include <qgsfield.h>
#include <qgsfeatureiterator.h>
#include <qgsvectorlayereditbuffer.h>
#include <qgsmaplayer.h>
#include <QFileInfo>

locationLinkProperty::locationLinkProperty(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.openVectorFile, &QPushButton::clicked, this, &locationLinkProperty::openVectorFile);
	connect(ui.openPoiFile, &QPushButton::clicked, this, &locationLinkProperty::openPoiFile);
	connect(ui.saveVectorFile, &QPushButton::clicked, this, &locationLinkProperty::saveVectorFile);
	connect(ui.begin, &QPushButton::clicked, this, &locationLinkProperty::beginAnalysis);
}

locationLinkProperty::~locationLinkProperty()
{}

void locationLinkProperty::openVectorFile() {
	mVectorFileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("File (*.shp)"));

	QFileInfo fileInfo(mVectorFileName);
	QString baseName = fileInfo.fileName();
	ui.vectorFileInput->setText(baseName);
};

void locationLinkProperty::openPoiFile() {
	mPoiFileName = QFileDialog::getOpenFileName(this, tr("OpenFile"), "", tr("File (*.shp)"));

	QFileInfo fileInfo(mVectorFileName);
	QString baseName = fileInfo.fileName();
	ui.poiFileInput->setText(baseName);
};

void locationLinkProperty::saveVectorFile() {
	mSaveFileName = QFileDialog::getSaveFileName(this, tr("SaveFile"), "", tr("File (*.shp)"));

	ui.vectorFileOutput->setText(mSaveFileName);
};

void locationLinkProperty::beginAnalysis() {
	QgsVectorLayer* vectorLayer = new QgsVectorLayer(mVectorFileName, "Vector Layer", "ogr");
	if (!vectorLayer->isValid()) {
		QMessageBox::critical(this, tr("Error"), tr("Failed to open vector file."));
		return;
	}
	
	QgsVectorLayer*  poiLayer = new QgsVectorLayer(mPoiFileName, "POI Layer", "ogr");
	if (!poiLayer->isValid()) {
		QMessageBox::critical(this, tr("Error"), tr("Failed to open POI file."));
		return;
	}

	QgsFields fields = vectorLayer->fields();
	fields.append(QgsField("PoiId", QVariant::Int));
	std::unique_ptr<QgsVectorFileWriter> writer(new QgsVectorFileWriter(
		mSaveFileName, "UTF-8", fields, Qgis::WkbType::Polygon, vectorLayer->crs(), "ESRI Shapefile"));

	QgsFeatureIterator vectorFeatures = vectorLayer->getFeatures();
	QgsFeature vectorFeature;
	QgsFeature poiFeature;

	while (vectorFeatures.nextFeature(vectorFeature)) {
		QgsGeometry vectorGeom = vectorFeature.geometry();
		int poiId = -1;

		QgsFeatureIterator poiFeatures = poiLayer->getFeatures();
		while (poiFeatures.nextFeature(poiFeature)) {
			if (vectorGeom.intersects(poiFeature.geometry())) {
				poiId = poiFeature.id();
				break;
			}
		}

		vectorFeature.setFields(fields, true);
		vectorFeature.setAttribute("PoiId", poiId);
		writer->addFeature(vectorFeature);
	}

	QMessageBox::information(this, tr("Success"), tr("Analysis completed and new shapefile saved."));
};
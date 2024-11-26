#include <QWidget>
#include "ui_rasterLayerStatistics.h"
#include <QFileDialog>
#include <qgsrasterlayer.h>
#include <qgsrasterbandstats.h>
#include <qgsrasterdataprovider.h>
#include <qgssinglebandgrayrenderer.h>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>

class rasterLayerStatistics : public QWidget
{
	Q_OBJECT

public:
	rasterLayerStatistics(QWidget *parent = nullptr);
	~rasterLayerStatistics();

	QString mRasterFileName;
	QString mCsvFileName;
	QgsRasterLayer *mRasterLayer;
	int wnSelectedBand;
	QgsRasterBandStats mBandStats;

public slots:
	void openRasterFile();
	void saveCsvFile();
	void beginStatistics();

private:
	Ui::rasterLayerStatisticsClass ui;
};

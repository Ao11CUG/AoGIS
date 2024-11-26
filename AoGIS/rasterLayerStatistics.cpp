#include "rasterLayerStatistics.h"

rasterLayerStatistics::rasterLayerStatistics(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    setWindowTitle("RasterLayerStatistics");
    setWindowIcon(QIcon(":/AoGIS/res/rasterLayerStatistics.png"));

    connect(ui.begin, &QPushButton::clicked, this, &rasterLayerStatistics::beginStatistics);
    connect(ui.openRasterFile, &QPushButton::clicked, this, &rasterLayerStatistics::openRasterFile);
    connect(ui.saveCsvFile, &QPushButton::clicked, this, &rasterLayerStatistics::saveCsvFile);
}

rasterLayerStatistics::~rasterLayerStatistics()
{
    delete mRasterLayer;
}

void rasterLayerStatistics::openRasterFile() {
    mRasterFileName = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "/home",
        tr("File (*.tif)"));

    if (mRasterFileName.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(mRasterFileName);
    QString baseName = fileInfo.fileName();

    ui.rasterFileInput->setText(baseName);

    //生成栅格图层
    mRasterLayer = new QgsRasterLayer(mRasterFileName, "Raster Layer");
    if (!mRasterLayer->isValid()) {
        delete mRasterLayer;
        mRasterLayer = nullptr;
        QMessageBox::warning(this, tr("Error"), tr("Failed to load the raster layer."));
        return;
    }

    //由输入的栅格图层生成对应的波段
    ui.chooseBand->clear();
    int bandCount = mRasterLayer->bandCount();
    for (int i = 1; i <= bandCount; ++i) {
        ui.chooseBand->addItem(tr("Band %1").arg(i), i);
    }
}

void rasterLayerStatistics::saveCsvFile() {
    wnSelectedBand = ui.chooseBand->currentData().toInt();
    if (!mRasterLayer || wnSelectedBand <= 0) {
        QMessageBox::warning(this, tr("Error"), tr("No band selected for analysis."));
        return;
    }

    mCsvFileName = QFileDialog::getSaveFileName(
        this,
        tr("Save File"),
        "/home",
        tr("CSV Files (*.csv)"));

    if (mCsvFileName.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(mCsvFileName);
    QString baseName = fileInfo.fileName();

    ui.csvFileOutput->setText(baseName);
}

void rasterLayerStatistics::beginStatistics() {
    if (!mRasterLayer || ui.chooseBand->count() == 0) {
        QMessageBox::warning(this, tr("Error"), tr("No raster file or band selected for analysis."));
        return;
    }

    if (mCsvFileName.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No save file specified."));
        return;
    }

    QgsRasterDataProvider* provider = mRasterLayer->dataProvider();

    mBandStats = provider->bandStatistics(wnSelectedBand, QgsRasterBandStats::All);

    QFile file(mCsvFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to open file for writing."));
        return;
    }

    //计算波段1
    if (wnSelectedBand == 1) {
        QTextStream out(&file);
        out << "Band, Minimum Value, Maximum Value, Mean, Standard Deviation\n";
        out << wnSelectedBand << ", "
            << mBandStats.minimumValue << ", "
            << mBandStats.maximumValue << ", "
            << mBandStats.mean << ", "
            << mBandStats.stdDev << "\n";

        file.close();
        QMessageBox::information(this, tr("Success"), tr("Statistics saved to file successfully."));
    }
}

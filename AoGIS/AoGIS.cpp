#include "AoGIS.h"
#include <QFileDialog>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileInfo>
#include <QVBoxLayout>
#include "qgsapplication.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgssinglesymbolrenderer.h"
#include "qgscoordinatereferencesystem.h"
#include <QMessageBox>
#include <QPushButton>
#include <QMouseEvent>
#include <QLabel>
#include <QStatusBar>
#include "qgsattributetableview.h"
#include <qgsattributetablemodel.h>
#include <QDialog>
#include <qgsattributetablefiltermodel.h>
#include <qgsvectorlayercache.h>

AoGIS::AoGIS(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setWindowIcon(QIcon(":/AoGIS/res/AoGIS.png"));

    // 初始化QGIS应用程序
    QgsApplication::init();
    QgsApplication::initQgis();

    // 初始化地图画布
    mMapcanvas = new QgsMapCanvas(this);
    mMapcanvas->setCanvasColor(Qt::white);

    // 设置地图画布的坐标参考系
    QgsCoordinateReferenceSystem crs("EPSG:4326"); // WGS 84
    mMapcanvas->setDestinationCrs(crs);

    // 初始化坐标转换
    mCoordinateTransform = new QgsCoordinateTransform(crs, crs, QgsProject::instance());

    // 初始化 Layer 管理器
    mLayerManager = new Layer(mMapcanvas, this);

    // 将地图画布添加到 scrollArea 中
    ui.scrollArea->setWidget(mMapcanvas);

    // 创建一个 QStandardItemModel 并设置给 listView
    QStandardItemModel* model = new QStandardItemModel(this);
    ui.listView->setModel(model);

    // 初始化状态栏标签
    mCoordinateLabel = new QLabel(this);
    mCoordinateLabel->setText("Ready"); // 初始文本
    ui.statusBar->addPermanentWidget(mCoordinateLabel);

    // 连接信号与槽
    connect(ui.deleteButton, &QPushButton::clicked, this, &AoGIS::removeSelectedItem);
    connect(model, &QStandardItemModel::itemChanged, this, &AoGIS::updateLayerVisibility);
    connect(ui.openVector, &QAction::triggered, this, &AoGIS::openVectorFile);
    connect(ui.openRaster, &QAction::triggered, this, &AoGIS::openRasterFile);
    connect(ui.openText, &QAction::triggered, this, &AoGIS::openTextFile);
    connect(ui.saveFile, &QAction::triggered, this, &AoGIS::saveAllMap);
    connect(ui.openLayerManagement, &QAction::triggered, this, &AoGIS::openLayerManagement);
    connect(ui.openToolBox, &QAction::triggered, this, &AoGIS::openToolBox);
    connect(ui.openLog, &QAction::triggered, this, &AoGIS::openLog);
    connect(ui.rasterLayerStatistics, &QAction::triggered, this, &AoGIS::showRasterLayerStatistics);
    connect(ui.rasterLayerStatisticsButton, &QPushButton::clicked, this, &AoGIS::showRasterLayerStatistics);
    connect(ui.viewVectorLayerAttributes, &QPushButton::clicked, this, &AoGIS::viewVectorLayerAttributes);
    connect(ui.KAlgorithmAnalysis, &QAction::triggered, this, &AoGIS::showKAlgorithmAnalysis);
    connect(ui.KAlgorithmButton, &QPushButton::clicked, this, &AoGIS::showKAlgorithmAnalysis);
    connect(ui.locationLinkProperty, &QAction::triggered, this, &AoGIS::showLocationLinkProperty);
    connect(ui.locationLinkPropertyButton, &QPushButton::clicked, this, &AoGIS::showLocationLinkProperty);

    // 连接地图画布的鼠标点击事件
    connect(mMapcanvas, &QgsMapCanvas::xyCoordinates, this, &AoGIS::updateStatusBar);
}

AoGIS::~AoGIS()
{
    QgsApplication::exitQgis();
}

//打开矢量文件
void AoGIS::openVectorFile() {
    QString vectorFileName = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "/home",
        tr("File (*.shp)"));

    if (vectorFileName.isEmpty()) {
        logMessage("Error: no valid file found!");
        return;
    }

    logMessage("Opening file: " + vectorFileName);

    QIcon vectorFileIcon = QIcon(":/AoGIS/res/vector.png");

    // 将文件名添加到 listView 的 model 中
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.listView->model());

    QFileInfo fileInfo(vectorFileName);
    QString vectorFileBaseName = fileInfo.fileName();
    QStandardItem* item = new QStandardItem(vectorFileIcon, vectorFileBaseName);
    item->setCheckable(true); // 设置复选框
    item->setCheckState(Qt::Checked); // 默认选中

    // 加载图层并获取图层 ID
    QString layerId = mLayerManager->mAddVectorLayer(vectorFileName);
    if (!layerId.isEmpty()) {
        item->setData(layerId, Qt::UserRole); // 设置图层 ID

        model->appendRow(item);

        logMessage("File added to list: " + vectorFileBaseName);
        logMessage("Vector file opened successfully: " + vectorFileName);
        logMessage("Layer ID: " + layerId);
    }
    else {
        logMessage("Error: failed to open vector file!");
        delete item; // 失败时删除未添加的 item
    }
}

//打开栅格文件
void AoGIS::openRasterFile() {
    QString rasterFileName = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "/home",
        tr("File (*.tif)"));

    if (rasterFileName.isEmpty()) {
        logMessage("Error:no valid file found!");
        return;
    }

    logMessage("Opening file: " + rasterFileName);

    QIcon rasterFileIcon = QIcon(":/AoGIS/res/raster.png");

    // 将文件名添加到 listView 的 model 中
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.listView->model());

    QFileInfo fileInfo(rasterFileName);

    QString rasterFileBaseName = fileInfo.fileName();
    QStandardItem* item = new QStandardItem(rasterFileIcon, rasterFileBaseName);
    item->setCheckable(true); // 设置复选框
    item->setCheckState(Qt::Checked); // 默认选中

    // 加载图层并获取图层 ID
    QString layerId = mLayerManager->mAddRasterLayer(rasterFileName);
    if (!layerId.isEmpty()) {
        item->setData(layerId, Qt::UserRole); // 设置图层 ID

        model->appendRow(item);

        logMessage("File added to list: " + rasterFileBaseName);
        logMessage("Vector file opened successfully: " + rasterFileName);
        logMessage("Layer ID: " + layerId);
    }
    else {
        logMessage("Error: failed to open vector file!");
        delete item; // 失败时删除未添加的 item
    }
}

//打开分隔符文本文件
void AoGIS::openTextFile() {
    QString textFileName = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "/home",
        tr("File (*.csv)"));

    if (textFileName.isEmpty()) {
        logMessage("Error:file not found.");
        return;
    }

    logMessage("Opening file: " + textFileName);

    QIcon textFileIcon = QIcon(":/AoGIS/res/text.png");

    // 将文件名添加到 listView 的 model 中
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.listView->model());

    QFileInfo fileInfo(textFileName);

    QString textFileBaseName = fileInfo.fileName();
    QStandardItem* item = new QStandardItem(textFileIcon, textFileBaseName);
    item->setCheckable(true); // 设置复选框
    item->setCheckState(Qt::Checked); // 默认选中
    model->appendRow(item);

    logMessage("File added to list: " + textFileBaseName);

    // 读取CSV文件内容并显示在QTableView中
    QFile file(textFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        logMessage("Error: failed to open CSV file.");
        return;
    }

    QTableView* tableView = new QTableView(this);
    QStandardItemModel* csvModel = new QStandardItemModel(this);

    QTextStream in(&file);
    QStringList headers;
    bool firstLine = true;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(",");

        if (firstLine) {
            headers = fields;
            csvModel->setHorizontalHeaderLabels(headers);
            firstLine = false;
        }
        else {
            QList<QStandardItem*> items;
            for (const QString& field : fields) {
                items.append(new QStandardItem(field));
            }
            csvModel->appendRow(items);
        }
    }
    file.close();

    tableView->setModel(csvModel);
    tableView->resize(800, 600); 
    tableView->show(); 

    logMessage("File opened successfully: " + textFileName);
}

//删除功能
void AoGIS::removeSelectedItem()
{
    // 获取当前 ListView 的 model
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.listView->model());
    if (!model) {
        logMessage("Error: failed to get model.");
        return;
    }

    // 获取选中的项的索引
    QModelIndexList selectedIndexes = ui.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        logMessage("Error: no selected item found!");
        return;
    }

    // 按照从大到小的顺序移除项，以免影响未处理项的索引
    QList<int> rowsToRemove;
    for (const QModelIndex& index : selectedIndexes) {
        int row = index.row();
        rowsToRemove.append(row);
    }
    std::sort(rowsToRemove.rbegin(), rowsToRemove.rend());

    // 使用 QStandardItemModel 的行索引找到图层 ID
    for (int row : rowsToRemove) {
        // 获取被删除项的 ID
        QStandardItem* item = model->item(row);
        if (!item) continue;

        QString layerId = item->data(Qt::UserRole).toString(); // 假设图层 ID 存储在 Qt::UserRole 中

        // 从 listView 的 model 中移除选中的项
        model->removeRow(row);

        // 删除对应的图层
        mLayerManager->removeLayer(layerId);

        logMessage("Selected item " + QString::number(row + 1) + " removed.");
    }
}

//隐藏功能
void AoGIS::updateLayerVisibility(QStandardItem* item)
{
    if (!item)
        return;

    // 获取图层 ID
    QString layerId = item->data(Qt::UserRole).toString(); // 假设图层 ID 存储在 Qt::UserRole 中
    bool isVisible = (item->checkState() == Qt::Checked);

    // 确保图层 ID 存在于 mLayerManager 中
    if (mLayerManager->mLayers.contains(layerId)) {
        mLayerManager->setLayerVisibility(layerId, isVisible);

        if (isVisible) {
            logMessage("Layer " + item->text() + " is now visible.");
        }
        else {
            logMessage("Layer " + item->text() + " is now hidden.");
        }
    }
    else {
        logMessage("Error: Layer ID not found.");
    }
}

//打开图层管理器
void AoGIS::openLayerManagement() {
    logMessage("Opening layer management.");

    if (ui.leftDock && !ui.leftDock->isVisible()) {
        ui.leftDock->setVisible(true);
    }

    logMessage("Layer management opened successfully.");
}

//打开工具箱
void AoGIS::openToolBox() {
    logMessage("Opening toolbox.");

    if (ui.rightDock && !ui.rightDock->isVisible()) {
        ui.rightDock->setVisible(true);
    }

    logMessage("Toolbox opened successfully.");
}

//打开程序执行日志
void AoGIS::openLog() {
    logMessage("Opening program execution log.");

    if (ui.bottomDock && !ui.bottomDock->isVisible()) {
        ui.bottomDock->setVisible(true);
    }

    logMessage("Program execution log opened successfully.");
}

//日志输出
void AoGIS::logMessage(const QString& message) {
    if (ui.log) {
        ui.log->append(message);
    }
}

//更新状态栏经纬度
void AoGIS::updateStatusBar(const QgsPointXY& point)
{
    // 将屏幕坐标转换为地图坐标
    QgsPointXY mapPoint = mCoordinateTransform->transform(point);

    QString coordinateText = QString("Latitude: %1, Lontitude: %2").arg(mapPoint.x()).arg(mapPoint.y());
    mCoordinateLabel->setText(coordinateText);
}

//跳转栅格图层统计
void AoGIS::showRasterLayerStatistics() {
    logMessage("Starting RasterLayerStatistics.");
    rasterLayerStatistics* RasterLayerStatistics = new rasterLayerStatistics();
    RasterLayerStatistics->show();
}

//查看矢量图层属性表
void AoGIS::viewVectorLayerAttributes() {
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.listView->model());
    if (!model) {
        logMessage("Error: failed to get model.");
        return;
    }

    QModelIndexList selectedIndexes = ui.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        logMessage("Error: no selected item found!");
        return;
    }

    QModelIndex index = selectedIndexes.first();
    QStandardItem* item = model->itemFromIndex(index);
    if (!item) {
        logMessage("Error: failed to get selected item.");
        return;
    }

    QString layerId = item->data(Qt::UserRole).toString();
    if (layerId.isEmpty()) {
        logMessage("Error: no layer ID found in the selected item.");
        return;
    }

    QgsMapLayer* layer = mLayerManager->mGetLayerById(layerId);
    if (!layer) {
        logMessage("Error: no layer found with the given ID.");
        return;
    }

    QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer*>(layer);
    if (!vectorLayer) {
        logMessage("Error: the selected layer is not a vector layer.");
        return;
    }

     QDialog* attributeDialog = new QDialog;
     attributeDialog->setAttribute(Qt::WA_DeleteOnClose);
     attributeDialog->setWindowTitle("Attribute Table");

     QgsVectorLayerCache* cache = new QgsVectorLayerCache(vectorLayer, 10000, attributeDialog);
     QgsAttributeTableModel* attributeModel = new QgsAttributeTableModel(cache, attributeDialog);
     attributeModel->loadLayer(); // 确保模型加载图层数据

     QgsAttributeTableFilterModel* filterModel = new QgsAttributeTableFilterModel(mMapcanvas, attributeModel, attributeDialog);

     QgsAttributeTableView* attributeView = new QgsAttributeTableView(attributeDialog);
     attributeView->setModel(filterModel);

     QVBoxLayout* layout = new QVBoxLayout;
     layout->addWidget(attributeView);
     attributeDialog->setLayout(layout);

     attributeDialog->resize(800, 600);
     attributeDialog->show();   
}

//跳转K均值聚类算法分析
void AoGIS::showKAlgorithmAnalysis() {
    logMessage("Starting analysis of k-means clustering algorithm.");
    KAlgorithmAnalysis* kAlgorithmAnalysis = new KAlgorithmAnalysis();
    kAlgorithmAnalysis->show();
}

//跳转按位置连接属性
void AoGIS::showLocationLinkProperty() {
    locationLinkProperty* LocationLinkProperty = new locationLinkProperty();
    LocationLinkProperty->show();
}

//保存地图
void AoGIS::saveAllMap() {
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save File"),
        "",
        tr("QGIS Project (*.qgz)")
    );

    if (filePath.isEmpty()) {
        logMessage("Error: no valid file path specified!");
        return;
    }

    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();

    if (extension == "qgz") {
        if (QgsProject::instance()->write(filePath)) {
            logMessage("Map document saved successfully: " + filePath);
        }
        else {
            logMessage("Error: failed to save map document!");
            QMessageBox::critical(this, tr("Error"), tr("Failed to save map document."));
        }
    }
    else {
        logMessage("Error: unsupported file format!");
        QMessageBox::critical(this, tr("Error"), tr("Unsupported file format! Please select .qgz format."));
    }
}
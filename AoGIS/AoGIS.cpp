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

    // ��ʼ��QGISӦ�ó���
    QgsApplication::init();
    QgsApplication::initQgis();

    // ��ʼ����ͼ����
    mMapcanvas = new QgsMapCanvas(this);
    mMapcanvas->setCanvasColor(Qt::white);

    // ���õ�ͼ����������ο�ϵ
    QgsCoordinateReferenceSystem crs("EPSG:4326"); // WGS 84
    mMapcanvas->setDestinationCrs(crs);

    // ��ʼ������ת��
    mCoordinateTransform = new QgsCoordinateTransform(crs, crs, QgsProject::instance());

    // ��ʼ�� Layer ������
    mLayerManager = new Layer(mMapcanvas, this);

    // ����ͼ������ӵ� scrollArea ��
    ui.scrollArea->setWidget(mMapcanvas);

    // ����һ�� QStandardItemModel �����ø� listView
    QStandardItemModel* model = new QStandardItemModel(this);
    ui.listView->setModel(model);

    // ��ʼ��״̬����ǩ
    mCoordinateLabel = new QLabel(this);
    mCoordinateLabel->setText("Ready"); // ��ʼ�ı�
    ui.statusBar->addPermanentWidget(mCoordinateLabel);

    // �����ź����
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

    // ���ӵ�ͼ������������¼�
    connect(mMapcanvas, &QgsMapCanvas::xyCoordinates, this, &AoGIS::updateStatusBar);
}

AoGIS::~AoGIS()
{
    QgsApplication::exitQgis();
}

//��ʸ���ļ�
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

    // ���ļ�����ӵ� listView �� model ��
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.listView->model());

    QFileInfo fileInfo(vectorFileName);
    QString vectorFileBaseName = fileInfo.fileName();
    QStandardItem* item = new QStandardItem(vectorFileIcon, vectorFileBaseName);
    item->setCheckable(true); // ���ø�ѡ��
    item->setCheckState(Qt::Checked); // Ĭ��ѡ��

    // ����ͼ�㲢��ȡͼ�� ID
    QString layerId = mLayerManager->mAddVectorLayer(vectorFileName);
    if (!layerId.isEmpty()) {
        item->setData(layerId, Qt::UserRole); // ����ͼ�� ID

        model->appendRow(item);

        logMessage("File added to list: " + vectorFileBaseName);
        logMessage("Vector file opened successfully: " + vectorFileName);
        logMessage("Layer ID: " + layerId);
    }
    else {
        logMessage("Error: failed to open vector file!");
        delete item; // ʧ��ʱɾ��δ��ӵ� item
    }
}

//��դ���ļ�
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

    // ���ļ�����ӵ� listView �� model ��
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.listView->model());

    QFileInfo fileInfo(rasterFileName);

    QString rasterFileBaseName = fileInfo.fileName();
    QStandardItem* item = new QStandardItem(rasterFileIcon, rasterFileBaseName);
    item->setCheckable(true); // ���ø�ѡ��
    item->setCheckState(Qt::Checked); // Ĭ��ѡ��

    // ����ͼ�㲢��ȡͼ�� ID
    QString layerId = mLayerManager->mAddRasterLayer(rasterFileName);
    if (!layerId.isEmpty()) {
        item->setData(layerId, Qt::UserRole); // ����ͼ�� ID

        model->appendRow(item);

        logMessage("File added to list: " + rasterFileBaseName);
        logMessage("Vector file opened successfully: " + rasterFileName);
        logMessage("Layer ID: " + layerId);
    }
    else {
        logMessage("Error: failed to open vector file!");
        delete item; // ʧ��ʱɾ��δ��ӵ� item
    }
}

//�򿪷ָ����ı��ļ�
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

    // ���ļ�����ӵ� listView �� model ��
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.listView->model());

    QFileInfo fileInfo(textFileName);

    QString textFileBaseName = fileInfo.fileName();
    QStandardItem* item = new QStandardItem(textFileIcon, textFileBaseName);
    item->setCheckable(true); // ���ø�ѡ��
    item->setCheckState(Qt::Checked); // Ĭ��ѡ��
    model->appendRow(item);

    logMessage("File added to list: " + textFileBaseName);

    // ��ȡCSV�ļ����ݲ���ʾ��QTableView��
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

//ɾ������
void AoGIS::removeSelectedItem()
{
    // ��ȡ��ǰ ListView �� model
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.listView->model());
    if (!model) {
        logMessage("Error: failed to get model.");
        return;
    }

    // ��ȡѡ�е��������
    QModelIndexList selectedIndexes = ui.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        logMessage("Error: no selected item found!");
        return;
    }

    // ���մӴ�С��˳���Ƴ������Ӱ��δ�����������
    QList<int> rowsToRemove;
    for (const QModelIndex& index : selectedIndexes) {
        int row = index.row();
        rowsToRemove.append(row);
    }
    std::sort(rowsToRemove.rbegin(), rowsToRemove.rend());

    // ʹ�� QStandardItemModel ���������ҵ�ͼ�� ID
    for (int row : rowsToRemove) {
        // ��ȡ��ɾ����� ID
        QStandardItem* item = model->item(row);
        if (!item) continue;

        QString layerId = item->data(Qt::UserRole).toString(); // ����ͼ�� ID �洢�� Qt::UserRole ��

        // �� listView �� model ���Ƴ�ѡ�е���
        model->removeRow(row);

        // ɾ����Ӧ��ͼ��
        mLayerManager->removeLayer(layerId);

        logMessage("Selected item " + QString::number(row + 1) + " removed.");
    }
}

//���ع���
void AoGIS::updateLayerVisibility(QStandardItem* item)
{
    if (!item)
        return;

    // ��ȡͼ�� ID
    QString layerId = item->data(Qt::UserRole).toString(); // ����ͼ�� ID �洢�� Qt::UserRole ��
    bool isVisible = (item->checkState() == Qt::Checked);

    // ȷ��ͼ�� ID ������ mLayerManager ��
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

//��ͼ�������
void AoGIS::openLayerManagement() {
    logMessage("Opening layer management.");

    if (ui.leftDock && !ui.leftDock->isVisible()) {
        ui.leftDock->setVisible(true);
    }

    logMessage("Layer management opened successfully.");
}

//�򿪹�����
void AoGIS::openToolBox() {
    logMessage("Opening toolbox.");

    if (ui.rightDock && !ui.rightDock->isVisible()) {
        ui.rightDock->setVisible(true);
    }

    logMessage("Toolbox opened successfully.");
}

//�򿪳���ִ����־
void AoGIS::openLog() {
    logMessage("Opening program execution log.");

    if (ui.bottomDock && !ui.bottomDock->isVisible()) {
        ui.bottomDock->setVisible(true);
    }

    logMessage("Program execution log opened successfully.");
}

//��־���
void AoGIS::logMessage(const QString& message) {
    if (ui.log) {
        ui.log->append(message);
    }
}

//����״̬����γ��
void AoGIS::updateStatusBar(const QgsPointXY& point)
{
    // ����Ļ����ת��Ϊ��ͼ����
    QgsPointXY mapPoint = mCoordinateTransform->transform(point);

    QString coordinateText = QString("Latitude: %1, Lontitude: %2").arg(mapPoint.x()).arg(mapPoint.y());
    mCoordinateLabel->setText(coordinateText);
}

//��תդ��ͼ��ͳ��
void AoGIS::showRasterLayerStatistics() {
    logMessage("Starting RasterLayerStatistics.");
    rasterLayerStatistics* RasterLayerStatistics = new rasterLayerStatistics();
    RasterLayerStatistics->show();
}

//�鿴ʸ��ͼ�����Ա�
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
     attributeModel->loadLayer(); // ȷ��ģ�ͼ���ͼ������

     QgsAttributeTableFilterModel* filterModel = new QgsAttributeTableFilterModel(mMapcanvas, attributeModel, attributeDialog);

     QgsAttributeTableView* attributeView = new QgsAttributeTableView(attributeDialog);
     attributeView->setModel(filterModel);

     QVBoxLayout* layout = new QVBoxLayout;
     layout->addWidget(attributeView);
     attributeDialog->setLayout(layout);

     attributeDialog->resize(800, 600);
     attributeDialog->show();   
}

//��תK��ֵ�����㷨����
void AoGIS::showKAlgorithmAnalysis() {
    logMessage("Starting analysis of k-means clustering algorithm.");
    KAlgorithmAnalysis* kAlgorithmAnalysis = new KAlgorithmAnalysis();
    kAlgorithmAnalysis->show();
}

//��ת��λ����������
void AoGIS::showLocationLinkProperty() {
    locationLinkProperty* LocationLinkProperty = new locationLinkProperty();
    LocationLinkProperty->show();
}

//�����ͼ
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
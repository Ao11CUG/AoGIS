#include <QMainWindow>
#include "ui_AoGIS.h"
#include "Layer.h"
#include "qgsmapcanvas.h"
#include "qgscoordinatetransform.h"
#include <QStandardItem>
#include <QLabel>
#include "rasterLayerStatistics.h"
#include "KAlgorithmAnalysis.h"
#include "locationLinkProperty.h"

class AoGIS : public QMainWindow
{
    Q_OBJECT

public:
    AoGIS(QWidget* parent = nullptr);
    ~AoGIS();

    void logMessage(const QString& message);

public slots:
    void openVectorFile();
    void openRasterFile();
    void openTextFile();
    void openLayerManagement();
    void openToolBox();
    void openLog();
    void removeSelectedItem();
    void updateLayerVisibility(QStandardItem* item);
    void updateStatusBar(const QgsPointXY& point);
    void showRasterLayerStatistics();
    void viewVectorLayerAttributes();
    void showKAlgorithmAnalysis();
    void showLocationLinkProperty();
    void saveAllMap();

private:
    Ui::AoGISClass ui;
    QgsMapCanvas* mMapcanvas;
    Layer* mLayerManager;

    // ������ʾ�����״̬����ǩ
    QLabel* mCoordinateLabel;

    // ��������ת��
    QgsCoordinateTransform* mCoordinateTransform;
};

#pragma once

#include <QWidget>
#include "ui_locationLinkProperty.h"

class locationLinkProperty : public QWidget
{
	Q_OBJECT

public:
	locationLinkProperty(QWidget *parent = nullptr);
	~locationLinkProperty();

	QString mVectorFileName;
	QString mPoiFileName;
	QString mSaveFileName;

public slots:
	void openVectorFile();
	void openPoiFile();
	void saveVectorFile();
	void beginAnalysis();

private:
	Ui::locationLinkPropertyClass ui;
};

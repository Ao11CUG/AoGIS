#pragma once

#include <QWidget>
#include "ui_KAlgorithmAnalysis.h"
#include "QgsVectorLayer.h"
#include <memory>

class KAlgorithmAnalysis : public QWidget
{
	Q_OBJECT

public:
	KAlgorithmAnalysis(QWidget *parent = nullptr);
	~KAlgorithmAnalysis();

	QString mVectorFileName;
	QString mSaveFileName;

public slots:
	void openVectorFile();
	void saveVectorFile();
	void beginAnalysis();

private:
	Ui::KAlgorithmAnalysisClass ui;
};

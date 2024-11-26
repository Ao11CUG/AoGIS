#include "AoGIS.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AoGIS w;
    w.show();
    return a.exec();
}

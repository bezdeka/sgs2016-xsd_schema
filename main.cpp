#include "xsd.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    xsd w;
    w.show();

    return a.exec();
}

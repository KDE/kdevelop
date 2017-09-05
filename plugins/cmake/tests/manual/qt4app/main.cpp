#include <QtGui/QApplication>
#include "qt4app.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    qt4app foo;
    foo.show();
    return app.exec();
}

#include <QtGui>

#include "docwidget.h"

int main(int ac, char** av)
{
    QApplication app(ac,av);
    DocWidget d;
    d.show();
    return app.exec();
}

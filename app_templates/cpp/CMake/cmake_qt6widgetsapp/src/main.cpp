#include "%{APPNAMELC}.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    %{APPNAMEID} w;
    w.show();

    return app.exec();
}


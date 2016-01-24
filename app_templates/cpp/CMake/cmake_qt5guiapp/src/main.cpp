#include "%{APPNAMELC}.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    %{APPNAME} w;
    w.show();

    return app.exec();
}


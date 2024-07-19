#include "%{APPNAMELC}.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto w = new %{APPNAMEID};
    w->show();

    return app.exec();
}


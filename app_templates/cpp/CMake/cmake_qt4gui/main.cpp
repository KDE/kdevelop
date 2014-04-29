#include <QApplication>
#include "%{APPNAME}.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    %{APPNAMEID} %{APPNAMELC};
    %{APPNAMELC}.show();
    return app.exec();
}

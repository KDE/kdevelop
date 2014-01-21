#include <QApplication>
#include "%{APPNAME}.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    %{APPNAMEID} foo;
    foo.show();
    return app.exec();
}

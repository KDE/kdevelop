#include <QCoreApplication>
#include "%{APPNAME}.h"


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    %{APPNAME} foo;
    return app.exec();
}

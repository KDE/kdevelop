#include <QCoreApplication>
#include "%{APPNAMEID}.h"


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    %{APPNAMEID} foo;
    return app.exec();
}

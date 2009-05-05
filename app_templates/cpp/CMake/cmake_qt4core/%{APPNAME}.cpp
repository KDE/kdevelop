#include "%{APPNAME}.h"

#include <QTimer>
#include <iostream>

%{APPNAME}::%{APPNAME}()
{
    QTimer* timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), SLOT(output()) );
    timer->start( 1000 );
}

%{APPNAME}::~%{APPNAME}()
{}

void %{APPNAME}::output()
{
    std::cout << "Hello World!" << std::endl;
}

#include "%{APPNAME}.moc"

#include "%{APPNAME}.h"

#include <QTimer>
#include <iostream>

%{APPNAMEID}::%{APPNAMEID}()
{
    QTimer* timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), SLOT(output()) );
    timer->start( 1000 );
}

%{APPNAMEID}::~%{APPNAMEID}()
{}

void %{APPNAMEID}::output()
{
    std::cout << "Hello World!" << std::endl;
}

#include "%{APPNAME}.moc"

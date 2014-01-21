#include "%{APPNAME}.h"

#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QAction>

%{APPNAMEID}::%{APPNAMEID}()
{
    QLabel* l = new QLabel( this );
    l->setText( "Hello World!" );
    setCentralWidget( l );
    QAction* a = new QAction(this);
    a->setText( "Quit" );
    connect(a, SIGNAL(triggered()), SLOT(close()) );
    menuBar()->addMenu( "File" )->addAction( a );
}

%{APPNAMEID}::~%{APPNAMEID}()
{}

#include "%{APPNAME}.moc"

#include "%{APPNAME}.h"

#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>

%{APPNAMEID}::%{APPNAMEID}()
{
    QLabel* label = new QLabel( this );
    label->setText( "Hello World!" );
    setCentralWidget( label );
    QAction* action = new QAction(this);
    action->setText( "Quit" );
    connect(action, SIGNAL(triggered()), SLOT(close()) );
    menuBar()->addMenu( "File" )->addAction( action );
}

%{APPNAMEID}::~%{APPNAMEID}()
{}

#include "%{APPNAME}.moc"

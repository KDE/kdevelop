#include "qt4app.h"

#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>

qt4app::qt4app()
{
    QLabel* l = new QLabel( this );
    l->setText( "Hello World!" );
    setCentralWidget( l );
    QAction* a = new QAction(this);
    a->setText( "Quit" );
    connect(a, SIGNAL(triggered()), SLOT(close()) );
    menuBar()->addMenu( "File" )->addAction( a );
}

qt4app::~qt4app()
{}

#include "qt4app.moc"

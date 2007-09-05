
#include "%{APPNAMELC}.h"
#include <qpushbutton.h>

%{APPNAME}Base::%{APPNAME}Base( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    setupUi( this );
}

%{APPNAME}Base::~%{APPNAME}Base()
{
}

/* 
 *  Constructs a %{APPNAME} which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
%{APPNAME}::%{APPNAME}( QWidget *parent, Qt::WFlags f )
    : %{APPNAME}Base( parent, f )
{
    connect(quit, SIGNAL(clicked()), this, SLOT(goodBye()));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
%{APPNAME}::~%{APPNAME}()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  A simple slot... not very interesting.
 */
void %{APPNAME}::goodBye()
{
    close();
}


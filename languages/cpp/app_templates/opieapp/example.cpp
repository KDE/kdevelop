#include "%{APPNAMELC}.h"
#include <qpushbutton.h>
#include <opie/oapplicationfactory.h>

typedef OApplicationFactory<%{APPNAME}> %{APPNAME}Factory;
OPIE_EXPORT_APP( %{APPNAME}Factory )

/*
 *  Constructs a %{APPNAME} which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
%{APPNAME}::%{APPNAME}( QWidget* parent,  const char* name, Qt::WFlags fl )
    : %{APPNAME}Base( parent, name, fl )
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

%{CPP_TEMPLATE}

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>


#include <kdevelop/kdevcore.h>


#include "%{APPNAMELC}_part.h"
#include "%{APPNAMELC}_widget.h"


%{APPNAME}Widget::%{APPNAME}Widget(%{APPNAME}Part *part)
 : QWidget(0, "%{APPNAMELC} widget")
{
}


%{APPNAME}Widget::~%{APPNAME}Widget()
{
}


#include "%{APPNAMELC}_widget.moc"

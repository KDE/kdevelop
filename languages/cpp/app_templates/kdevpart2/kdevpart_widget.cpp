%{CPP_TEMPLATE}
#include "%{APPNAMELC}widget.h"

#include <kurl.h>
#include <klibloader.h>
#include <kparts/part.h>

#include <kdevcore.h>

#include "%{APPNAMELC}part.h"

%{APPNAME}Widget::%{APPNAME}Widget(%{APPNAME}Part *part)
    : QWidget(0, "%{APPNAMELC} widget"), m_part(part)
{
}

%{APPNAME}Widget::~%{APPNAME}Widget()
{
}

#include "%{APPNAMELC}widget.moc"

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>


#include <kdevcore.h>


#include "sourcebrowser_part.h"
#include "sourcebrowser_widget.h"


SourceBrowserWidget::SourceBrowserWidget(SourceBrowserPart *part)
 : QWidget(0, "sourcebrowser widget")
{
}


SourceBrowserWidget::~SourceBrowserWidget()
{
}


#include "sourcebrowser_widget.moc"

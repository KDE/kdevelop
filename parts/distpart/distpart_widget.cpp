#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>


#include <kdevcore.h>


#include "distpart_part.h"
#include "distpart_widget.h"


distpartWidget::distpartWidget(distpartPart *part)
 : distpart_ui(0, "distpart widget")
{
}


distpartWidget::~distpartWidget()
{
}


#include "distpart_widget.moc"

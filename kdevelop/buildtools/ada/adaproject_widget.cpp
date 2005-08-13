#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>

#include <kdevcore.h>

#include "adaproject_part.h"
#include "adaproject_widget.h"


AdaProjectWidget::AdaProjectWidget(AdaProjectPart *part)
 : QWidget(0, "AdaProject widget")
{
    Q_UNUSED( part );
}


AdaProjectWidget::~AdaProjectWidget()
{
}


#include "adaproject_widget.moc"

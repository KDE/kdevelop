#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>


#include <kdevcore.h>


#include "pascalproject_part.h"
#include "pascalproject_widget.h"


PascalProjectWidget::PascalProjectWidget(PascalProjectPart *part)
 : QWidget(0, "PascalProject widget")
{
    Q_UNUSED( part );
}


PascalProjectWidget::~PascalProjectWidget()
{
}


#include "pascalproject_widget.moc"

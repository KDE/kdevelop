//
// C++ Implementation:
//
// Description:
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kcolorcombo.h>

#include "vcscolorsconfigwidget.h"
#include "fileviewpart.h"

///////////////////////////////////////////////////////////////////////////////
// struct VCSColors
///////////////////////////////////////////////////////////////////////////////

VCSColors::VCSColors()
{
}

///////////////////////////////////////////////////////////////////////////////

VCSColors::VCSColors( const VCSColors &others )
{
    *this = others;
}

///////////////////////////////////////////////////////////////////////////////

VCSColors &VCSColors::operator=( const VCSColors &others )
{
    added = others.added;
    conflict = others.conflict;
    modified = others.modified;
    sticky = others.sticky;
    unknown = others.unknown;
    updated = others.updated;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
// class VCSColorsConfigWidget
///////////////////////////////////////////////////////////////////////////////

VCSColorsConfigWidget::VCSColorsConfigWidget(FileViewPart *part, const VCSColors &vcsColors,
    QWidget* parent, const char* name, WFlags fl)
    : VCSColorsConfigWidgetBase( parent, name, fl ), m_part( part )
{
    setColors( vcsColors );
}

///////////////////////////////////////////////////////////////////////////////

VCSColorsConfigWidget::~VCSColorsConfigWidget()
{
}

///////////////////////////////////////////////////////////////////////////////

void VCSColorsConfigWidget::getColors( VCSColors &vcsColors ) const
{
    vcsColors.added = fileAddedColorCombo->color();
    vcsColors.conflict = fileConflictColorCombo->color();
    vcsColors.modified = fileModifiedColorCombo->color();
    vcsColors.sticky = fileStickyColorCombo->color();
    vcsColors.unknown = fileUnknownColorCombo->color();
    vcsColors.updated = fileUpdatedColorCombo->color();
}

///////////////////////////////////////////////////////////////////////////////

void VCSColorsConfigWidget::setColors( const VCSColors &vcsColors )
{
    fileAddedColorCombo->setColor( vcsColors.added );
    fileConflictColorCombo->setColor( vcsColors.conflict );
    fileModifiedColorCombo->setColor( vcsColors.modified );
    fileStickyColorCombo->setColor( vcsColors.sticky );
    fileUnknownColorCombo->setColor( vcsColors.unknown );
    fileUpdatedColorCombo->setColor( vcsColors.updated );
}

/*$SPECIALIZATION$*/

///////////////////////////////////////////////////////////////////////////////

void VCSColorsConfigWidget::slotAccept()
{
    VCSColors selectedColors;
    getColors( selectedColors );

    m_part->vcsColors = selectedColors;
}

#include "vcscolorsconfigwidget.moc"


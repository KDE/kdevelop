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


#include "setupqtpage.h"
#include "cppsupportpart.h"

SetupQtPage::SetupQtPage(CppSupportPart* part, QWidget* parent, const char* name, WFlags fl)
    : SetupQtPageBase(parent,name,fl), m_part( part )
{
}

SetupQtPage::~SetupQtPage()
{
}

/*$SPECIALIZATION$*/

#include "setupqtpage.moc"


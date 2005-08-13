//
//
// C++ Implementation: cpp
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "kdevfilterIface.h"
#include "filterpart.h"

KDevFilterIface::KDevFilterIface( FilterPart* part )
    : QObject( part ), DCOPObject( "KDevFilter" ), m_part( part )
{
}

KDevFilterIface::~KDevFilterIface()
{
}

void KDevFilterIface::shellInsert()
{
    m_part->slotShellInsert();
}

void KDevFilterIface::shellFilter()
{
    m_part->slotShellFilter();
}

#include "kdevfilterIface.moc"

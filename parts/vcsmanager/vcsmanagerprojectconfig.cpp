/***************************************************************************
 *   Copyright (C) 2005 by Jens Dagerbo   *
 *   jens.dagerbo@swipnet.se   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <qcombobox.h>
#include <qstring.h>

#include <ktrader.h>
#include <klocale.h>

#include "domutil.h"

#include "vcsmanagerprojectconfig.h"
#include "vcsmanagerpart.h"

VCSManagerProjectConfig::VCSManagerProjectConfig( VCSManagerPart *part, QWidget *parent, const char *name )
	: VCSManagerProjectConfigBase( parent, name ), m_part( part )
{
	setup();
}

void VCSManagerProjectConfig::accept()
{
	QString vcsPluginName = *m_vcsPluginNames.at( vcsCombo->currentItem() );
	
	QDomDocument & dom = *m_part->projectDom();
	DomUtil::writeEntry( dom, "/general/versioncontrol", vcsPluginName );
	
	m_part->loadVCSPlugin();
}

void VCSManagerProjectConfig::setup( )
{
	vcsCombo->insertItem( i18n("No Version Control System", "None"), 0 );
	m_vcsPluginNames << "";
	
	int current = 0;

	QString constraint = QString("[X-KDevelop-Version] == %1").arg(KDEVELOP_PLUGIN_VERSION);
	KTrader::OfferList offers = KTrader::self()->query("KDevelop/VersionControl", constraint );
	KTrader::OfferList::const_iterator it = offers.begin();
	for ( int i = 1; it != offers.end(); ++it, ++i )
	{
		vcsCombo->insertItem( (*it)->genericName(), i );
		m_vcsPluginNames << (*it)->desktopEntryName();
		if ( (*it)->desktopEntryName() == m_part->vcsPlugin() )
		{
			current = i;
		}
	}
	vcsCombo->setCurrentItem( current );
}

#include "vcsmanagerprojectconfig.moc"

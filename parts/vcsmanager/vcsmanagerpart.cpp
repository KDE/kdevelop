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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#include "vcsmanagerpart.h"

#include <qtimer.h>
#include <q3popupmenu.h>
#include <q3whatsthis.h>

#include <klocale.h>
#include <kaction.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdevplugininfo.h>
#include <kdevgenericfactory.h>
#include <kdebug.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <configwidgetproxy.h>
#include <kdevplugincontroller.h>
#include "domutil.h"

#include "vcsmanagerprojectconfig.h"

typedef KDevGenericFactory<VCSManagerPart> VCSManagerFactory;
KDevPluginInfo data("kdevvcsmanager");
K_EXPORT_COMPONENT_FACTORY( libkdevvcsmanager, VCSManagerFactory( data ) )

#define GLOBALDOC_OPTIONS 1
#define PROJECTDOC_OPTIONS 2

VCSManagerPart::VCSManagerPart(QObject *parent, const char *name, const QStringList &/*args*/)
	: KDevPlugin(&data, parent, name ? name : "VCSManagerPart")
{
	kdDebug() << " ********** VCSManagerPart::VCSManagerPart() ************** " << endl;

	setInstance(VCSManagerFactory::instance());
//	setXMLFile("kdevvcsmanager.rc");

	m_configProxy = new ConfigWidgetProxy(core());
	m_configProxy->createProjectConfigPage(i18n("Version Control"), PROJECTDOC_OPTIONS, info()->icon());
	connect(m_configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )),
		this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int)));

	connect(core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()));
	connect(core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()));

//	QTimer::singleShot(0, this, SLOT(init()));
}

VCSManagerPart::~VCSManagerPart()
{
	delete m_configProxy;
}

void VCSManagerPart::init()
{
// delayed initialization stuff goes here
}

void VCSManagerPart::insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo)
{
	switch (pageNo)
	{
		case PROJECTDOC_OPTIONS:
		{
			VCSManagerProjectConfig *w = new VCSManagerProjectConfig(this, page, "project config");
			connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
			break;
		}
	}
}

void VCSManagerPart::projectOpened()
{
	loadVCSPlugin();
}

void VCSManagerPart::projectClosed()
{
	unloadVCSPlugin();
	m_vcsPluginName = "";
}

void VCSManagerPart::loadVCSPlugin()
{
	unloadVCSPlugin();

	QDomDocument & dom = *projectDom();
	m_vcsPluginName = DomUtil::readEntry( dom, "/general/versioncontrol" ).stripWhiteSpace();
	if ( m_vcsPluginName.isEmpty() ) return;
	
	QString constraint = QString( "DesktopEntryName=='%1'" ).arg( m_vcsPluginName );
	pluginController()->loadPlugin( "KDevelop/VersionControl", constraint );
}

void VCSManagerPart::unloadVCSPlugin()
{
	if ( !m_vcsPluginName.isEmpty() )
	{
		pluginController()->unloadPlugin( m_vcsPluginName );
	}
}

QString VCSManagerPart::vcsPlugin()
{
	return m_vcsPluginName;
}

#include "vcsmanagerpart.moc"

/***************************************************************************
 *   Copyright (C) 2005 by ian reinhart geiser                             *
 *   ian@geiseri.com                                                       *
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

#include "scriptingpart.h"

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

#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <configwidgetproxy.h>
#include <kapplication.h>
#include <kconfig.h>
#include "scriptingglobalconfig.h"
#include "kscriptactionmanager.h"

typedef KDevGenericFactory<scriptingPart> scriptingFactory;
KDevPluginInfo data("kdevscripting");
K_EXPORT_COMPONENT_FACTORY( libkdevscripting, scriptingFactory( data ) )

#define GLOBALDOC_OPTIONS 1
#define PROJECTDOC_OPTIONS 2

scriptingPart::scriptingPart(QObject *parent, const char *name, const QStringList &/*args*/)
    : KDevPlugin(&data, parent, name ? name : "scriptingPart"), m_scripts(0L)
{
    setInstance(scriptingFactory::instance());
    setXMLFile("kdevscripting.rc");

    m_configProxy = new ConfigWidgetProxy(core());
    m_configProxy->createGlobalConfigPage(i18n("Scripting"), GLOBALDOC_OPTIONS, info()->icon());
    connect(m_configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )),
        this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int)));

  
        
    QTimer::singleShot(0, this, SLOT(init()));
}

scriptingPart::~scriptingPart()
{
    delete m_configProxy;
    delete m_scripts;
}

void scriptingPart::init()
{
// delayed initialization stuff goes here
    m_scripts = new KScriptActionManager(this,actionCollection());
    setupActions();
}

void scriptingPart::setupActions()
{
    kdDebug() << "Load plugins" << endl;
    // Read KConfig and get the list of custom directories.
    QStringList searchDirs;
    searchDirs += "kate/scripts";
    
    KConfig *cfg = kapp->config();
    searchDirs += cfg->readListEntry("SearchDirs");
    
    unplugActionList(QString::fromLatin1( "script_actions" ));
    plugActionList( QString::fromLatin1( "script_actions" ), m_scripts->scripts(core(),searchDirs));
}

void scriptingPart::insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo)
{
// create configuraton dialogs here
    switch (pageNo)
    {
        case GLOBALDOC_OPTIONS:
        {
            scriptingGlobalConfig *w = new scriptingGlobalConfig(this, page, "global config");
            connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
            break;
        }
    }
}


#include "scriptingpart.moc"

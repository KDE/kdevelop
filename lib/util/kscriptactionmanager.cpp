/***************************************************************************
*   Copyright (C) 2004 by ian geiser                                      *
*   geiseri@sourcextreme.com                                              *
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
#include "kscriptactionmanager.h"
#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <kapplication.h>
#include <kdesktopfile.h>
#include <kstandarddirs.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <scriptinterface.h>
#include <kaction.h>
#include <qfileinfo.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3PtrList>

KScriptAction::KScriptAction( const QString &scriptDesktopFile, QObject *interface, KActionCollection *ac )
    : QObject(interface), KScriptClientInterface( )
{
    m_interface = 0L;
    m_action = 0L;
    m_isValid = false;
    m_refs = 0;
    // Read the desktop file
    if(KDesktopFile::isDesktopFile(scriptDesktopFile))
    {
        KDesktopFile desktop(scriptDesktopFile, true);
        QFileInfo scriptPath(scriptDesktopFile);
        
        m_scriptFile = scriptPath.dirPath(true) + "/" + desktop.readEntry("X-KDE-ScriptName", "");
        m_scriptName = desktop.readName();
        m_scriptType = desktop.readType();
        QString scriptTypeQuery = "([X-KDE-Script-Runner] == '" + m_scriptType + "')";
	KTrader::OfferList offers = KTrader::self()->query( "KScriptRunner/KScriptRunner", scriptTypeQuery );
        if ( !offers.isEmpty() )
        {
            m_action = new KAction(m_scriptName, KShortcut(), this, SLOT(activate()), ac, "script");
            m_isValid = true;
            m_timeout = new QTimer(this);
            QString icon = desktop.readIcon();
            m_action->setStatusText(desktop.readComment());
                if( !icon.isEmpty() ) 
                    m_action->setIcon(icon);
            m_action->setShortcutConfigurable(true);
            connect( m_timeout, SIGNAL(timeout()), SLOT(cleanup()) );
        }
    }
}

KScriptAction::~KScriptAction()
{
    if( m_interface ) delete m_interface;
    if( m_action ) delete m_action;
}



KAction * KScriptAction::action( )
{
    return m_action;
}

void KScriptAction::activate( )
{
    if( m_interface == 0L)
    {
        QString scriptTypeQuery = "([X-KDE-Script-Runner] == '" + m_scriptType + "')";
        m_interface= KParts::ComponentFactory::createInstanceFromQuery<KScriptInterface>( "KScriptRunner/KScriptRunner", scriptTypeQuery, this );
	if ( m_interface != 0L)
        {
            m_interface->ScriptClientInterface= this;
            if( m_scriptMethod.isEmpty() )
                m_interface->setScript(m_scriptFile);
            else
                m_interface->setScript(m_scriptFile, m_scriptMethod);
            connect(this, SIGNAL(done(KScriptClientInterface::Result, const QVariant &)), this, SLOT(scriptFinished()));
        }
        else
        {
            KMessageBox::sorry(0, i18n("Unable to get KScript Runner for type \"%1\".").arg(m_scriptType), i18n("KScript Error"));
	    kdDebug() << "Query string: " << scriptTypeQuery << endl;
            return;
        }
    }
    m_interface->run(parent(), QVariant());
    m_timeout->start(60000,FALSE); // after 1 minute unload
    m_refs++;
}

void KScriptAction::cleanup() 
{
    if( m_interface && m_refs == 0)
    {
	delete m_interface;
	m_interface = 0L;
    }
}

void KScriptAction::scriptFinished()
{
    m_refs--;
}

KScriptActionManager::KScriptActionManager(  QObject *parent, KActionCollection * ac ) : QObject(parent), m_ac(ac)
{
    m_actions.setAutoDelete(true);
}

KScriptActionManager::~ KScriptActionManager( )
{
    m_actions.clear();
}

Q3PtrList< KAction > KScriptActionManager::scripts( QObject * interface , const QStringList &dirs) const
{
    m_actions.clear();
    Q3PtrList<KAction> actions;
    QStringList scripts;

    scripts += KGlobal::dirs()->findAllResources("data",
        QString(kapp->name())+"/scripts/*.desktop", false, true );

    for( QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it)
    {
        scripts += KGlobal::dirs()->findAllResources("data",
        (*it)+"/*.desktop", false, true );
    }

    for (QStringList::Iterator it = scripts.begin(); it != scripts.end(); ++it )
    {
        kdDebug() << "Loading " << *it << endl;
        KScriptAction *script = new KScriptAction(*it, interface, m_ac);
	if( script->isValid())
	{
	  actions.append(script->action());
	  m_actions.append(script);
	  connect(script, SIGNAL(error( const QString&)), this, 
	  	SIGNAL(scriptError( const QString&)));
	  connect(script, SIGNAL(warning( const QString&)), this, 
	  	SIGNAL(scriptWarning( const QString&)));
	  connect(script, SIGNAL(output( const QString&)), this,
	  	SIGNAL(scriptOutput( const QString&)));
	  connect(script, SIGNAL(progress( int )), this, 
	  	SIGNAL(scriptProgress(int)));
	  connect(script, SIGNAL(done( KScriptClientInterface::Result, const QVariant &)),this,
	  	SIGNAL(scriptDone( KScriptClientInterface::Result, const QVariant &)));
	}
	else
	  delete script;
    }
    return actions;
}

bool KScriptAction::isValid( ) const
{
  return m_isValid;
}

#include "kscriptactionmanager.moc"

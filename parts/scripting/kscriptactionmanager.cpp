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
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
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

KScriptAction::KScriptAction( const QString &scriptDesktopFile, QObject *parent, KActionCollection *ac )
    : QObject(parent), KScriptClientInterface( )
{
    m_interface = 0L;
    // Read the desktop file
    if(KDesktopFile::isDesktopFile(scriptDesktopFile))
    {
        KDesktopFile desktop(scriptDesktopFile, true);
        QString localpath = QString(kapp->name()) + "/scripts/" + desktop.readEntry("X-KDE-ScriptName", "");
        m_scriptName = desktop.readName();
        m_scriptType = desktop.readType();
        m_scriptFile = locate("data", localpath);
        m_action = new KAction(m_scriptName, KShortcut(), this, SLOT(activate()), ac, "script");
    }
}


KScriptAction::~KScriptAction()
{
    if( m_interface ) delete m_interface;
}



KAction * KScriptAction::action( )
{
    return m_action;
}

void KScriptAction::done( KScriptClientInterface::Result result, const QVariant & returned )
{
    emit scriptDone(result,returned);
}

void KScriptAction::progress( int percent )
{
    emit scriptProgress(percent);
}

void KScriptAction::output( const QString & msg )
{
    emit scriptOutput(msg);
}

void KScriptAction::warning( const QString & msg )
{
    emit scriptWarning(msg);
}

void KScriptAction::error( const QString & msg )
{
    emit scriptError(msg);
}

void KScriptAction::activate( )
{
    if( m_interface == 0L)
    {
        QString scriptTypeQuery = "([X-KDE-Script-Runner] == '" + m_scriptType + "')";
        m_interface= KParts::ComponentFactory::createInstanceFromQuery<KScriptInterface>( "KScriptRunner/KScriptRunner", scriptTypeQuery, this );
        if ( m_interface )
        {
            m_interface->ScriptClientInterface= this;
            if( m_scriptMethod.isEmpty() )
                m_interface->setScript(m_scriptFile);
            else
                m_interface->setScript(m_scriptFile, m_scriptMethod);
        }
        else
        {
            KMessageBox::sorry(0, i18n("Unable to get KScript Runner for type \"%1\".").arg(m_scriptType), i18n("KScript Error"));
            return;
        }
    }
    m_interface->run(parent(), QVariant());
}

KScriptActionManager::KScriptActionManager( KActionCollection * ac ) : m_ac(ac)
{
    m_actions.setAutoDelete(true);
}

KScriptActionManager::~ KScriptActionManager( )
{
    m_actions.clear();
}

QPtrList< KAction > KScriptActionManager::scripts( QObject * interface , const QStringList &dirs) const
{
    m_actions.clear();
    QPtrList<KAction> actions;
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
        actions.append(script->action());
        m_actions.append(script);
    }
    return actions;
}

#include "kscriptactionmanager.moc"

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
#include <klibloader.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <scriptinterface.h>
#include <kaction.h>
#include <qfileinfo.h>

static QString _scriptRunner = "KScriptRunner/KScriptRunner";
KScriptAction::KScriptAction( const QString &scriptDesktopFile, QObject *parent, KActionCollection *ac )
  : QObject(parent, scriptDesktopFile.latin1()), KScriptClientInterface( )
{
    m_interface = 0L;
    m_action = 0L;
    m_isValid = false;
    // Read the desktop file
    if(KDesktopFile::isDesktopFile(scriptDesktopFile))
    {
        KDesktopFile desktop(scriptDesktopFile, true);
        QFileInfo scriptPath(scriptDesktopFile);
        
        m_scriptFile = scriptPath.dirPath(true) + "/" + desktop.readEntry("X-KDE-ScriptName", "");
        m_scriptName = desktop.readName();
        m_scriptType = desktop.readType();
        QString scriptTypeQuery = "([X-KDE-Script-Runner] == '" + m_scriptType + "')";
        KTrader::OfferList offers = KTrader::self()->query( _scriptRunner, scriptTypeQuery );
        if ( !offers.isEmpty() )
        {
                m_action = new KAction(m_scriptName, KShortcut(), this, SLOT(activate()), ac, "script");
                m_isValid = true;
        }
    }
}

KScriptAction::~KScriptAction()
{
  kdDebug() << "Script cleaning up" << endl;
  
    if( m_interface ) delete m_interface;
    if( m_action ) delete m_action;
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
  kdDebug() << "Percent: " << percent << endl;
    emit scriptProgress(percent);
}

void KScriptAction::output( const QString & msg )
{
  kdDebug() << "Output: " << msg << endl;
    emit scriptOutput(msg);
}

void KScriptAction::warning( const QString & msg )
{
  kdDebug() << "Warning: " << msg << endl;
    emit scriptWarning(msg);
}

void KScriptAction::error( const QString & msg )
{
  kdDebug() << "Error: " << msg << endl;
    emit scriptError(msg);
}

void KScriptAction::activate( )
{
  kdDebug() << "Run script" << endl;
    if( m_interface == 0L)
    {
        QString scriptTypeQuery = "([X-KDE-Script-Runner] == '" + m_scriptType + "')";
        int err = 0;
        KTrader::OfferList offers = KTrader::self()->query( _scriptRunner, scriptTypeQuery );
        if ( offers.isEmpty() )
          kdDebug() << "Not found" << endl;
        QStringList args;
        m_interface= KParts::ComponentFactory::createInstanceFromQuery<KScriptInterface>( _scriptRunner, scriptTypeQuery, this, "kscript", args, &err );
        if ( m_interface != 0L)
        {
            m_interface->ScriptClientInterface= this;
            kdDebug() << "Set script file" << m_scriptFile << endl;
            if( m_scriptMethod.isEmpty() )
                m_interface->setScript(m_scriptFile);
            else
                m_interface->setScript(m_scriptFile, m_scriptMethod);
        }
        else
        {
            KMessageBox::sorry(0, i18n("Unable to get KScript Runner for type \"%1\".").arg(m_scriptType), i18n("KScript Error"));
            kdDebug() << "Query string: " << scriptTypeQuery << endl;
            kdDebug() << "Error number: " << err << endl;
            kdDebug() << "Lib loader: " << KLibLoader::self()->lastErrorMessage() << endl;
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
        if( script->isValid())
        {
          actions.append(script->action());
          m_actions.append(script);
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

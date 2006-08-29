/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "kdevenvironment.h"

#include <QProcess>

#include <QStringList>

#include <kdebug.h>
#include <kprocess.h>

#include <stdlib.h>

#include "kdevcore.h"
#include "kdevconfig.h"
#include "kdevprojectcontroller.h"

//Taken from Qt4.2's QProcess
#ifdef Q_OS_MAC
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#elif !defined(Q_OS_WIN)
extern char **environ;
#endif

KDevEnvironment::KDevEnvironment( QObject *parent )
        : QObject( parent )
{
    populate();

    //repopulate when the project is opened
    connect( KDevCore::projectController(), SIGNAL( projectOpened() ),
             this, SLOT( populate() ) );
}

KDevEnvironment::~KDevEnvironment()
{}

void KDevEnvironment::loadSettings()
{
}

void KDevEnvironment::initialize()
{
}

void KDevEnvironment::cleanup()
{
}

QString KDevEnvironment::variable( const QString &name ) const
{
    Q_ASSERT( !name.isEmpty() );
    //Get the override if the user has specified it
    if ( m_overrides.contains( name ) )
        return m_overrides.value( name );

    //Get the process default
    if ( m_processDefaults.contains( name ) )
        return m_processDefaults.value( name );

    return QString::null;
}

void KDevEnvironment::setVariable( const QString &name, const QString &value )
{
    Q_ASSERT( !name.isEmpty() );
    if ( m_processDefaults.value( name ) != value )
        m_overrides.insert( name, value );
    else
        m_overrides.remove( name );

    //http://www.opengroup.org/onlinepubs/009695399/functions/setenv.html
    setenv( name.toLatin1().data(), value.toLatin1().data(), 1 );
}

void KDevEnvironment::unsetVariable( const QString &name )
{
    Q_ASSERT( !name.isEmpty() );
    if ( isOverride( name ) )
        removeOverride( name, true );
    if ( isProcessDefault( name ) )
        removeProcessDefault( name, true );
}

bool KDevEnvironment::revertToProcessDefault( const QString &name )
{
    Q_ASSERT( !name.isEmpty() );
    bool revert = m_processDefaults.contains( name );
    if ( revert )
        setVariable( name, m_processDefaults.value( name ) );
    return revert;
}

void KDevEnvironment::saveSettings()
{
    QStringList pairs;
    EnvironmentMap::const_iterator it = m_overrides.constBegin();
    for ( ; it != m_overrides.constEnd(); ++it )
    {
        pairs.append( it.key() + "=" + it.value() );
    }

    KConfig *local = KDevConfig::localProject();
    local->setGroup( "Environment" );
    if ( !pairs.isEmpty() )
        local->writeEntry( "Variables", pairs );
    else
        local->deleteEntry( "Variables" );

    local->sync();
}

void KDevEnvironment::syncProcess( QProcess *process )
{
    QStringList pairs;
    EnvironmentMap::const_iterator it = m_overrides.constBegin();
    for ( ; it != m_overrides.constEnd(); ++it )
    {
        pairs.append( it.key() + "=" + it.value() );
    }
    EnvironmentMap::const_iterator it2 = m_processDefaults.constBegin();
    for ( ; it2 != m_processDefaults.constEnd(); ++it2 )
    {
        if ( !m_overrides.contains( it2.key() ) )
            pairs.append( it2.key() + "=" + it2.value() );
    }
    process->setEnvironment( pairs );
}

void KDevEnvironment::syncProcess( KProcess *process )
{
    EnvironmentMap::const_iterator it = m_overrides.constBegin();
    for ( ; it != m_overrides.constEnd(); ++it )
    {
        process->setEnvironment( it.key(), it.value( ) );
    }
    EnvironmentMap::const_iterator it2 = m_processDefaults.constBegin();
    for ( ; it2 != m_processDefaults.constEnd(); ++it2 )
    {
        if ( !m_overrides.contains( it2.key() ) )
            process->setEnvironment( it2.key(), it2.value() );
    }
}

void KDevEnvironment::populate()
{
    populateProcessDefaults();
    populateOverrides();
}

void KDevEnvironment::saveSettings( EnvironmentMap overrides )
{
    if ( overrides != m_overrides )
    {
        //Handle deleted entries
        foreach( QString v, m_overrides.keys() )
        {
            if ( !overrides.contains( v ) )
            {
                if ( !m_processDefaults.contains( v ) )
                {
                    removeOverride( v, true );
                }
                else
                {
                    removeOverride( v, false );
                    setVariable( v, m_processDefaults.value( v ) );
                }
            }
        }

        //Handle new or edited entries
        foreach( QString v, overrides.keys() )
        {
            setVariable( v, overrides.value( v ) );
        }

        saveSettings();
    }
}

void KDevEnvironment::populateOverrides()
{
    m_overrides.clear();
    KConfig *config = KDevConfig::standard();
    config->setGroup( "Environment" );
    QStringList pairs = config->readEntry( "Variables", QStringList() );
    foreach ( QString v, pairs )
    {
        QStringList nv = QString( v ).split( '=' );
        if ( nv.count() == 2 )
            setVariable( nv[ 0 ], nv[ 1 ] );
    }
}

void KDevEnvironment::populateProcessDefaults()
{
    m_processDefaults.clear();

    //http://www.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap08.html

    //parse environ
    register char **ep;
    for ( ep = environ; *ep != 0; ++ep )
    {
        QStringList nv = QString( *ep ).split( '=' );
        if ( nv.count() == 2 )
            m_processDefaults.insert( nv[ 0 ], nv[ 1 ] );
    }
}

bool KDevEnvironment::isOverride( const QString &name ) const
{
    return m_overrides.contains( name );
}

QString KDevEnvironment::override( const QString &name ) const
{
    if ( m_overrides.contains( name ) )
        return m_overrides.value( name );
    return QString::null;
}

bool KDevEnvironment::removeOverride( const QString &name, bool unset )
{
    bool c = m_overrides.contains( name );
    if ( c )
    {
        m_overrides.remove( name );

        //http://www.opengroup.org/onlinepubs/009695399/functions/unsetenv.html
        if ( unset )
            unsetenv( name.toLatin1().data() );
    }
    return c;
}

void KDevEnvironment::clearOverrides()
{
    foreach( QString v, m_overrides.keys() )
    {
        //http://www.opengroup.org/onlinepubs/009695399/functions/unsetenv.html
        unsetenv( v.toLatin1().data() );
    }
    m_overrides.clear();
}

bool KDevEnvironment::isProcessDefault( const QString &name ) const
{
    return m_processDefaults.contains( name );
}

QString KDevEnvironment::processDefault( const QString &name ) const
{
    if ( m_processDefaults.contains( name ) )
        return m_processDefaults.value( name );
    return QString::null;
}

bool KDevEnvironment::removeProcessDefault( const QString &name, bool unset )
{
    bool c = m_processDefaults.contains( name );
    if ( c )
    {
        m_processDefaults.remove( name );
        //http://www.opengroup.org/onlinepubs/009695399/functions/unsetenv.html
        if ( unset )
            unsetenv( name.toLatin1().data() );
    }
    return c;
}

void KDevEnvironment::clearProcessDefaults()
{
    foreach( QString v, m_processDefaults.keys() )
    {
        //http://www.opengroup.org/onlinepubs/009695399/functions/unsetenv.html
        unsetenv( v.toLatin1().data() );
    }
    m_processDefaults.clear();
}

#include "kdevenvironment.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

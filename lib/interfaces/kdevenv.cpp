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

#include "kdevenv.h"

#include <QStringList>
#include <kdebug.h>
#include <stdlib.h>

#include "kdevapi.h"

extern char **environ;

KDevEnv::KDevEnv( QObject *parent )
        : QObject( parent )
{
    populateOverrides();
    populateProcessDefaults();

    KDevApi::self() ->setEnvironment( this );
}

KDevEnv::~KDevEnv()
{}

void KDevEnv::populateOverrides()
{
    //Read from config
}

void KDevEnv::populateProcessDefaults()
{
    clearProcessDefaults();
    //http://www.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap08.html

    //parse environ
    register char **ep;
    for ( ep = environ; *ep != 0; ++ep )
    {
        QStringList nv = QString( *ep ).split( '=' );
        m_processDefaults.insert( nv[ 0 ], nv[ 1 ] );
    }
}

QString KDevEnv::variable( const QString &name ) const
{
    //Get the override if the user has specified it
    if ( m_overrides.contains( name ) )
        return m_overrides.value( name );

    //Get the process default
    if ( m_processDefaults.contains( name ) )
        return m_processDefaults.value( name );

    return QString::null;
}

void KDevEnv::setVariable( const QString &name, const QString &value )
{
    m_overrides.insert( name, value );

    //http://www.opengroup.org/onlinepubs/009695399/functions/setenv.html
    setenv( name.toLatin1().data(), value.toLatin1().data(), 1 );
}

void KDevEnv::unsetVariable( const QString &name )
{
    if ( isOverride( name ) )
        removeOverride( name, true );
    if ( isProcessDefault( name ) )
        removeProcessDefault( name, true );
}

bool KDevEnv::isOverride( const QString &name ) const
{
    return m_overrides.contains( name );
}

QString KDevEnv::override( const QString &name ) const
{
    if ( m_overrides.contains( name ) )
        return m_overrides.value( name );
    return QString::null;
}

bool KDevEnv::removeOverride( const QString &name, bool unset )
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

void KDevEnv::clearOverrides()
{
    foreach( QString v, m_overrides.keys() )
    {
        //http://www.opengroup.org/onlinepubs/009695399/functions/unsetenv.html
        unsetenv( v.toLatin1().data() );
    }
    m_overrides.clear();
}

bool KDevEnv::isProcessDefault( const QString &name ) const
{
    return m_processDefaults.contains( name );
}

QString KDevEnv::processDefault( const QString &name ) const
{
    if ( m_processDefaults.contains( name ) )
        return m_processDefaults.value( name );
    return QString::null;
}

bool KDevEnv::removeProcessDefault( const QString &name, bool unset )
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

void KDevEnv::clearProcessDefaults()
{
    foreach( QString v, m_processDefaults.keys() )
    {
        //http://www.opengroup.org/onlinepubs/009695399/functions/unsetenv.html
        unsetenv( v.toLatin1().data() );
    }
    m_processDefaults.clear();
}

#include "kdevenv.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

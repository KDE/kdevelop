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
    //http://www.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap08.html

    //parse environ
    register char **ep;
    for ( ep = environ; *ep != 0; ++ep )
    {
        QStringList nv = QString( *ep ).split( '=' );
        m_variables.insert( nv[ 0 ], nv[ 1 ] );
    }

    KDevApi::self()->setEnvironment( this );
}

KDevEnv::~KDevEnv()
{}

QString KDevEnv::variable( const QString &name ) const
{
    //Get the override if the user has specified it
    if ( m_overrides.contains( name ) )
        return m_overrides.value( name );

    //Get the override if the user has specified it
    if ( m_variables.contains( name ) )
        return m_variables.value( name );
}

void KDevEnv::setVariable( const QString &name, const QString &value )
{
    m_overrides.insert( name, value );

    //http://www.opengroup.org/onlinepubs/009695399/functions/setenv.html
    setenv( name.toLatin1().data(), value.toLatin1().data(), 1 );
}

void KDevEnv::unsetVariable( const QString &name )
{
    //http://www.opengroup.org/onlinepubs/009695399/functions/unsetenv.html
    unsetenv( name.toLatin1().data() );
}

void KDevEnv::clear()
{
    foreach( QString v, m_variables.keys() )
        unsetVariable( v );
}

#include "kdevenv.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

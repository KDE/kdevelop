/*
 * KDevelop config for split header/source
 *
 * Copyright (c) 2005 Adam Treat <treat@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "cppsplitheadersourceconfig.h"
#include "cppsupportpart.h"

#include <domutil.h>

#include <kdebug.h>
#include <qdom.h>

QString CppSplitHeaderSourceConfig::defaultPath = QString::fromLatin1( "/kdevcppsupport/splitheadersource" );

CppSplitHeaderSourceConfig::CppSplitHeaderSourceConfig( CppSupportPart * part, QDomDocument* dom )
        : QObject( part ), m_part( part ), m_dom( dom )
{
    init();
}

CppSplitHeaderSourceConfig::~CppSplitHeaderSourceConfig()
{}

void CppSplitHeaderSourceConfig::init( )
{
    m_splitEnable =
        DomUtil::readBoolEntry( *m_dom, defaultPath + "/enabled", false );
    m_splitSync =
        DomUtil::readBoolEntry( *m_dom, defaultPath + "/synchronize", true );
    m_splitOrientation =
        DomUtil::readEntry( *m_dom, defaultPath + "/orientation", "Vertical" );
}

void CppSplitHeaderSourceConfig::store( )
{
    DomUtil::writeBoolEntry( *m_dom,
                             defaultPath + "/enabled",
                             m_splitEnable );
    DomUtil::writeBoolEntry( *m_dom,
                             defaultPath + "/synchronize",
                             m_splitSync );
    DomUtil::writeEntry( *m_dom,
                         defaultPath + "/orientation",
                         m_splitOrientation );

    emit stored();
}

void CppSplitHeaderSourceConfig::setSplitEnable( bool b )
{
    m_splitEnable = b;
}

void CppSplitHeaderSourceConfig::setAutoSync( bool b )
{
    m_splitSync = b;
}

void CppSplitHeaderSourceConfig::setOrientation( const QString &o )
{
    m_splitOrientation = o;
}

#include "cppsplitheadersourceconfig.moc" 
// kate: indent-mode csands; tab-width 4;

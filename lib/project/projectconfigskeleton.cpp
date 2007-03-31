/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>
Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

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

#include "projectconfigskeleton.h"
#include "iproject.h"

namespace KDevelop
{

ProjectConfigSkeleton::ProjectConfigSkeleton( const QString & configname )
        : KConfigSkeleton( configname )
{
}

ProjectConfigSkeleton::ProjectConfigSkeleton( KSharedConfig::Ptr config )
        : KConfigSkeleton( config )
{
}

void ProjectConfigSkeleton::setProjectConfig( const QString& config )
{
    m_projectDefaultsConfigFile = config;
}

ProjectConfigSkeleton::~ProjectConfigSkeleton()
{}

void ProjectConfigSkeleton::usrSetDefaults()
{
}

}
#include "projectconfigskeleton.moc"

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

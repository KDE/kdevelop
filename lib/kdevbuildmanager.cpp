/*  This file is part of KDevelop
    Copyright (C) 2006 Matt Rogers <mattr@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "kdevbuildmanager.h"

KDevBuildManager::KDevBuildManager(QObject* parent)
: KDevFileManager(parent)
{
}


KDevBuildManager::~KDevBuildManager()
{
}

KUrl::List KDevBuildManager::includeDirectories() const
{
  return KUrl::List();
}

KUrl::List KDevBuildManager::preprocessorDefines() const
{
  return KUrl::List();
}

KUrl::List KDevBuildManager::findMakefiles( KDevProjectFolderItem * ) const
{
  return KUrl::List();
}

KUrl KDevBuildManager::findMakefile( KDevProjectFolderItem * ) const
{
  return KUrl();
}

#include "kdevbuildmanager.moc"
//kate: space-indent on; indent-width 2; indent-mode cstyle; replace-tabs on;

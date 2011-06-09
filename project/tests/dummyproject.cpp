/***************************************************************************
 *   Copyright 2010 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "dummyproject.h"

#include <project/interfaces/ibuildsystemmanager.h>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <project/projectmodel.h>

using KDevelop::IBuildSystemManager;
using KDevelop::ProjectFileItem;
using KDevelop::ProjectFolderItem;
using KDevelop::ProjectBaseItem;
using KDevelop::IndexedString;

DummyProject::DummyProject( const QString& name, QObject* parent )
    : KDevelop::IProject( parent ), m_name(name), m_projectItem(0)
{
}

void DummyProject::addToFileSet(const IndexedString& )
{
}

KDevelop::IBuildSystemManager* DummyProject::buildSystemManager() const
{
    return 0;
}

ProjectFileItem* DummyProject::fileAt(int /*pos*/) const
{
    return 0;
}

int DummyProject::fileCount() const
{
    return 0;
}

const KUrl DummyProject::folder() const
{
    if( m_projectItem ) {
        return m_projectItem->url();
    }
    return KUrl();
}

QList< ProjectFileItem* > DummyProject::files() const
{
    return QList<ProjectFileItem*>();
}

QSet<IndexedString> DummyProject::fileSet() const
{
    return QSet<IndexedString>();
}

QList< ProjectFileItem* > DummyProject::filesForUrl(const KUrl& /*file*/) const
{
    return QList<ProjectFileItem*>();
}

QList< KDevelop::ProjectFolderItem* > DummyProject::foldersForUrl(const KUrl& /*folder*/) const
{
    return QList<ProjectFolderItem*>();
}

QList< ProjectBaseItem* > DummyProject::itemsForUrl(const KUrl& /*url*/) const
{
    return QList<ProjectBaseItem*>();
}

void DummyProject::setProjectItem(KDevelop::ProjectFolderItem* item)
{
    m_projectItem = item;
}

bool DummyProject::inProject(const KUrl& /*url*/) const
{
    return false;
}

bool DummyProject::isReady() const
{
    return true;
}

KDevelop::IPlugin* DummyProject::managerPlugin() const
{
    return 0;
}

QString DummyProject::name() const
{
    return m_name;
}

KSharedConfig::Ptr DummyProject::projectConfiguration() const
{
    return KSharedConfig::openConfig();
}

KDevelop::IProjectFileManager* DummyProject::projectFileManager() const
{
    return 0;
}

KDevelop::ProjectFolderItem* DummyProject::projectItem() const
{
    return m_projectItem;
}

KUrl DummyProject::projectFileUrl() const
{
    return KUrl();
}

KUrl DummyProject::relativeUrl(const KUrl& /*absoluteUrl*/) const
{
    return m_projectItem->url();
}

void DummyProject::reloadModel()
{
}

void DummyProject::removeFromFileSet(const IndexedString& )
{
}

KDevelop::IPlugin* DummyProject::versionControlPlugin() const
{
    return 0;
}


#include "dummyproject.moc"

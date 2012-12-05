/* This file is part of KDevelop
    CopyRight 2010 Milian Wolff <mail@milianw.de>
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>

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
#include "genericmanager.h"

#include <KPluginLoader>
#include <KPluginFactory>
#include <KAboutData>
#include <KConfigGroup>

#include <interfaces/iproject.h>
#include <project/projectmodel.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(GenericSupportFactory, registerPlugin<GenericProjectManager>(); )
K_EXPORT_PLUGIN(GenericSupportFactory(
    KAboutData("kdevgenericmanager", "kdevgenericprojectmanager", ki18n("Generic Project Manager"),
               "0.2", ki18n("A plugin to support basic project management on a filesystem level"), KAboutData::License_GPL)))

GenericProjectManager::GenericProjectManager( QObject* parent, const QVariantList& args )
    : AbstractFileManagerPlugin( GenericSupportFactory::componentData(), parent, args )
{
}

void GenericProjectManager::updateIncludeRules( IProject* project )
{
    KConfigGroup filtersConfig = project->projectConfiguration()->group("Filters");
    QStringList includes = filtersConfig.readEntry("Includes", QStringList("*"));
    QStringList excludes = filtersConfig.readEntry("Excludes", QStringList() << "*/.*" << "*~");

    m_includeRules[project] = qMakePair(includes, excludes);
}

KJob* GenericProjectManager::createImportJob( ProjectFolderItem* item )
{
    updateIncludeRules(item->project());
    return AbstractFileManagerPlugin::createImportJob( item );
}

bool GenericProjectManager::reload( ProjectFolderItem* item )
{
    updateIncludeRules(item->project());
    return AbstractFileManagerPlugin::reload( item );
}

bool GenericProjectManager::isValid( const Path &path, const bool isFolder, IProject* project ) const
{
    if ( isFolder && path.fileName() == ".kdev4" && path.up() == project->path() ) {
        return false;
    } else if ( path == project->projectFile() ) {
        return false;
    }

    bool ok = isFolder;

    // we operate on the relative pathto the project base
    // by prepending a slash we can filter hidden files with the pattern "*/.*"
    // by appending a slash to folders we can filter them with "*/"
    const QString relativePath = '/' + project->path().relativePath(path)
                                + (isFolder ? "/" : "");

    Q_ASSERT( m_includeRules.contains( project ) );
    const IncludeRules& rules = m_includeRules.value( project );

    QStringList::ConstIterator it;
    for ( it = rules.first.constBegin(); !ok && it != rules.first.constEnd(); ++it ) {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( relativePath ) ) {
            ok = true;
            break;
        }
    }

    if ( !ok ) {
        return false;
    }

    for ( it = rules.second.constBegin(); it != rules.second.constEnd(); ++it ) {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( relativePath ) ) {
            return false;
        }
    }

    return true;
}

#include "genericmanager.moc"

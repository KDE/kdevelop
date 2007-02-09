/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

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
#include "genericimporter.h"
#include <projectmodel.h>

#include <icore.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <domutil.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QExtensionFactory>
#include <QRegExp>

typedef KGenericFactory<GenericImporter> GenericSupportFactory;
K_EXPORT_COMPONENT_FACTORY( kdevgenericimporter, GenericSupportFactory( "kdevgenericimporter" ) )

KDEV_USE_EXTENSION_INTERFACE_NS( KDevelop, IFileManager, GenericImporter )

class GenericImporterPrivate
{
    public:
        KDevelop::IProject *m_project;

        QStringList includes;
        QStringList excludes;
};

GenericImporter::GenericImporter( QObject *parent, const QStringList & args )
        : KDevelop::IPlugin( GenericSupportFactory::componentData(), parent ), KDevelop::IFileManager(), d( new GenericImporterPrivate )
{
    Q_UNUSED( args )
    if ( d->includes.isEmpty() )
        d->includes << "*.h" << "*.cpp" << "*.c" << "*.ui" << "*.cs" << "*.java";   // ### remove me

    d->excludes << ".svn" << "CVS" << "moc_*.cpp"; // ### remove me
}

GenericImporter::~GenericImporter()
{}

bool GenericImporter::isValid( const QFileInfo &fileInfo ) const
{
    QString fileName = fileInfo.fileName();

    bool ok = fileInfo.isDir();
    for ( QStringList::ConstIterator it = d->includes.begin(); !ok && it != d->includes.end(); ++it )
    {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( fileName ) )
        {
            ok = true;
        }
    }

    if ( !ok )
        return false;

    for ( QStringList::ConstIterator it = d->excludes.begin(); it != d->excludes.end(); ++it )
    {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( fileName ) )
        {
            return false;
        }
    }

    return true;
}

QList<KDevelop::ProjectFolderItem*> GenericImporter::parse( KDevelop::ProjectFolderItem *item )
{
    QDir dir( item->url().toLocalFile() );

    QList<KDevelop::ProjectFolderItem*> folder_list;
    QFileInfoList entries = dir.entryInfoList();

    for ( int i = 0; i < entries.count(); ++i )
    {
        QFileInfo fileInfo = entries.at( i );

        if ( !isValid( fileInfo ) )
        {
            //kDebug(9000) << "skip:" << fileInfo.absoluteFilePath() << endl;
        }
        else if ( fileInfo.isDir() && fileInfo.fileName() != QLatin1String( "." )
                  && fileInfo.fileName() != QLatin1String( ".." ) )
        {
            KDevelop::ProjectFolderItem *folder = new KDevelop::ProjectFolderItem( item->project(), KUrl( fileInfo.absoluteFilePath() ), item );
            folder_list.append( folder );
        }
        else if ( fileInfo.isFile() )
        {
             new KDevelop::ProjectFileItem( item->project(), KUrl( fileInfo.absoluteFilePath() ), item );
        }
    }

    return folder_list;
}

KDevelop::ProjectItem *GenericImporter::import( KDevelop::IProject *project )
{
    return new KDevelop::ProjectItem( project, project->name(), 0 );;
}

KDevelop::ProjectFolderItem* GenericImporter::addFolder( const KUrl& url,
        KDevelop::ProjectFolderItem * folder )
{
    Q_UNUSED( url )
    Q_UNUSED( folder )
    return 0;
}


KDevelop::ProjectFileItem* GenericImporter::addFile( const KUrl& url,
        KDevelop::ProjectFolderItem * folder )
{
    Q_UNUSED( url )
    Q_UNUSED( folder )
    return 0;
}

bool GenericImporter::renameFolder( KDevelop::ProjectFolderItem * folder, const KUrl& url )
{
    Q_UNUSED( folder )
    Q_UNUSED( url )
    return false;
}

bool GenericImporter::renameFile( KDevelop::ProjectFileItem * file, const KUrl& url )
{
    Q_UNUSED(file)
    Q_UNUSED(url)
    return false;
}

bool GenericImporter::removeFolder( KDevelop::ProjectFolderItem * folder )
{
    Q_UNUSED( folder )
    return false;
}

bool GenericImporter::removeFile( KDevelop::ProjectFileItem * file )
{
    Q_UNUSED( file )
    return false;
}

QStringList GenericImporter::extensions() const
{
    return QStringList() << "IFileManager";
}


#include "genericimporter.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

/* This file is part of KDevelop
    Copyright (C) 2004,2005 Roberto Raggi <roberto@kdevelop.org>

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
#ifndef KDEVGENERICIMPORTER_H
#define KDEVGENERICIMPORTER_H

#include "ifilemanager.h"
#include "iplugin.h"

class QFileInfo;
class QStringList;
class KUrl;
template <typename T> class QList;
namespace Koncrete
{
class ProjectBaseItem;
class ProjectFolderItem;
class ProjectFileItem;
}

class KDialogBase;

class GenericImporter: public Koncrete::IPlugin, public Koncrete::IFileManager
{
    Q_OBJECT
    Q_INTERFACES( Koncrete::IFileManager )
public:
    GenericImporter( QObject *parent = 0,
                     const QStringList &args = QStringList() );
    virtual ~GenericImporter();

//
// IFileManager interface
//
    virtual Features features() const
    {
        return Features( Folders | Files );
    }

    virtual Koncrete::ProjectFolderItem* addFolder( const KUrl& folder, Koncrete::ProjectFolderItem *parent );
    virtual Koncrete::ProjectFileItem* addFile( const KUrl& file, Koncrete::ProjectFolderItem *parent );
    virtual bool removeFolder( Koncrete::ProjectFolderItem *folder );
    virtual bool removeFile( Koncrete::ProjectFileItem *file );
    virtual bool renameFolder( Koncrete::ProjectFolderItem *folder, const KUrl& url );
    virtual bool renameFile( Koncrete::ProjectFileItem *file, const KUrl& url );

    virtual QList<Koncrete::ProjectFolderItem*> parse( Koncrete::ProjectFolderItem *item );
    virtual Koncrete::ProjectItem *import( Koncrete::IProject *project );

    void registerExtensions();
    void unregisterExtensions();
    QStringList extensions() const;

Q_SIGNALS:
    void projectItemConfigWidget(const QList<Koncrete::ProjectBaseItem*> &dom, KDialogBase *dialog);

    void folderAdded( Koncrete::ProjectFolderItem* folder );
    void folderRemoved( Koncrete::ProjectFolderItem* folder );
    void folderRenamed( const KUrl& oldFolder,
                        Koncrete::ProjectFolderItem* newFolder );

    void fileAdded(Koncrete::ProjectFileItem* file);
    void fileRemoved(Koncrete::ProjectFileItem* file);
    void fileRenamed(const KUrl& oldFile,
                     Koncrete::ProjectFileItem* newFile);

private:
    bool isValid( const QFileInfo &fileName ) const;

private:
    struct GenericImporterPrivate* const d;
};

#endif // KDEVGENERICIMPORTER_H
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

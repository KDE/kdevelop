/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>
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

#ifndef KDEVPROJECTMODEL_H
#define KDEVPROJECTMODEL_H

#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include "projectexport.h"
#include "kurl.h"

template<typename T1,typename T2> class QPair;
template<typename T> class QList;
class QDir;
class QFileInfo;

namespace KDevelop
{

class IProject;
class BuildProject;
class ProjectFolderItem;
class ProjectBuildFolderItem;
class ProjectFileItem;
class ProjectItem;
class ProjectTargetItem;

/**
 * Interface that allows a developer to implement the three basic types of
 * items you would see in a multi-project
 * \li Folder
 * \li Project
 * \li Build Target
 * \li File
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectBaseItem: public QStandardItem
{
    public:
        ProjectBaseItem( IProject*, const QString &name, QStandardItem *parent = 0 );
        virtual ~ProjectBaseItem();

        /**
         * add the item @p item to the list of children for this item
         * do not use this function if you gave the item a parent when you
         * created it
         */
        void add( ProjectBaseItem* item );

        enum ProjectItemType
        {
            Folder = QStandardItem::UserType,
            File,
            Project,
            Target,
            BuildFolder
        };

        // Convenience function to return the current project
        IProject* project() const;

        virtual ProjectFolderItem *folder() const;
        virtual ProjectTargetItem *target() const;
        virtual ProjectFileItem *file() const;
        virtual ProjectItem* projectItem() const;

        void setParent( QStandardItem* );
        virtual void setIcon();

        QList<ProjectFolderItem*> folderList() const;
        QList<ProjectTargetItem*> targetList() const;
        QList<ProjectFileItem*> fileList() const;
    protected:
        class ProjectBaseItemPrivate* const d_ptr;
        ProjectBaseItem( ProjectBaseItemPrivate& dd );
    private:
        Q_DECLARE_PRIVATE(ProjectBaseItem)
};

/**
 * Implementation of the ProjectItem interface that is specific to a
 * folder
 */
class ProjectFolderItemPrivate;
class KDEVPLATFORMPROJECT_EXPORT ProjectFolderItem: public ProjectBaseItem
{
public:
    ProjectFolderItem( IProject*, const KUrl &dir, QStandardItem *parent = 0 );

    virtual ~ProjectFolderItem();

    virtual ProjectFolderItem *folder() const;

    ///Reimplemented from QStandardItem
    virtual int type() const;

    /** Get the url of this folder */
    const KUrl& url() const;

    /** Set the url of this folder */
    void setUrl( const KUrl& );

    virtual void setIcon();

protected:
    ProjectFolderItem( ProjectFolderItemPrivate& );
private:
    Q_DECLARE_PRIVATE(ProjectFolderItem)
};


/**
 * Folder which contains buildable targets as part of a buildable project
 */
class ProjectBuildFolderItemPrivate;
class KDEVPLATFORMPROJECT_EXPORT ProjectBuildFolderItem: public ProjectFolderItem
{
public:
    ProjectBuildFolderItem( IProject*, const KUrl &dir, QStandardItem *parent = 0 );

    ///Reimplemented from QStandardItem
    virtual int type() const;

    void setIncludeDirectories( const KUrl::List& includeList );

    /**
     * Return a list of directories that are used as include directories
     * for all targets in this directory.
     */
    const KUrl::List& includeDirectories() const;

    /**
     * Returns an association of environment variables which have been defined
     * for all targets in this directory.
     */
    const QHash<QString, QString>& environment() const;

    virtual void setIcon();

protected:
    ProjectBuildFolderItem( ProjectBuildFolderItemPrivate& );
private:
    Q_DECLARE_PRIVATE(ProjectBuildFolderItem)
};

/**
 * Special folder, the project root folder
 */
class ProjectItemPrivate;
class KDEVPLATFORMPROJECT_EXPORT ProjectItem: public ProjectBuildFolderItem
{
    public:
        ProjectItem( IProject*, const QString &name, QStandardItem *parent = 0 );
        virtual ~ProjectItem();

        int type() const;

        ProjectItem* projectItem() const;
    protected:
        ProjectItem( ProjectItemPrivate& );
    private:
        Q_DECLARE_PRIVATE(ProjectItem)
};



/**
 * Object which represents a target in a build system.
 *
 * This object contains all properties specific to a target.
 */
class ProjectTargetItemPrivate;
class KDEVPLATFORMPROJECT_EXPORT ProjectTargetItem: public ProjectBaseItem
{
public:
    ProjectTargetItem( IProject*, const QString &name, QStandardItem *parent = 0 );

    ///Reimplemented from QStandardItem
    virtual int type() const;

    virtual ProjectTargetItem *target() const;

    /**
     * Return a list of directories that are used as additional include directories
     * specific to this target.
     */
    virtual const KUrl::List& includeDirectories() const = 0;

    /**
     * Returns an association of additional environment variables which have been defined
     * specifically for this target.
     */
    virtual const QHash<QString, QString>& environment() const = 0;

    /**
     * Returns a list of defines passed to the compiler with -D(macro) (value)
     */
    virtual const QList<QPair<QString, QString> >& defines() const = 0;
protected:
    ProjectTargetItem( ProjectTargetItemPrivate& );
private:
    Q_DECLARE_PRIVATE(ProjectTargetItem)
};

/**
 * Object which represents a file.
 */
class ProjectFileItemPrivate;
class KDEVPLATFORMPROJECT_EXPORT ProjectFileItem: public ProjectBaseItem
{
public:
    ProjectFileItem( IProject*, const KUrl& file, QStandardItem *parent = 0 );

    ///Reimplemented from QStandardItem
    virtual int type() const;

    virtual ProjectFileItem *file() const;

    /** Get the url of this file. */
    const KUrl& url() const;
    void setUrl( const KUrl& );

    virtual void setIcon();

protected:
    ProjectFileItem( ProjectFileItemPrivate& );
private:
    Q_DECLARE_PRIVATE(ProjectFileItem)
};

class KDEVPLATFORMPROJECT_EXPORT ProjectModel: public QStandardItemModel
{
        Q_OBJECT
    public:
        ProjectModel( QObject *parent = 0 );
        virtual ~ProjectModel();

        using QStandardItemModel::item;
        ProjectBaseItem *item( const QModelIndex &index ) const;

        void resetModel();

        virtual void fetchMore( const QModelIndex &parent );
        virtual bool canFetchMore( const QModelIndex & parent ) const;

    private:
        class ProjectModelPrivate* const d;

};

}

#endif // KDEVPROJECTMODEL_H
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

/* This file is part of KDevelop
    Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2007 Aleix Pol <aleixpol@gmail.com>

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
#include <KDE/KUrl>
#include <KDE/KSharedConfig>

template<typename T1,typename T2> struct QPair;
template<typename T> class QList;

namespace KDevelop
{

class IProject;
class ProjectFolderItem;
class ProjectBuildFolderItem;
class ProjectFileItem;
class ProjectTargetItem;
class ProjectExecutableTargetItem;
class ProjectLibraryTargetItem;

class KDEVPLATFORMPROJECT_EXPORT ProjectVisitor
{
public:
    ProjectVisitor();
    virtual ~ProjectVisitor();
    virtual void visit( IProject* );
    virtual void visit( ProjectBuildFolderItem* );
    virtual void visit( ProjectExecutableTargetItem* );
    virtual void visit( ProjectFolderItem* );
    virtual void visit( ProjectFileItem* );
    virtual void visit( ProjectLibraryTargetItem* );
};

/**
 * Interface that allows a developer to implement the three basic types of
 * items you would see in a multi-project
 * \li Folder
 * \li Project
 * \li Custom Target
 * \li Library Target
 * \li Executable Target
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
            BuildFolder = QStandardItem::UserType         /** item is a buildable folder */,
            Folder = QStandardItem::UserType+1            /** item is a folder */,
            ExecutableTarget = QStandardItem::UserType+2  /** item is an executable target */,
            LibraryTarget = QStandardItem::UserType+3     /** item is a library target */,
            Target = QStandardItem::UserType+5            /** item is a target */,
            File = QStandardItem::UserType+6              /** item is a file */,
            CustomProjectItemType = QStandardItem::UserType+100 /** type which should be used as base for custom types */
        };
        
        /** @returns Returns the project that the item belongs to.  */
        IProject* project() const;

        /** @returns If this item is a folder, it returns a pointer to the folder, otherwise returns a 0 pointer. */
        virtual ProjectFolderItem *folder() const;

        /** @returns If this item is a target, it returns a pointer to the target, otherwise returns a 0 pointer. */
        virtual ProjectTargetItem *target() const;

        /** @returns If this item is a file, it returns a pointer to the file, otherwise returns a 0 pointer. */
        virtual ProjectFileItem *file() const;
        
        /** @returns If this item is a file, it returns a pointer to the file, otherwise returns a 0 pointer. */
        virtual ProjectExecutableTargetItem *executable() const;

        /**  @param parent sets the item parent to @p parent */
        void setParent( QStandardItem* parent);

        /** @returns Returns a list of the folders that have this object as the parent. */
        QList<ProjectFolderItem*> folderList() const;

        /** @returns Returns a list of the targets that have this object as the parent. */
        QList<ProjectTargetItem*> targetList() const;

        /** @returns Returns a list of the files that have this object as the parent. */
        QList<ProjectFileItem*> fileList() const;

        /** @returns the url of this item if its a file or folder related object */
        virtual KUrl url() const;

        virtual bool lessThan( const KDevelop::ProjectBaseItem* ) const;

    protected:
        class ProjectBaseItemPrivate* const d_ptr;
        ProjectBaseItem( ProjectBaseItemPrivate& dd );
    private:
        Q_DECLARE_PRIVATE(ProjectBaseItem)
};

/**
 * Implementation of the ProjectBaseItem interface that is specific to a
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
    KUrl url() const;

    /** Get the folder name, equal to url().fileName() but faster (precomputed) */
    const QString& folderName() const;

    /** Set the url of this folder */
    void setUrl( const KUrl& );

    /** Returns whether it is the project root folder */
    bool isProjectRoot() const;

    /** Sets whether it is the project root folder and sets the project name to the item */
    void setProjectRoot(bool isRoot);

    /** @returns Returns whether this folder directly contains the specified file or folder. */
    bool hasFileOrFolder(const QString& name) const;

    /** If @p role is Qt::EditRole, it tells the projectcontroller that the folder name is changing to value.
        Otherwise works like QStandardItem::setData */
    void setData(const QVariant& value, int role = Qt::UserRole + 1);
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

protected:
    ProjectBuildFolderItem( ProjectBuildFolderItemPrivate& );
private:
    Q_DECLARE_PRIVATE(ProjectBuildFolderItem)
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
protected:
    ProjectTargetItem( ProjectTargetItemPrivate& );
private:
    Q_DECLARE_PRIVATE(ProjectTargetItem)
};

/**
 * Object which represents an executable target in a build system.
 *
 * This object contains all properties specific to an executable.
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectExecutableTargetItem: public ProjectTargetItem
{
    public:
        ProjectExecutableTargetItem( IProject*, const QString &name, QStandardItem *parent = 0 );

        virtual ProjectExecutableTargetItem *executable() const;
        virtual int type() const;
        virtual KUrl builtUrl() const=0;
        virtual KUrl installedUrl() const=0;
};


/**
 * Object which represents a library target in a build system.
 *
 * This object contains all properties specific to a library.
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectLibraryTargetItem: public ProjectTargetItem
{
    public:
        ProjectLibraryTargetItem(IProject* project, const QString &name, QStandardItem *parent = 0 );

        virtual int type() const;
};

/**
 * Object which represents a file.
 */
class ProjectFileItemPrivate;
class KDEVPLATFORMPROJECT_EXPORT ProjectFileItem: public ProjectBaseItem
{
public:
    ProjectFileItem( IProject*, const KUrl& file, QStandardItem *parent = 0 );
    ~ProjectFileItem();

    ///Reimplemented from QStandardItem
    virtual int type() const;

    virtual ProjectFileItem *file() const;

    /** Get the url of this file. */
    KUrl url() const;

    /** Get the file name, equal to url().fileName() but faster (precomputed) */
    const QString& fileName() const;

    /** Set the url of this file. */
    void setUrl( const KUrl& );
    
    /** If @p role is Qt::EditRole, it tells the projectcontroller that the folder name is changing to value.
        Otherwise works like QStandardItem::setData */
    virtual void setData(const QVariant& value, int role = Qt::UserRole + 1);

protected:
    ProjectFileItem( ProjectFileItemPrivate& );
private:
    Q_DECLARE_PRIVATE(ProjectFileItem)
};

/**
 * Class providing some convenience methods for accessing the project model
 * @todo: maybe switch to QAbstractItemModel, would make the implementation
 *        for at least the checkbox-behaviour easier
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectModel: public QStandardItemModel
{
    Q_OBJECT
public:
    ProjectModel( QObject *parent = 0 );
    virtual ~ProjectModel();

    using QStandardItemModel::item;
    ProjectBaseItem *item( const QModelIndex &index ) const;

    void resetModel();

    QModelIndex pathToIndex(const QStringList& tofetch) const;
    QStringList pathFromIndex(const QModelIndex& index) const;
private:
    class ProjectModelPrivate* const d;
};

KDEVPLATFORMPROJECT_EXPORT QStringList joinProjectBasePath( const QStringList& partialpath, KDevelop::ProjectBaseItem* item );
KDEVPLATFORMPROJECT_EXPORT QStringList removeProjectBasePath( const QStringList& fullpath, KDevelop::ProjectBaseItem* item );

}

#endif // KDEVPROJECTMODEL_H

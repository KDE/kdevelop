/*
    SPDX-FileCopyrightText: 2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROJECTMODEL_H
#define KDEVPLATFORM_PROJECTMODEL_H

#include <QAbstractItemModel>
#include <QUrl>

#include "projectexport.h"

namespace KDevelop
{

class IProject;
class ProjectFolderItem;
class ProjectBuildFolderItem;
class ProjectFileItem;
class ProjectTargetItem;
class ProjectExecutableTargetItem;
class ProjectLibraryTargetItem;
class ProjectModel;
class IndexedString;
class Path;
class ProjectModelPrivate;

/**
 * Base class to implement the visitor pattern for the project item tree.
 *
 * Feel free to subclass it and add overloads for the methods corresponding
 * to the items you are interested in.
 *
 * Start visiting using one of the visit methods.
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectVisitor
{
public:
    ProjectVisitor();
    virtual ~ProjectVisitor();
    /**
     * Visit the whole project model tree.
     */
    virtual void visit( ProjectModel* );
    /**
     * Visit the tree starting from the project root item.
     */
    virtual void visit( IProject* );
    /**
     * Visit the folder and anything it contains.
     */
    virtual void visit( ProjectFolderItem* );
    /**
     * Visit the file.
     */
    virtual void visit( ProjectFileItem* );
    /**
     * Visit the build folder and anything it contains.
     */
    virtual void visit( ProjectBuildFolderItem* );
    /**
     * Visit the target and all children it may contain.
     */
    virtual void visit( ProjectExecutableTargetItem* );
    /**
     * Visit the target and all children it may contain.
     */
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
class KDEVPLATFORMPROJECT_EXPORT ProjectBaseItem
{
    public:
        ProjectBaseItem( IProject*, const QString &name, ProjectBaseItem *parent = nullptr );
        virtual ~ProjectBaseItem();


        enum ProjectItemType
        {
            BaseItem = 0          /** item is a base item */,
            BuildFolder = 1       /** item is a buildable folder */,
            Folder = 2            /** item is a folder */,
            ExecutableTarget = 3  /** item is an executable target */,
            LibraryTarget = 4     /** item is a library target */,
            Target = 5            /** item is a target */,
            File = 6              /** item is a file */,
            CustomProjectItemType = 100 /** type which should be used as base for custom types */
        };

        enum RenameStatus
        {
            RenameOk = 0,
            ExistingItemSameName = 1,
            ProjectManagerRenameFailed = 2,
            InvalidNewName = 3
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

        /** @returns Returns a list of the folders that have this object as the parent. */
        QList<ProjectFolderItem*> folderList() const;

        /** @returns Returns a list of the targets that have this object as the parent. */
        QList<ProjectTargetItem*> targetList() const;

        /** @returns Returns a list of the files that have this object as the parent. */
        QList<ProjectFileItem*> fileList() const;

        virtual bool lessThan( const KDevelop::ProjectBaseItem* ) const;
        static bool pathLessThan(KDevelop::ProjectBaseItem* item1, KDevelop::ProjectBaseItem* item2);

        /** @returns the @p row item in the list of children of this item or 0 if there is no such child. */
        ProjectBaseItem* child( int row ) const;
        /** @returns the list of children of this item. */
        QList<ProjectBaseItem*> children() const;
        /** @returns a valid QModelIndex for usage with the model API for this item. */
        QModelIndex index() const;
        /** @returns The parent item if this item has one, else it return 0. */
        virtual ProjectBaseItem* parent() const;
        /** @returns the displayed text of this item. */
        QString text() const;
        /** @returns the row in the list of children of this items parent, or -1. */
        int row() const;

        /** @returns the number of children of this item, or 0 if there are none. */
        int rowCount() const;

        /** @returns the model to which this item belongs, or 0 if its not associated to a model. */
        ProjectModel* model() const;

        /**
         * Adds a new child item to this item.
         */
        void appendRow( ProjectBaseItem* item );

        /**
         * Removes and deletes the item at the given @p row if there is one.
         */
        void removeRow( int row );

        /**
         * Removes and deletes the @p count items after the given @p row if there is one.
         */
        void removeRows( int row, int count );

        /**
         * Returns and removes the item at the given @p row if there is one.
         */
        ProjectBaseItem* takeRow( int row );

        /** @returns RTTI info, allows one to know the type of item */
        virtual int type() const;

        /** @returns a string to pass to QIcon::fromTheme() as icon-name suitable to represent this item. */
        virtual QString iconName() const;

        /**
         * Set the path of this item.
         *
         * @note This function never renames the item in the project manager or
         * on the filesystem, it only changes the path and possibly the text nothing else.
         */
        virtual void setPath( const Path& );

        /**
         * @returns the path of this item.
         */
        Path path() const;

        /**
         * @return the items indexed path, which is often required for performant
         * lookups or memory efficient storage.
         */
        IndexedString indexedPath() const;

        /**
         * @returns the basename of this items path (if any)
         *
         * Convenience function, returns the same as @c text() for most items.
         */
        QString baseName() const;

        /**
         * Renames the item to the new name.
         * @returns status information whether the renaming succeeded.
         */
        virtual RenameStatus rename( const QString& newname );

        bool isProjectRoot() const;

         /**
         * Default flags: Qt::ItemIsEnabled | Qt::ItemIsSelectable
         *
         * @returns the flags supported by the item
         */
        virtual Qt::ItemFlags flags();

        /**
         * Sets what flags should be returned by flags() method.
         */
        void setFlags(Qt::ItemFlags flags);

    protected:
        /**
         * Allows to change the displayed text of this item.
         *
         * Most items assume text == baseName so this is *not* public.
         *
         * @param text the new text
         */
        void setText( const QString& text );

        const QScopedPointer<class ProjectBaseItemPrivate> d_ptr;
        void setRow( int row );
        void setModel( ProjectModel* model );
    private:
        Q_DECLARE_PRIVATE(ProjectBaseItem)
        friend class ProjectModel;
};

/**
 * Implementation of the ProjectBaseItem interface that is specific to a
 * folder
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectFolderItem: public ProjectBaseItem
{
public:
    /**
     * Create a new ProjectFolderItem with the given @p path and an optional @p parent
     * in the given @p project.
     */
    ProjectFolderItem( IProject* project, const Path& path, ProjectBaseItem* parent = nullptr );

    /**
     * Create a child ProjectFolderItem of @p parent with @p name.
     *
     * The path is set automatically.
     */
    ProjectFolderItem( const QString& name, ProjectBaseItem *parent );

    ~ProjectFolderItem() override;

    void setPath(const Path& ) override;

    ProjectFolderItem *folder() const override;

    ///Reimplemented from QStandardItem
    int type() const override;

    /**
     * Get the folder name, equal to path().fileName() or text().
     */
    QString folderName() const;

    /** @returns Returns whether this folder directly contains the specified file or folder. */
    bool hasFileOrFolder(const QString& name) const;

    QString iconName() const override;
    RenameStatus rename(const QString& newname) override;

private:
    void propagateRename( const Path& newBase ) const;
};


/**
 * Folder which contains buildable targets as part of a buildable project
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectBuildFolderItem: public ProjectFolderItem
{
public:
    /**
     * Create a new ProjectBuildFolderItem with the given @p path with the optional
     * parent @p parent in the given @p project.
     */
    ProjectBuildFolderItem( IProject* project, const Path &path, ProjectBaseItem* parent = nullptr );
    /**
     * Create a child ProjectBuildFolderItem of @p parent with @p name.
     *
     * The path is set automatically.
     */
    ProjectBuildFolderItem( const QString &name, ProjectBaseItem *parent );

    ///Reimplemented from QStandardItem
    int type() const override;
    QString iconName() const override;
};

/**
 * Object which represents a target in a build system.
 *
 * This object contains all properties specific to a target.
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectTargetItem: public ProjectBaseItem
{
public:
    ProjectTargetItem( IProject*, const QString &name, ProjectBaseItem *parent = nullptr );

    ///Reimplemented from QStandardItem
    int type() const override;

    ProjectTargetItem *target() const override;
    QString iconName() const override;
    void setPath(const Path& path ) override;
};

/**
 * Object which represents an executable target in a build system.
 *
 * This object contains all properties specific to an executable.
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectExecutableTargetItem: public ProjectTargetItem
{
    public:
        ProjectExecutableTargetItem( IProject*, const QString &name, ProjectBaseItem *parent = nullptr );

        ProjectExecutableTargetItem *executable() const override;
        int type() const override;
        virtual QUrl builtUrl() const=0;
        virtual QUrl installedUrl() const=0;
};


/**
 * Object which represents a library target in a build system.
 *
 * This object contains all properties specific to a library.
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectLibraryTargetItem: public ProjectTargetItem
{
    public:
        ProjectLibraryTargetItem(IProject* project, const QString &name, ProjectBaseItem *parent = nullptr );

        int type() const override;
};

/**
 * Object which represents a file.
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectFileItem: public ProjectBaseItem
{
public:
    /**
     * Create a new ProjectFileItem with the given @p path and an optional @p parent
     * in the given @p project.
     */
    ProjectFileItem( IProject* project, const Path& path, ProjectBaseItem* parent = nullptr );

    /**
     * Create a child ProjectFileItem of @p parent with the given @p name.
     *
     * The path is set automatically.
     */
    ProjectFileItem( const QString& name, ProjectBaseItem *parent );
    ~ProjectFileItem() override;

    ///Reimplemented from QStandardItem
    int type() const override;

    ProjectFileItem *file() const override;

    /**
     * @returns the file name, equal to path().fileName() or text()
     */
    QString fileName() const;

    void setPath( const Path& ) override;
    QString iconName() const override;
    RenameStatus rename(const QString& newname) override;
};

/**
 * Class providing some convenience methods for accessing the project model
 * @todo: maybe switch to QAbstractItemModel, would make the implementation
 *        for at least the checkbox-behaviour easier
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles {
          ProjectRole = Qt::UserRole+1
        , ProjectItemRole
        , UrlRole
        , LastRole
    };

    explicit ProjectModel( QObject *parent = nullptr );
    ~ProjectModel() override;

    void clear();

    void appendRow( ProjectBaseItem* item );
    void removeRow( int row );
    ProjectBaseItem* takeRow( int row );

    ProjectBaseItem* itemAt( int row ) const;
    QList<ProjectBaseItem*> topItems() const;

    QModelIndex pathToIndex(const QStringList& tofetch) const;
    QStringList pathFromIndex(const QModelIndex& index) const;

    QModelIndex indexFromItem( const ProjectBaseItem* item ) const;
    ProjectBaseItem* itemFromIndex( const QModelIndex& ) const;

    int columnCount( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    QModelIndex parent( const QModelIndex& child ) const override;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    bool insertColumns(int column, int count, const QModelIndex& parent = QModelIndex()) override;
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    Qt::DropActions supportedDropActions() const override;

    /**
     * @return all items for the given indexed path.
     */
    QList<ProjectBaseItem*> itemsForPath(const IndexedString& path) const;

    /**
     * Returns the first item for the given indexed path.
     */
    ProjectBaseItem* itemForPath(const IndexedString& path) const;

private:
    const QScopedPointer<class ProjectModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProjectModel)
    friend class ProjectBaseItem;
};

KDEVPLATFORMPROJECT_EXPORT QStringList joinProjectBasePath( const QStringList& partialpath, KDevelop::ProjectBaseItem* item );
KDEVPLATFORMPROJECT_EXPORT QStringList removeProjectBasePath( const QStringList& fullpath, KDevelop::ProjectBaseItem* item );

}

Q_DECLARE_METATYPE(KDevelop::ProjectBaseItem*)
Q_DECLARE_METATYPE(KDevelop::ProjectFolderItem*)
Q_DECLARE_METATYPE(KDevelop::ProjectFileItem*)
Q_DECLARE_METATYPE(KDevelop::ProjectLibraryTargetItem*)
Q_DECLARE_METATYPE(KDevelop::ProjectExecutableTargetItem*)
Q_DECLARE_METATYPE(KDevelop::ProjectTargetItem*)
Q_DECLARE_METATYPE(KDevelop::ProjectBuildFolderItem*)
Q_DECLARE_METATYPE(QList<KDevelop::ProjectBaseItem*>)

#endif // KDEVPLATFORM_PROJECTMODEL_H

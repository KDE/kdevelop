/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROJECTBUILDSETMODEL_H
#define KDEVPLATFORM_PROJECTBUILDSETMODEL_H

#include "projectexport.h"

#include <QAbstractTableModel>
#include <QStringList>

namespace KDevelop
{
class ProjectBaseItem;
class ICore;
class IProject;
class ISession;
class ProjectBuildSetModelPrivate;

class KDEVPLATFORMPROJECT_EXPORT BuildItem
{
public:
    BuildItem();
    explicit BuildItem( const QStringList& itemPath );
    BuildItem( const BuildItem& rhs );
    explicit BuildItem( KDevelop::ProjectBaseItem* );
    ~BuildItem() = default;

    void initializeFromItem( KDevelop::ProjectBaseItem* item );
    KDevelop::ProjectBaseItem* findItem() const;
    BuildItem& operator=( const BuildItem& );
    QString itemName() const;
    QStringList itemPath() const { return m_itemPath; }
    QString itemProject() const;
private:
    QStringList m_itemPath;
};

bool operator==( const BuildItem& rhs, const BuildItem& lhs );

class KDEVPLATFORMPROJECT_EXPORT ProjectBuildSetModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ProjectBuildSetModel( QObject* parent );
    ~ProjectBuildSetModel() override;
    QVariant data( const QModelIndex&, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int, Qt::Orientation, int role = Qt::DisplayRole ) const override;
    int rowCount( const QModelIndex& = QModelIndex() ) const override;
    int columnCount( const QModelIndex&  = QModelIndex() ) const override;

    void loadFromSession( ISession* session );
    void storeToSession( ISession* session );

    void addProjectItem( KDevelop::ProjectBaseItem* );
    bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;
    void moveRowsUp( int row, int count );
    void moveRowsDown( int row, int count );
    void moveRowsToTop( int row, int count );
    void moveRowsToBottom( int row, int count );
    QList<BuildItem> items() const;
public Q_SLOTS:
    void saveToProject( KDevelop::IProject* ) const;
    void loadFromProject( KDevelop::IProject* );
    void projectClosed( KDevelop::IProject* );
private:
    int findInsertionPlace( const QStringList& itemPath );
    void removeItemsWithCache( const QList<int>& itemIndices );
    void insertItemWithCache( const KDevelop::BuildItem& item );
    void insertItemsOverrideCache( int index, const QList<KDevelop::BuildItem>& items );

private:
    const QScopedPointer<class ProjectBuildSetModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProjectBuildSetModel)
};

}

Q_DECLARE_TYPEINFO(KDevelop::BuildItem, Q_MOVABLE_TYPE);

#endif

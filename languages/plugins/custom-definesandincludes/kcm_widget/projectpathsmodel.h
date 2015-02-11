/************************************************************************
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#ifndef PROJECTPATHSMODEL_H
#define PROJECTPATHSMODEL_H

#include <QAbstractListModel>
#include <QList>

#include <QUrl>

#include "../compilerprovider/settingsmanager.h"

namespace KDevelop
{
class IProject;
}

class ProjectPathsModel : public QAbstractListModel
{
Q_OBJECT
public:
    enum SpecialRoles {
        IncludesDataRole = Qt::UserRole + 1,
        DefinesDataRole = Qt::UserRole + 2,
        FullUrlDataRole = Qt::UserRole + 3,
        CompilerDataRole = Qt::UserRole + 4
    };
    ProjectPathsModel( QObject* parent = 0 );
    void setProject( KDevelop::IProject* w_project );
    void setPaths( const QList< ConfigEntry >& paths );
    void addPath( const QUrl &url );
    QList<ConfigEntry> paths() const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;
private:
    QList<ConfigEntry> projectPaths;
    KDevelop::IProject* project;

    void addPathInternal( const ConfigEntry& config, bool prepend );
    QString sanitizePath( const QString& path, bool expectRelative = true, bool needRelative = true ) const;
    QString sanitizeUrl( QUrl url, bool needRelative = true ) const;
};

#endif // PROJECTPATHSMODEL_H

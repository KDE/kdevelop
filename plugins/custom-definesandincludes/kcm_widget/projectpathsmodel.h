/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef PROJECTPATHSMODEL_H
#define PROJECTPATHSMODEL_H

#include <QAbstractListModel>
#include <QVector>
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
        CompilerDataRole = Qt::UserRole + 4,
        ParserArgumentsRole = Qt::UserRole + 5
    };
    explicit ProjectPathsModel( QObject* parent = nullptr );
    void setProject( KDevelop::IProject* w_project );
    void setPaths( const QVector< ConfigEntry >& paths );
    void addPath( const QUrl &url );
    QVector<ConfigEntry> paths() const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;
private:
    QVector<ConfigEntry> projectPaths;
    KDevelop::IProject* project = nullptr;

    void addPathInternal( const ConfigEntry& config, bool prepend );
    QString sanitizePath( const QString& path, bool expectRelative = true, bool needRelative = true ) const;
    QString sanitizeUrl( const QUrl& url, bool needRelative = true ) const;
};

#endif // PROJECTPATHSMODEL_H

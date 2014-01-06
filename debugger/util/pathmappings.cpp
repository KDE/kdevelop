/*
* This file is part of KDevelop
*
* Copyright 2009 Niko Sams <niko.sams@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "pathmappings.h"

#include <QAbstractTableModel>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QAction>
#include <QFile>
#include <QIcon>

#include <KLocalizedString>
#include <KDebug>
#include <KConfigGroup>

namespace KDevelop {

const QString PathMappings::pathMappingsEntry("Path Mappings");
const QString PathMappings::pathMappingRemoteEntry("Remote");
const QString PathMappings::pathMappingLocalEntry("Local");

KUrl PathMappings::convertToLocalUrl(const KConfigGroup& config, const KUrl& remoteUrl)
{
    if (remoteUrl.isLocalFile() && QFile::exists(remoteUrl.toLocalFile())) {
        return remoteUrl;
    }

    kDebug() << remoteUrl;

    KConfigGroup cfg = config.group(pathMappingsEntry);
    foreach (const QString &group, cfg.groupList()) {
        KConfigGroup pCfg = cfg.group(group);
        KUrl remote = pCfg.readEntry(pathMappingRemoteEntry, KUrl());
        KUrl local = pCfg.readEntry(pathMappingLocalEntry, KUrl());
        kDebug() << remote << local;
        kDebug() << remoteUrl.pathOrUrl() << remote.pathOrUrl();
        if (remoteUrl.pathOrUrl().startsWith(remote.pathOrUrl())) {
            QString path = remoteUrl.pathOrUrl().mid(remote.pathOrUrl().length());
            local.addPath(path);
            return local;
        }
    }

    kDebug() << "no mapping found";
    return remoteUrl;
}

KUrl PathMappings::convertToRemoteUrl(const KConfigGroup& config, const KUrl& localUrl)
{
    kDebug() << localUrl;

    KConfigGroup cfg = config.group(pathMappingsEntry);
    foreach (const QString &group, cfg.groupList()) {
        KConfigGroup pCfg = cfg.group(group);
        KUrl remote = pCfg.readEntry(pathMappingRemoteEntry, KUrl());
        KUrl local = pCfg.readEntry(pathMappingLocalEntry, KUrl());
        kDebug() << remote << local;
        kDebug() << localUrl.pathOrUrl() << local.pathOrUrl();
        if (localUrl.pathOrUrl().startsWith(local.pathOrUrl())) {
            QString path = localUrl.pathOrUrl().mid(local.pathOrUrl().length());
            remote.addPath(path);
            return remote;
        }
    }

    kDebug() << "no mapping found";
    return localUrl;
}


class PathMappingModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const
    {
        if (parent.isValid()) return 0;
        return 2;
    }

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
    {
        if (parent.isValid()) return 0;
        return m_paths.count() + 1;
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == 0) {
                return i18n("Remote Path");
            } else if (section == 1) {
                return i18n("Local Path");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
    {
        if (!index.isValid()) return QVariant();
        if (index.parent().isValid()) return QVariant();
        if (index.column() > 1) return QVariant();
        if (index.row() > m_paths.count()) return QVariant();
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            if (index.row() == m_paths.count()) return QString();
            if (index.column() == 0) {
                return m_paths[index.row()].remote.pathOrUrl();
            } else if (index.column() == 1) {
                return m_paths[index.row()].local.pathOrUrl();
            }
        }
        return QVariant();
    }

    virtual Qt::ItemFlags flags(const QModelIndex& index) const
    {
        if (index.parent().isValid()) return Qt::NoItemFlags;
        if (!index.isValid()) return Qt::NoItemFlags;
        return ( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );

    }

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)
    {
        if (!index.isValid()) return false;
        if (index.parent().isValid()) return false;
        if (index.column() > 1) return false;
        if (index.row() > m_paths.count()) return false;
        if (role == Qt::EditRole) {
            if (index.row() == m_paths.count()) {
                beginInsertRows(QModelIndex(), index.row()+1, index.row()+1);
                m_paths.append(Path());
                endInsertRows();
            }
            if (index.column() == 0) {
                m_paths[index.row()].remote = KUrl(value.toString());
            } else if (index.column() == 1) {
                m_paths[index.row()].local = KUrl(value.toString());
            }
            dataChanged(index, index);
            return true;
        }
        return false;
    }

    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex())
    {
        if (parent.isValid()) return false;
        if (row+count > m_paths.count()) return false;
        beginRemoveRows(parent, row, row+count-1);
        for (int i=0; i<count; ++i) {
            kDebug() << row + i;
            m_paths.removeAt(row + i);
        }
        kDebug() << m_paths.count();
        endRemoveRows();

        return true;
    }

    void loadFromConfiguration(const KConfigGroup &config)
    {
        m_paths.clear();
        KConfigGroup cfg = config.group(PathMappings::pathMappingsEntry);
        for (int i=0; i<cfg.readEntry("Count", 0); ++i) {
            KConfigGroup pCfg = cfg.group(QString::number(i+1));
            Path p;
            p.remote = pCfg.readEntry(PathMappings::pathMappingRemoteEntry, KUrl());
            p.local = pCfg.readEntry(PathMappings::pathMappingLocalEntry, KUrl());
            m_paths << p;
        }
        reset();
    }

    void saveToConfiguration(KConfigGroup config)
    {
        kDebug() << m_paths.count();

        KConfigGroup cfg = config.group(PathMappings::pathMappingsEntry);
        cfg.writeEntry("Count", m_paths.count());
        int i=0;
        foreach (const Path &p, m_paths) {
            i++;
            KConfigGroup pCfg = cfg.group(QString::number(i));
            pCfg.writeEntry(PathMappings::pathMappingRemoteEntry, p.remote);
            pCfg.writeEntry(PathMappings::pathMappingLocalEntry, p.local);
        }
        cfg.sync();
    }

private:
    struct Path {
        KUrl remote;
        KUrl local;
    };
    QList<Path> m_paths;
};


PathMappingsWidget::PathMappingsWidget(QWidget* parent): QWidget(parent)
{
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);

    m_pathMappingTable = new QTableView(this);
    m_pathMappingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pathMappingTable->horizontalHeader()->setDefaultSectionSize(150);
    m_pathMappingTable->horizontalHeader()->setStretchLastSection(true);

    verticalLayout->addWidget(m_pathMappingTable);

    m_pathMappingTable->setModel(new PathMappingModel());
    connect(m_pathMappingTable->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SIGNAL(changed()));
    connect(m_pathMappingTable->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(changed()));
    connect(m_pathMappingTable->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(changed()));

    QAction* deletePath = new QAction(
        QIcon::fromTheme("edit-delete"),
        i18n( "Delete" ),
        this
    );
    connect(deletePath, SIGNAL(triggered(bool)), SLOT(deletePath()));
    deletePath->setShortcut(Qt::Key_Delete);
    deletePath->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_pathMappingTable->addAction(deletePath);

}

void PathMappingsWidget::deletePath()
{
    foreach (const QModelIndex &i, m_pathMappingTable->selectionModel()->selectedRows()) {
        m_pathMappingTable->model()->removeRow(i.row(), i.parent());
    }
}
void PathMappingsWidget::loadFromConfiguration(const KConfigGroup& cfg)
{
    static_cast<PathMappingModel*>(m_pathMappingTable->model())->loadFromConfiguration(cfg);
}

void PathMappingsWidget::saveToConfiguration(KConfigGroup cfg) const
{
    static_cast<PathMappingModel*>(m_pathMappingTable->model())->saveToConfiguration(cfg);
}

}

#include "pathmappings.moc"
#include "moc_pathmappings.cpp"

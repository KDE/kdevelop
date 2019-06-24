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

#include <debug.h>

#include <QAbstractTableModel>
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QAction>
#include <QFile>
#include <QIcon>

#include <KLocalizedString>
#include <KConfigGroup>

namespace {

static QUrl rebaseMatchingUrl(const QUrl& toRebase, const KConfigGroup& config, const QString& baseEntry, const QString& rebaseEntry)
{
    const QUrl::UrlFormattingOption matchOpts = QUrl::NormalizePathSegments;
    const auto configGroups = config.groupList();
    for (const QString& group : configGroups) {
        KConfigGroup pathCfg = config.group(group);
        const QString baseStr = pathCfg.readEntry(baseEntry, QUrl()).url(matchOpts);
        const QString searchStr = toRebase.url(matchOpts);
        if (searchStr.contains(baseStr)) {
            const QUrl rebase = pathCfg.readEntry(rebaseEntry, QUrl());
            return rebase.resolved(QUrl(searchStr.mid(baseStr.length())));
        }
    }
    //No mapping found
    return toRebase;
}

}

namespace KDevelop {

const QString PathMappings::pathMappingsEntry(QStringLiteral("Path Mappings"));
const QString PathMappings::pathMappingRemoteEntry(QStringLiteral("Remote"));
const QString PathMappings::pathMappingLocalEntry(QStringLiteral("Local"));

QUrl PathMappings::convertToLocalUrl(const KConfigGroup& config, const QUrl& remoteUrl)
{
    if (remoteUrl.isLocalFile() && QFile::exists(remoteUrl.toLocalFile())) {
        return remoteUrl;
    }

    KConfigGroup cfg = config.group(pathMappingsEntry);
    return rebaseMatchingUrl(remoteUrl, cfg, pathMappingRemoteEntry, pathMappingLocalEntry);
}

QUrl PathMappings::convertToRemoteUrl(const KConfigGroup& config, const QUrl& localUrl)
{
    KConfigGroup cfg = config.group(pathMappingsEntry);
    return rebaseMatchingUrl(localUrl, cfg, pathMappingLocalEntry, pathMappingRemoteEntry);
}


class PathMappingModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid()) return 0;
        return 2;
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid()) return 0;
        return m_paths.count() + 1;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
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

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid()) return QVariant();
        if (index.parent().isValid()) return QVariant();
        if (index.column() > 1) return QVariant();
        if (index.row() > m_paths.count()) return QVariant();
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            if (index.row() == m_paths.count()) return QString();
            if (index.column() == 0) {
                return m_paths[index.row()].remote.toDisplayString(QUrl::PreferLocalFile);
            } else if (index.column() == 1) {
                return m_paths[index.row()].local.toDisplayString(QUrl::PreferLocalFile);
            }
        }
        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if (index.parent().isValid()) return Qt::NoItemFlags;
        if (!index.isValid()) return Qt::NoItemFlags;
        return ( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );

    }

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
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
                m_paths[index.row()].remote = QUrl::fromUserInput(value.toString());
            } else if (index.column() == 1) {
                m_paths[index.row()].local = QUrl::fromLocalFile(value.toString());
            }
            emit dataChanged(index, index);
            return true;
        }
        return false;
    }

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
    {
        if (parent.isValid()) return false;
        if (row+count > m_paths.count()) return false;
        beginRemoveRows(parent, row, row+count-1);
        for (int i=0; i<count; ++i) {
            qCDebug(DEBUGGER) << row + i;
            m_paths.removeAt(row + i);
        }
        qCDebug(DEBUGGER) << m_paths.count();
        endRemoveRows();

        return true;
    }

    void loadFromConfiguration(const KConfigGroup &config)
    {
        beginResetModel();
        m_paths.clear();
        KConfigGroup cfg = config.group(PathMappings::pathMappingsEntry);
        const auto pathCount = cfg.readEntry("Count", 0);
        m_paths.reserve(pathCount);
        for (int i = 0; i < pathCount; ++i) {
            KConfigGroup pCfg = cfg.group(QString::number(i+1));
            Path p;
            p.remote = pCfg.readEntry(PathMappings::pathMappingRemoteEntry, QUrl());
            p.local = pCfg.readEntry(PathMappings::pathMappingLocalEntry, QUrl());
            m_paths << p;
        }
        endResetModel();
    }

    void saveToConfiguration(KConfigGroup config) const
    {
        qCDebug(DEBUGGER) << m_paths.count();

        KConfigGroup cfg = config.group(PathMappings::pathMappingsEntry);
        cfg.writeEntry("Count", m_paths.count());
        int i=0;
        for (const Path& p : m_paths) {
            i++;
            KConfigGroup pCfg = cfg.group(QString::number(i));
            pCfg.writeEntry(PathMappings::pathMappingRemoteEntry, p.remote);
            pCfg.writeEntry(PathMappings::pathMappingLocalEntry, p.local);
        }
        cfg.sync();
    }

private:
    struct Path {
        QUrl remote;
        QUrl local;
    };
    QVector<Path> m_paths;
};


PathMappingsWidget::PathMappingsWidget(QWidget* parent): QWidget(parent)
{
    auto *verticalLayout = new QVBoxLayout(this);

    m_pathMappingTable = new QTableView(this);
    m_pathMappingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pathMappingTable->horizontalHeader()->setDefaultSectionSize(150);
    m_pathMappingTable->horizontalHeader()->setStretchLastSection(true);

    verticalLayout->addWidget(m_pathMappingTable);

    m_pathMappingTable->setModel(new PathMappingModel());
    connect(m_pathMappingTable->model(), &QAbstractItemModel::dataChanged, this, &PathMappingsWidget::changed);
    connect(m_pathMappingTable->model(), &QAbstractItemModel::rowsRemoved, this, &PathMappingsWidget::changed);
    connect(m_pathMappingTable->model(), &QAbstractItemModel::rowsInserted, this, &PathMappingsWidget::changed);

    QAction* deletePath = new QAction(
        QIcon::fromTheme(QStringLiteral("edit-delete")),
        i18n( "Delete" ),
        this
    );
    connect(deletePath, &QAction::triggered, this, &PathMappingsWidget::deletePath);
    deletePath->setShortcut(Qt::Key_Delete);
    deletePath->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_pathMappingTable->addAction(deletePath);

}

void PathMappingsWidget::deletePath()
{
    const auto selectedRows = m_pathMappingTable->selectionModel()->selectedRows();
    for (const QModelIndex& i : selectedRows) {
        m_pathMappingTable->model()->removeRow(i.row(), i.parent());
    }
}
void PathMappingsWidget::loadFromConfiguration(const KConfigGroup& cfg)
{
    static_cast<PathMappingModel*>(m_pathMappingTable->model())->loadFromConfiguration(cfg);
}

void PathMappingsWidget::saveToConfiguration(const KConfigGroup& cfg) const
{
    static_cast<PathMappingModel*>(m_pathMappingTable->model())->saveToConfiguration(cfg);
}

}

#include "pathmappings.moc"
#include "moc_pathmappings.cpp"

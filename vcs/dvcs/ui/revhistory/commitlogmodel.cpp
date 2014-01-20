/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "commitlogmodel.h"

#include <QtCore/QStringList>
#include <KDE/KDebug>

#include "commitView.h"
#include <dvcs/dvcsplugin.h>

CommitLogModel::CommitLogModel(KDevelop::DistributedVersionControlPlugin* plugin, const QString& repo, QObject* parent)
    : QAbstractItemModel(parent), m_repo(repo), m_plugin(plugin)
{
    headerInfo << "Graph" << "Short Log" << "Author" << "Date";
    QMetaObject::invokeMethod(this, "initializeModel", Qt::QueuedConnection);
}

void CommitLogModel::initializeModel()
{
    revs = m_plugin->getAllCommits(m_repo);
    if (!revs.isEmpty() )
        branchCnt = revs.last().getProperties().count(); //num of branch (size of properties of initial commit)
    
    reset();
}

Qt::ItemFlags CommitLogModel::flags(const QModelIndex&) const 
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable; // read only
}

QVariant CommitLogModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return headerInfo.at(section);

    return QVariant();
}

QModelIndex CommitLogModel::index(int row, int column, const QModelIndex&) const 
{
    if (row < 0 || row >= rowCount())
        return QModelIndex();

    return createIndex(row, column);
}

QVariant CommitLogModel::data(const QModelIndex& index, int role) const 
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const DVcsEvent &revision = revs.at(index.row());

    int column = index.column();

    if (column == SLOG_COLUMN)
        return revision.getLog().split('\n')[0];

    if (column == AUTHOR_COLUMN)
        return revision.getAuthor();

    if (column == DATE_COLUMN) {
        return revision.getDate();
    }
    return QVariant();
}

QModelIndex CommitLogModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int CommitLogModel::rowCount(const QModelIndex& parent) const
{
    return (!parent.isValid() ? revs.size() : 0);
}

int CommitLogModel::columnCount(const QModelIndex&) const 
{
    return 4;
}

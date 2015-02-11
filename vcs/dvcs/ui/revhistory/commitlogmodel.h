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

#ifndef KDEVPLATFORM_COMMITLOGMODEL_H
#define KDEVPLATFORM_COMMITLOGMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <dvcs/dvcsevent.h>

namespace KDevelop
{
    class DistributedVersionControlPlugin;
}

class QStringList;

class CommitLogModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    CommitLogModel(KDevelop::DistributedVersionControlPlugin* plugin, const QString& repo, QObject* parent = 0);
    ~CommitLogModel() {};

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int s, Qt::Orientation o, int role = Qt::DisplayRole) const override;
    QModelIndex index(int r, int c, const QModelIndex& par = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& par = QModelIndex()) const override;
    int columnCount(const QModelIndex&) const override;
    int branchCount(const int) const {return branchCnt;}
    QList<int> getProperties(const int i) const {return revs[i].getProperties();}
    
private slots:
    void initializeModel();

private:
    QStringList headerInfo;
    QList<DVcsEvent> revs;

    int branchCnt;
    
    QString m_repo;
    KDevelop::DistributedVersionControlPlugin* m_plugin;
};

#endif

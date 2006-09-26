/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef FILELISTMODEL_H
#define FILELISTMODEL_H

#include <QAbstractItemModel>

#include <kdirlister.h>

#include "filelistmodel_p.h"

class KUrl;
class KFileItem;

namespace KIO {
class Job;
}

class FileListModel: public QAbstractItemModel {
    Q_OBJECT
public:
    FileListModel(QObject *parent = 0);
    virtual ~FileListModel();

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;

    /**@return the model index for the url.*/
    QModelIndex index(KUrl url);

private slots:
    void slotClear();
    void slotClear(const KUrl &url);
    void slotNewItems(const KFileItemList &items);
    void slotStarted(KUrl &url);
    void slotCompleted();
    void slotCompleted(KUrl &url);

private:
    void populateTree();

    KUrl url(const QModelIndex &index) const;
    FileNode *node(const QModelIndex &index) const;
    KFileItem *fileItem(const QModelIndex &index) const;

    FileTree *m_tree;
    KFileItem *m_rootItem;
    KDirLister m_dirLister;

};

#endif

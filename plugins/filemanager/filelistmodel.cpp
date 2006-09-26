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
#include "filelistmodel.h"

#include <ksharedptr.h>
// #include <kio/jobclasses.h>
#include <kio/netaccess.h>
// #include <kio/job.h>
#include <kurl.h>
#include <kdebug.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>

FileListModel::FileListModel(QObject * parent)
    :QAbstractItemModel(parent), m_tree(0), m_rootItem(0)
{
    connect(&m_dirLister, SIGNAL(clear()),
        this, SLOT(slotClear()));
    connect(&m_dirLister, SIGNAL(clear(const KUrl&)),
        this, SLOT(slotClear(const KUrl&)));
    connect(&m_dirLister, SIGNAL(newItems(const KFileItemList&)),
        this, SLOT(slotNewItems(const KFileItemList&)));
    connect(&m_dirLister, SIGNAL(completed()),
        this, SLOT(slotCompleted()));
    connect(&m_dirLister, SIGNAL(completed(KUrl&)),
        this, SLOT(slotCompleted(KUrl&)));
}

FileListModel::~FileListModel()
{
    delete m_tree;
    delete m_rootItem;
}

Qt::ItemFlags FileListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant FileListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case 0: return url(index).fileName();
//             case 1:
//                 KUrl url = url(index);
        }
    }
    return QVariant();
}

QVariant FileListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QAbstractItemModel::headerData(section, orientation, role);
}

int FileListModel::rowCount(const QModelIndex &parent) const
{
    if (!m_tree)
        return 0;
    FileNode *parentNode;
    if (!parent.isValid())
        parentNode = m_tree->root();
    else
        parentNode = node(parent);
    kDebug() << "node count: " << parentNode->nodes().count() << endl;
    return parentNode->nodes().count();
}

int FileListModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

QModelIndex FileListModel::index(int row, int column, const QModelIndex &parent) const
{
    kDebug() << k_funcinfo << endl;
    if (!m_tree)
        return QModelIndex();

    kDebug() << "requesting index of [" << row << ", " << column << "]" << endl;
    if (column >= 1)
        return QModelIndex();

    FileNode *parentNode = 0;
    if (!parent.isValid())
        parentNode = m_tree->root();
    else
        parentNode = node(parent);

    if (parentNode)
    {
        kDebug() << "   creating index for child" << endl;
        kDebug() << "       parent node contains " << parentNode->nodes().count() << " children" << endl;
        if (row >= parentNode->nodes().count())
            return QModelIndex();
        else
            return createIndex(row, column, parentNode->nodes()[row]);
    }
    else
    {
        kDebug() << "   parent is invalid, returning root" << endl;
        return QModelIndex();
    }
}

QModelIndex FileListModel::parent(const QModelIndex &index) const
{
    kDebug() << k_funcinfo << endl;
    if (!index.isValid())
    {
        kDebug() << "   parent is invalid, returning root" << endl;
        return QModelIndex();
    }
    else
    {
        FileNode *n = node(index);
        FileNode *parentNode = n ? n->parent() : 0;
        if ((!parentNode) || (parentNode == m_tree->root()))
            return QModelIndex();
        else
            return createIndex(parentNode->row(), 0, parentNode);
    }
}

QModelIndex FileListModel::index(KUrl url)
{
    if (!url.isValid())
    {
        kDebug() << "not valid url: " << url << endl;
        return QModelIndex();
    }
    if (m_tree)
        return QModelIndex();

    kDebug() << "creating index for url " << url << endl;

    KIO::UDSEntry entry;
    KIO::NetAccess::stat(url, entry, 0);
    m_rootItem = new KFileItem(entry, url);

    m_tree = new FileTree(m_rootItem);
    populateTree();
//     m_tree->root()->addChild(new FileNode(KUrl::fromPath("/bin")));
    kDebug() << "   nodes created: " << m_tree->root()->nodes().count() << endl;
    return createIndex(0, 0, m_tree->root());
}

KUrl FileListModel::url(const QModelIndex &index) const
{
    if (FileNode *n = node(index))
        return n->data()->url();
    return KUrl();
}

FileNode *FileListModel::node(const QModelIndex &index) const
{
    if (!index.isValid() || !index.internalPointer())
        return 0;
    return static_cast<FileNode*>(index.internalPointer());
}

KFileItem *FileListModel::fileItem(const QModelIndex &index) const
{
    return static_cast<KFileItem*>(index.internalPointer());
}

void FileListModel::populateTree()
{
    if (!m_tree)
        return;

    m_dirLister.openUrl(m_tree->root()->data()->url());
}

void FileListModel::slotClear()
{
    kDebug() << k_funcinfo << endl;
    reset();
}

void FileListModel::slotClear(const KUrl &url)
{
    kDebug() << k_funcinfo << endl;
    reset();
}

void FileListModel::slotNewItems(const KFileItemList &items)
{
    kDebug() << k_funcinfo << endl;
    int first = m_tree->root()->nodes().count();
    int last = first + items.count() - 1;
    kDebug() << "   insert from " << first << " to " << last << endl;
    beginInsertRows(QModelIndex(), first, last);
    foreach (const KFileItem *item, items)
        m_tree->root()->addChild(new FileNode(item));
    endInsertRows();
}

void FileListModel::slotCompleted()
{
    kDebug() << k_funcinfo << endl;
}

void FileListModel::slotCompleted(KUrl &url)
{
    kDebug() << k_funcinfo << endl;
}

void FileListModel::slotStarted(KUrl &url)
{
    kDebug() << k_funcinfo << endl;
//     beginInsertRows();
}

#include "filelistmodel.moc"

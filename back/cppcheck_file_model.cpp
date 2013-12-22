/* This file is part of KDevelop
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
 * Copyright 2011 Lionel Duc <lionel.data@gmail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; see the file COPYING.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*/

#include "cppcheck_file_model.h"

#include <QApplication>

#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobalsettings.h>

#include <cppcheck_file_item.h>

#include "modelwrapper.h"

namespace cppcheck
{

CppcheckFileModel::CppcheckFileModel(QObject * parent)
{
    Q_UNUSED(parent)
    m_rootItem = new CppcheckFileItem();
    m_rootItem->incomingData("", "", 0, "", "", "", "", "");
}

CppcheckFileModel::~ CppcheckFileModel()
{
    delete m_rootItem;
}

void CppcheckFileModel::newItem(ModelItem *i)
{
     kDebug() << "CppcheckFileModel::newItem() start";
    if (!i) {
        kDebug() << "CppcheckFileModel::newItem() invalid item -> model changed";
        emit static_cast<ModelEvents *>(m_modelWrapper)->modelChanged();
        return;
    }
    CppcheckFileItem *m = dynamic_cast<CppcheckFileItem *>(i);
    Q_ASSERT(m);

    kDebug() << "CppcheckFileModel::newItem(): ErrorLine: " << m->ErrorLine<< ", Message: " << m->Message;
    int ErrorLine = m->ErrorLine;
    QString ErrorFile = m->ErrorFile;
    QString Message = m->Message;
    QString MessageVerbose = m->MessageVerbose;
    QString ProjectPath = m->ProjectPath;
    QString Severity = m->Severity;

    bool fileAlreadyInList = false;
    for (int x=0; x < m_rootItem->childCount(); x++) {
        CppcheckFileItem *current = m_rootItem->child(x) ;
        kDebug() << "CppcheckFileModel::newItem() elem at " << x << ", current->ProjectPath: " << current->ProjectPath << ", ProjectPath: " << ProjectPath << ", current->ErrorFile: " << current->ErrorFile << ", ErrorFile: " << ErrorFile;
        if (current->ProjectPath == ProjectPath && current->ErrorFile == ErrorFile) {
           kDebug() << "CppcheckFileModel::newItem() adding at " << x << ", ErrorLine: " << ErrorLine << ", Message: " << Message;
           fileAlreadyInList = true;
           m->setParent(current);
           current->appendChild(m);
           delete m;
           break;
        }
   }
    if (!fileAlreadyInList) {
        kDebug() << "CppcheckFileModel::newItem() adding new , ErrorLine: " << m->ErrorLine << ", Message: " << m->Message;
        CppcheckFileItem *m2 = new CppcheckFileItem();
        m2->ErrorLine = m->ErrorLine;
        m2->ErrorFile = m->ErrorFile;
        m2->Message = m->Message;
        m2->MessageVerbose = m->MessageVerbose;
        m2->Severity = m->Severity;
        m2->setParent(m);
        m->setParent(m_rootItem);
        m->ErrorLine = -1;
        m->Message = "";
        m->MessageVerbose = "";
        m->Severity = "unknown";
        m_rootItem->appendChild(m);
        
        m->appendChild(m2);
    }
}

void CppcheckFileModel::reset()
{
}

QVariant CppcheckFileModel::getColumnAtSnapshot(int snap, CppcheckFileItem::Columns col)
{
    return m_rootItem->child(snap)->data(col);
}

QModelIndex CppcheckFileModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    CppcheckFileItem *parentItem;
    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<CppcheckFileItem*>(parent.internalPointer());
    CppcheckFileItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex CppcheckFileModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();
    CppcheckFileItem *childItem = static_cast<CppcheckFileItem*>(index.internalPointer());
    CppcheckFileItem *parentItem = childItem->parent();
    if (parentItem == m_rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int CppcheckFileModel::rowCount(const QModelIndex &parent) const
{
    CppcheckFileItem *parentItem;
//     if (parent.column() > 0)
//         return 0;
    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<CppcheckFileItem*>(parent.internalPointer());
    if (parentItem == 0)
	return 0;
    return parentItem->childCount();
}

int CppcheckFileModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<CppcheckFileItem*>(parent.internalPointer())->columnCount();
    else
        return m_rootItem->columnCount();
}

QVariant CppcheckFileModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();
    switch (role) {
    case Qt::DisplayRole: {
        CppcheckFileItem *item = static_cast<CppcheckFileItem*>(index.internalPointer());
        return item->data(index.column());
    }
    break;

    case Qt::FontRole: {
        QFont f = KGlobalSettings::generalFont();
        if ((static_cast<CppcheckFileItem*>(index.internalPointer()))->parent() == m_rootItem)
            f.setBold(true);
        return f;
    }
    break;
    }
    return QVariant();
}

QVariant CppcheckFileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
        case CppcheckFileItem::ColumnErrorFile:
            return i18n("File");
            break;
        case CppcheckFileItem::ColumnErrorLine:
            return i18n("Line");
            break;
        case CppcheckFileItem::ColumnMessage:
            return i18n("Message");
            break;
        case CppcheckFileItem::ColumnProjectPath:
            return i18n("Project path");
            break;
        case CppcheckFileItem::ColumnMessageVerbose:
            return i18n("Message detailed");
            break;

            break;
        }
    }
    return QVariant();
}

QAbstractItemModel*  CppcheckFileModel::getQAbstractItemModel(int) {
        return this;
    }


}

#include "cppcheck_file_model.moc"

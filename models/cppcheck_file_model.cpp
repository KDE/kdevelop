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
#include <QFontDatabase>

#include <kmessagebox.h>
#include <klocale.h>

#include <models/cppcheckitemsimpl.h>
#include <cppcheck_file_item.h>
#include <cppcheck_file_model.h>

#include "debug.h"
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
    //qCDebug(KDEV_CPPCHECK) << "start";
    if (!i) {
        qCDebug(KDEV_CPPCHECK) << "invalid item -> model changed";
        emit static_cast<ModelEvents *>(m_modelWrapper)->modelChanged();
        return;
    }
    CppcheckFileItem *m = dynamic_cast<CppcheckFileItem *>(i);
    Q_ASSERT(m);

    //qCDebug(KDEV_CPPCHECK) << "ErrorLine: " << m->ErrorLine<< ", Message: " << m->Message;
    int ErrorLine = m->ErrorLine;
    QString ErrorFile = m->ErrorFile;
    QString Message = m->Message;
    QString MessageVerbose = m->MessageVerbose;
    QString ProjectPath = m->ProjectPath;
    QString Severity = m->Severity;

    bool fileAlreadyInList = false;
    for (int x=0; x < m_rootItem->childCount(); x++) {
        CppcheckFileItem *current = m_rootItem->child(x) ;
        //qCDebug(KDEV_CPPCHECK) << "elem at " << x << ", current->ProjectPath: " << current->ProjectPath << ", ProjectPath: " << ProjectPath << ", current->ErrorFile: " << current->ErrorFile << ", ErrorFile: " << ErrorFile;
        if (current->ProjectPath == ProjectPath && current->ErrorFile == ErrorFile) {
           qCDebug(KDEV_CPPCHECK) << "adding at " << x << ", ErrorLine: " << ErrorLine << ", Message: " << Message;
           fileAlreadyInList = true;
           m->setParent(current);
           m->setIsChild(true);
           current->appendChild(m);
           break;
        }
   }
    if (!fileAlreadyInList) {
        qCDebug(KDEV_CPPCHECK) << "adding new , ErrorLine: " << m->ErrorLine << ", Message: " << m->Message;
        CppcheckFileItem *m2 = new CppcheckFileItem(true);
        m2->ErrorLine = m->ErrorLine;
        m2->ErrorFile = m->ErrorFile;
        m2->Message = m->Message;
        m2->MessageVerbose = m->MessageVerbose;
        m2->Severity = m->Severity;
        m2->ProjectPath = ProjectPath;
        m2->setParent(m);
        m->setParent(m_rootItem);
        m->ErrorLine = -1;
        m->Message = "";
        m->MessageVerbose = "";
        m->Severity = i18n("unknown");
        if(m->ErrorFile == "")
            m->ErrorFile = i18n("Global scope");
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
    CppcheckFileItem *parentItem = 0;
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

CppcheckItem* CppcheckFileModel::parent() const {
        return 0L;
    }

QVariant CppcheckFileModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();
    switch (role) {
        case Qt::DisplayRole: {
            CppcheckFileItem *item = static_cast<CppcheckFileItem*>(index.internalPointer());
            return item->data(index.column(), role);
        }
        case Qt::DecorationRole: {
            CppcheckFileItem *item = static_cast<CppcheckFileItem*>(index.internalPointer());
            return item->data(index.column(), role);
        }
        case Qt::ToolTipRole: {
            CppcheckFileItem *item = static_cast<CppcheckFileItem*>(index.internalPointer());
            return item->data(index.column(), role);
        }
    case Qt::FontRole: {
            QFont f = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
            if ((static_cast<CppcheckFileItem*>(index.internalPointer()))->parent() == m_rootItem)
                f.setBold(true);
            return f;
        }
        break;
    case Qt::UserRole:
        switch (index.column()) {
         case CppcheckFileItem::ColumnSeverity:
             return static_cast<CppcheckFileItem*>(index.internalPointer())->Severity;
             break;
        case CppcheckFileItem::ColumnErrorFile:
            return static_cast<CppcheckFileItem*>(index.internalPointer())->ErrorFile;
            break;
        case CppcheckFileItem::ColumnErrorLine:
            return QString().setNum(static_cast<CppcheckFileItem*>(index.internalPointer())->ErrorLine);
            break;
        case CppcheckFileItem::ColumnMessage:
            return static_cast<CppcheckFileItem*>(index.internalPointer())->Message;
            break;
        case CppcheckFileItem::ColumnProjectPath:
            return static_cast<CppcheckFileItem*>(index.internalPointer())->ProjectPath;
            break;
        case CppcheckFileItem::ColumnMessageVerbose:
            return static_cast<CppcheckFileItem*>(index.internalPointer())->MessageVerbose;
            break;
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
            return i18n("File / Line");
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


void CppcheckFileModel::incomingData(QString, QString, int, QString, QString, QString, QString, QString)
{
}

}

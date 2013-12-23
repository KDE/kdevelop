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

#include "cppcheck_severity_model.h"

#include <QApplication>

#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobalsettings.h>

#include <models/cppcheckitemsimpl.h>
#include <cppcheck_severity_item.h>
#include <cppcheck_severity_model.h>

#include "modelwrapper.h"

namespace cppcheck
{

CppcheckSeverityModel::CppcheckSeverityModel(QObject * parent)
{
    Q_UNUSED(parent)
    m_rootItem = new CppcheckSeverityItem();
    m_rootItem->incomingData("", "", 0, "", "", "", "", "");
}

CppcheckSeverityModel::~ CppcheckSeverityModel()
{
    delete m_rootItem;
}

void CppcheckSeverityModel::newItem(ModelItem *i)
{
    //kDebug() << "start";
    if (!i) {
        kDebug() << "invalid item -> model changed";
        emit static_cast<ModelEvents *>(m_modelWrapper)->modelChanged();
        return;
    }
    CppcheckSeverityItem *m = dynamic_cast<CppcheckSeverityItem *>(i);
    Q_ASSERT(m);

    //kDebug() << "ErrorLine: " << m->ErrorLine<< ", Message: " << m->Message;
    int ErrorLine = m->ErrorLine;
    QString ErrorFile = m->ErrorFile;
    QString Message = m->Message;
    QString MessageVerbose = m->MessageVerbose;
    QString ProjectPath = m->ProjectPath;
    QString Severity = m->Severity;

    bool severityAlreadyInList = false;
    for (int x=0; x < m_rootItem->childCount(); x++) {
        CppcheckSeverityItem *current = m_rootItem->child(x) ;
        //kDebug() << "elem at " << x << ", current->Severity: " << current->Severity << ", current->ErrorFile: " << current->ErrorFile << ", ErrorFile: " << ErrorFile;
        if (current->Severity == Severity) {
           kDebug() << "adding at " << x << ", ErrorLine: " << ErrorLine << ", Message: " << Message;
           severityAlreadyInList = true;
           m->setParent(current);
           m->setIsChild(true);
           current->appendChild(m);
           break;
        }
   }
    if (!severityAlreadyInList) {
        kDebug() << "adding new , ErrorLine: " << m->ErrorLine << ", Message: " << m->Message;
        CppcheckSeverityItem *m2 = new CppcheckSeverityItem(true);
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
        m_rootItem->appendChild(m);
        
        m->appendChild(m2);
    }
}

void CppcheckSeverityModel::reset()
{
}

QVariant CppcheckSeverityModel::getColumnAtSnapshot(int snap, CppcheckSeverityItem::Columns col)
{
    return m_rootItem->child(snap)->data(col);
}

QModelIndex CppcheckSeverityModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    CppcheckSeverityItem *parentItem;
    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<CppcheckSeverityItem*>(parent.internalPointer());
    CppcheckSeverityItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex CppcheckSeverityModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();
    CppcheckSeverityItem *childItem = static_cast<CppcheckSeverityItem*>(index.internalPointer());
    CppcheckSeverityItem *parentItem = childItem->parent();
    if (parentItem == m_rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int CppcheckSeverityModel::rowCount(const QModelIndex &parent) const
{
    CppcheckSeverityItem *parentItem;
//     if (parent.column() > 0)
//         return 0;
    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<CppcheckSeverityItem*>(parent.internalPointer());

    if (parentItem == 0)
        return 0;

    return parentItem->childCount();
}

int CppcheckSeverityModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<CppcheckSeverityItem*>(parent.internalPointer())->columnCount();
    else
        return m_rootItem->columnCount();
}

CppcheckItem* CppcheckSeverityModel::parent() const {
        return 0L;
    }

QVariant CppcheckSeverityModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();
    switch (role) {
        case Qt::DisplayRole: {
            CppcheckSeverityItem *item = static_cast<CppcheckSeverityItem*>(index.internalPointer());
            return item->data(index.column(), role);
        }
        case Qt::DecorationRole: {
            CppcheckSeverityItem *item = static_cast<CppcheckSeverityItem*>(index.internalPointer());
            return item->data(index.column(), role);
        }
        case Qt::ToolTipRole: {
            CppcheckSeverityItem *item = static_cast<CppcheckSeverityItem*>(index.internalPointer());
            return item->data(index.column(), role);
        }
        case Qt::FontRole: {
            QFont f = KGlobalSettings::generalFont();
            if ((static_cast<CppcheckSeverityItem*>(index.internalPointer()))->parent() == m_rootItem)
                f.setBold(true);
            return f;
        }
        break;
    case Qt::UserRole:
        switch (index.column()) {
         case CppcheckSeverityItem::ColumnSeverity:
             return static_cast<CppcheckSeverityItem*>(index.internalPointer())->Severity;
             break;
        case CppcheckSeverityItem::ColumnErrorFile:
            return static_cast<CppcheckSeverityItem*>(index.internalPointer())->ErrorFile;
            break;
        case CppcheckSeverityItem::ColumnErrorLine:
            return QString().setNum(static_cast<CppcheckSeverityItem*>(index.internalPointer())->ErrorLine);
            break;
        case CppcheckSeverityItem::ColumnMessage:
            return static_cast<CppcheckSeverityItem*>(index.internalPointer())->Message;
            break;
        case CppcheckSeverityItem::ColumnProjectPath:
            return static_cast<CppcheckSeverityItem*>(index.internalPointer())->ProjectPath;
            break;
        case CppcheckSeverityItem::ColumnMessageVerbose:
            return static_cast<CppcheckSeverityItem*>(index.internalPointer())->MessageVerbose;
            break;
        }
        break;
    }
    return QVariant();
}

QVariant CppcheckSeverityModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
        case CppcheckSeverityItem::ColumnSeverity:
            return i18n("Severity / File");
            break;
        case CppcheckSeverityItem::ColumnMessage:
            return i18n("Message");
            break;
        case CppcheckSeverityItem::ColumnProjectPath:
            return i18n("Project path");
            break;
        case CppcheckSeverityItem::ColumnMessageVerbose:
            return i18n("Message detailed");
            break;
        }
    }
    return QVariant();
}

QAbstractItemModel*  CppcheckSeverityModel::getQAbstractItemModel(int) {
        return this;
    }

void CppcheckSeverityModel::incomingData(QString, QString, int, QString, QString, QString, QString, QString)
{
}

}

#include "cppcheck_severity_model.moc"

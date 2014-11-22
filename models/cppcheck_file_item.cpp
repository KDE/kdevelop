/* This file is part of KDevelop
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>

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

#include "cppcheck_file_item.h"

#include <QDir>
#include <QFont>

#include <kglobalsettings.h>
#include <kiconloader.h>

namespace cppcheck
{

CppcheckFileItem::CppcheckFileItem()
{
    m_child = false;
}

CppcheckFileItem::CppcheckFileItem(bool child)
{
    m_child = child;
}

CppcheckFileItem::~CppcheckFileItem()
{
    qDeleteAll(m_childItems);
}

void CppcheckFileItem::incomingData(const QString &name, const QString &value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity)
{
    Q_UNUSED(name);
    Q_UNUSED(value);

    //m_values[name] = value;
    this->ProjectPath = ProjectPath;
    this->ErrorFile = ErrorFile;
    this->ErrorLine = ErrorLine;
    this->Message = Message;
    this->MessageVerbose = MessageVerbose;
    this->Severity = Severity;
}

void CppcheckFileItem::appendChild(CppcheckFileItem *item)
{
    m_childItems.append(item);
}

void CppcheckFileItem::setParent(CppcheckFileItem *parent)
{
    m_parentItem = parent;
}

CppcheckFileItem *CppcheckFileItem::child(int row)
{
    return m_childItems.value(row);
}

int CppcheckFileItem::childCount() const
{
    if (!m_childItems.isEmpty())
        return m_childItems.count();
    else
        return 0;
}

int CppcheckFileItem::columnCount() const
{
    //return m_values.size()
    return 2;
}

QVariant CppcheckFileItem::data(int column, int role) const
{
    if (!m_child ) {
        switch (role)
        {
            case Qt::DisplayRole:
                switch (column) {
                    case ColumnErrorFile:
                        return ErrorFile;
                        break;
                }
                break;
                case Qt::ToolTipRole: {
                    switch (column) {
                        case CppcheckFileItem::ColumnErrorFile:
                            return QString("<p>" + ProjectPath + ErrorFile + "</p>");
                            break;
                        case CppcheckFileItem::ColumnMessage:
                            break;
                        case CppcheckFileItem::ColumnProjectPath:
                            break;
                        case CppcheckFileItem::ColumnMessageVerbose:
                            break;
                    }
                }
        }
        return QVariant();
    }
    else {
        switch (role)
        {
            case Qt::DisplayRole:
                switch (column)
                {
                    case ColumnErrorFile:
                            if (ErrorLine == -1)
                                return QString("-");
                            else
                                return QString().setNum(ErrorLine);
                        break;            case ColumnMessage:
                            return Message;
                        break;
                    case ColumnProjectPath:
                            return ProjectPath;
                        break;
                    case ColumnMessageVerbose:
                            return MessageVerbose;
                        break;
                    case ColumnSeverity:
                            return Severity;
                        break;
                }
                break;
            case Qt::DecorationRole: {
                switch (column) {
        //                 case Severity:
        //                     if (CppcheckError* e = dynamic_cast<CppcheckError*>(this)) {
        //                         //return e->Severity;
        //                         if (e->Severity == "error") {
        //                             return QVariant( KIconLoader().loadIcon( "dialog_close", KIconLoader::NoGroup, 16 ));
        //                         }
        //                        else if (e->Severity == "warning") {
        //                             return QVariant( KIconLoader().loadIcon( "task-attention.png", KIconLoader::NoGroup, 16 ));
        //                         }
        //                         else if (e->Severity == "style") {
        //                             return QVariant( KIconLoader().loadIcon( "documentinfo", KIconLoader::NoGroup, 16 ));
        //                         }
        //                         else if (e->Severity == "information") {
        //                             return QVariant( KIconLoader().loadIcon( "documentinfo", KIconLoader::NoGroup, 16 ));
        //                         }
        //                         break;
        //                     }
        //                     break;
                case CppcheckFileItem::ColumnErrorFile:
                    //return Severity;
                    if (Severity == "error") {
                        return QVariant(KIconLoader().loadIcon("dialog_close", KIconLoader::NoGroup, 16));
                    } else if (Severity == "warning") {
                        return QVariant(KIconLoader().loadIcon("task-attention.png", KIconLoader::NoGroup, 16));
                    } else if (Severity == "style") {
                        return QVariant(KIconLoader().loadIcon("documentinfo", KIconLoader::NoGroup, 16));
                    } else if (Severity == "information") {
                        return QVariant(KIconLoader().loadIcon("documentinfo", KIconLoader::NoGroup, 16));
                    }
                    break;
                case CppcheckFileItem::ColumnMessage:
                    break;
                case CppcheckFileItem::ColumnProjectPath:
                    break;
                case CppcheckFileItem::ColumnMessageVerbose:
                    break;
                }
                break;
            }
            case Qt::ToolTipRole: {
                switch (column) {
                    case CppcheckFileItem::ColumnErrorFile:
                        return QString("<p>" + ProjectPath + ErrorFile + "</p>");
                        break;
                    case CppcheckFileItem::ColumnMessage:
                        return QString("<p>" + MessageVerbose + "</p>");
                        break;
                    case CppcheckFileItem::ColumnProjectPath:
                        break;
                    case CppcheckFileItem::ColumnMessageVerbose:
                        break;
                }
            }
            break;
        }
    }
    return QVariant();
}

CppcheckFileItem *CppcheckFileItem::parent() const
{
    return m_parentItem;
}

int CppcheckFileItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<CppcheckFileItem*>(this));

    return 0;
}

QUrl CppcheckFileItem::url() const
{
    if (m_dir.isEmpty() && m_file.isEmpty())
        return QUrl();

    QUrl base = QUrl::fromLocalFile(m_dir);
    base.setPath(base.path() + '/');
    QUrl url = QUrl(base).resolved(QUrl(m_file));
    url = QDir::cleanPath(url.path());
    return url;
}

int CppcheckFileItem::getLine() const
{
    return m_line;
}

void CppcheckFileItem::setIsChild(bool isChild)
{
    m_child = isChild;
}

bool CppcheckFileItem::isChild()
{
    return m_child;
}

}

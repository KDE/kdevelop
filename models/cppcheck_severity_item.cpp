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

#include "cppcheck_severity_item.h"

#include <QDir>
#include <QFont>
#include <QUrl>

#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>

namespace cppcheck
{

CppcheckSeverityItem::CppcheckSeverityItem()
{
    m_child = false;
}

CppcheckSeverityItem::CppcheckSeverityItem(bool child)
{
    m_child = child;
}

CppcheckSeverityItem::~CppcheckSeverityItem()
{
    qDeleteAll(m_childItems);
}

void CppcheckSeverityItem::incomingData(const QString &name, const QString &value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity)
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

void CppcheckSeverityItem::appendChild(CppcheckSeverityItem *item)
{
    m_childItems.append(item);
}

void CppcheckSeverityItem::setParent(CppcheckSeverityItem *parent)
{
    m_parentItem = parent;
}

CppcheckSeverityItem *CppcheckSeverityItem::child(int row)
{
    return m_childItems.value(row);
}

int CppcheckSeverityItem::childCount() const
{
    if (!m_childItems.isEmpty())
        return m_childItems.count();
    else
        return 0;
}

int CppcheckSeverityItem::columnCount() const
{
    //return m_values.size()
    return 2;
}

QVariant CppcheckSeverityItem::data(int column, int role) const
{
    if (!m_child ) {
        switch (role)
        {
            case Qt::DisplayRole:
               switch (column) {
                    case ColumnSeverity:
                   return i18n(Severity.toLocal8Bit());
                        break;
                    return QVariant();
                }
                break;
            case Qt::DecorationRole: {
                switch (column) {
                    case CppcheckSeverityItem::ColumnSeverity:
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
                    case CppcheckSeverityItem::ColumnMessage:
                        break;
                    case CppcheckSeverityItem::ColumnProjectPath:
                        break;
                    case CppcheckSeverityItem::ColumnMessageVerbose:
                        break;
                }
                break;
            }
        }
    }
    else {
        switch (role)
        {
            case Qt::DisplayRole:
                switch (column)
                {
                    case ColumnSeverity:
                    if (ErrorFile != "")
                        return ErrorFile+":"+QString().setNum(ErrorLine);
                    else
                        return ("-");
                        break;
                    case ColumnMessage:
                            return Message;
                        break;
                    case ColumnProjectPath:
                            return ProjectPath;
                        break;
                    case ColumnMessageVerbose:
                            return MessageVerbose;
                        break;
                }
                break;
                case Qt::ToolTipRole: {
                    switch (column) {
                        case CppcheckSeverityItem::ColumnSeverity:
                            if (m_child)
                                return QString("<p>" + ProjectPath + ErrorFile + "</p>");
                            else
                                if (m_child)
                                    return QString("<p>" + Severity + "</p>");
                            break;
                        case CppcheckSeverityItem::ColumnMessage:
                            return QString("<p>" + MessageVerbose + "</p>");
                            break;
                        case CppcheckSeverityItem::ColumnProjectPath:
                            break;
                        case CppcheckSeverityItem::ColumnMessageVerbose:
                            break;
                    }
                 }
                 break;
            break;
        }
    }
    return QVariant();
}

CppcheckSeverityItem *CppcheckSeverityItem::parent() const
{
    return m_parentItem;
}

int CppcheckSeverityItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<CppcheckSeverityItem*>(this));

    return 0;
}

QUrl CppcheckSeverityItem::url() const
{
    if (m_dir.isEmpty() && m_file.isEmpty())
        return QUrl();

    QUrl base = QUrl::fromLocalFile(m_dir);
    base.setPath(base.path() + '/');
    QUrl url = QUrl(base).resolved(QUrl(m_file));
    url = QDir::cleanPath(url.path());
    return url;
}

int CppcheckSeverityItem::getLine() const
{
    return m_line;
}

void CppcheckSeverityItem::setIsChild(bool isChild)
{
    m_child = isChild;
}

bool CppcheckSeverityItem::isChild()
{
    return m_child;
}

}



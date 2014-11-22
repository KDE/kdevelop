/* This file is part of KDeelop
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
 * Copyright 2011 Lionel Duc <lionel.data@gmail.com>
 * Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

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

#include "cppcheckitemsimpl.h"
#include "cppcheckmodel.h"

#include <KIconLoader>
#include <KLocalizedString>

#include <QApplication>
#include <QDir>
#include <QPixmap>

#include <iostream>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include "modelwrapper.h"

namespace cppcheck
{

CppcheckModel::CppcheckModel(QObject* parent)
{
    Q_UNUSED(parent); // do we need it ?
}

CppcheckModel::~ CppcheckModel()
{
    //    qDeleteAll(errors);
}

int CppcheckModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return numColumns;
}

QVariant CppcheckModel::data(const QModelIndex& index, int role) const
{
    CppcheckItem* item = itemForIndex(index);

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
//         case Severity:
//             if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
// //                 return e->Severity;
//                     return QString("");
//                     break;
//             }
//             break;
        case ErrorFile:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return e->ErrorFile;
            }
            break;
        case ErrorLine:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                if (e->ErrorLine == -1)
                    return QString("-");
                else
                    return QString().setNum(e->ErrorLine);
            }
            break;
        case Message:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return e->Message;
            }
            break;
        case ProjectPath:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return e->ProjectPath;
                //return e->ErrorFile;
            }
            break;
        case MessageVerbose:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return e->MessageVerbose;
            }
            break;

        }
        break;
    case Qt::FontRole:
        break;

    case Qt::ToolTipRole:
        switch (index.column()) {
//         case Severity:
//             if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
//                return i18n(e->Severity.toLocal8Bit());
//             }
//             break;
        case ErrorFile:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return e->ProjectPath + e->ErrorFile;
            }
            break;
        case ErrorLine:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return QString().setNum(e->ErrorLine);
            }
            break;
        case Message:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
//                return e->Message;
                return QString("<p>" + e->MessageVerbose + "</p>");
            }
            break;
        case ProjectPath:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return e->ProjectPath;
            }
            break;
        case MessageVerbose:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return e->MessageVerbose;
            }
            break;

        }
        break;

    case Qt::UserRole:
        switch (index.column()) {
//         case Severity:
//             if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
//                return e->Severity;
//             }
//             break;
        case ErrorFile:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return e->ErrorFile;
            }
            break;
        case ErrorLine:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return QString().setNum(e->ErrorLine);
            }
            break;
        case Message:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return e->Message;
            }
            break;
        case ProjectPath:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return e->ProjectPath;
            }
            break;
        case MessageVerbose:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                return e->MessageVerbose;
            }
            break;
        }


        // begin
    case Qt::DecorationRole: {
        switch (index.column()) {
//                 case Severity:
//                     if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
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
        case ErrorFile:
            if (CppcheckError* e = dynamic_cast<CppcheckError*>(item)) {
                //return e->Severity;
                if (e->Severity == "error") {
                    return QVariant(KIconLoader().loadIcon("dialog_close", KIconLoader::NoGroup, 16));
                } else if (e->Severity == "warning") {
                    return QVariant(KIconLoader().loadIcon("task-attention.png", KIconLoader::NoGroup, 16));
                } else if (e->Severity == "style") {
                    return QVariant(KIconLoader().loadIcon("documentinfo", KIconLoader::NoGroup, 16));
                } else if (e->Severity == "information") {
                    return QVariant(KIconLoader().loadIcon("documentinfo", KIconLoader::NoGroup, 16));
                }
                break;
            }
            break;
        case ErrorLine:
            break;
        case Message:
            break;
        case ProjectPath:
            break;
        case MessageVerbose:
            break;
        }
        break;
    }
    // end

    break;
    }


    return QVariant();
}

QVariant CppcheckModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
//         case Severity:
//             return i18n("Severity");
//             break;
        case ErrorFile:
            return i18n("File");
            break;
        case ErrorLine:
            return i18n("Line");
            break;
        case Message:
            return i18n("Message");
            break;
        case ProjectPath:
            return i18n("Project path");
            break;
        case MessageVerbose:
            return i18n("Message detailed");
            break;

            break;
        }
    }
    return QVariant();
}

QModelIndex CppcheckModel::index(int row, int column, const QModelIndex& p) const
{
    if (row < 0 || column < 0 || column >= numColumns)
        return QModelIndex();

    if (p.isValid() && p.column() != 0)
        return QModelIndex();

    CppcheckItem* parent = itemForIndex(p);

    if (!parent) {
        if (row < m_errors.count())
            return createIndex(row, column, m_errors.at(row));

    } else if (CppcheckError* e = dynamic_cast<CppcheckError*>(parent)) {
        int r2 = row;

        foreach (CppcheckStack * stack, e->getStack()) {
            if (row < stack->getFrames().size())
                return createIndex(row, column, stack->getFrames().at(row));
            r2 -= stack->getFrames().count();
        }

    }
    return QModelIndex();
}

QModelIndex CppcheckModel::parent(const QModelIndex& index) const
{
    CppcheckItem* item = itemForIndex(index);
    if (!item)
        return QModelIndex();
    item = item->parent();
    return indexForItem(item, 0);
}

int CppcheckModel::rowCount(const QModelIndex& p) const
{
    if (!p.isValid())
        return m_errors.count();

    if (p.column() != 0)
        return 0;

//     qCDebug(KDEV_CPPCHECK) << "rowCount 1: " << p.model()->rowCount();
//         return p.model()->rowCount();

    CppcheckItem* parent = itemForIndex(p);

    if (CppcheckError* e = dynamic_cast<CppcheckError*>(parent)) {
        int ret = 0;
        foreach (const CppcheckStack * stack, e->getStack())
            ret += stack->getFrames().count();
//         qCDebug(KDEV_CPPCHECK) << "rowCount: " << ret;
        return ret;
    }
    return 0;
}

QModelIndex CppcheckModel::indexForItem(CppcheckItem* item, int column) const
{
    int index = -1;

    if (CppcheckError* e = dynamic_cast<CppcheckError*>(item))
        index = e->parent()->m_errors.indexOf(e);
    if (index != -1)
        return createIndex(index, column, item);
    return QModelIndex();
}

CppcheckItem* CppcheckModel::itemForIndex(const QModelIndex& index) const
{
    if (index.internalPointer())
        return static_cast<CppcheckItem*>(index.internalPointer());
    return 0L;
}

void CppcheckModel::newElement(CppcheckModel::eElementType e)
{
    switch (e) {
    case startError:
        newStartError();
        break;
    default:
        break;
    }
}

void CppcheckModel::newStack()
{
    if (m_errors.back()->getStack().count()) {
        beginInsertRows(indexForItem(m_errors.back()), m_errors.back()->getStack().count(),
                        m_errors.back()->getStack().count());
        endInsertRows();
    }
    m_errors.back()->addStack();
}

void CppcheckModel::newStartError()
{
    m_errors << new CppcheckError(this);
}


void CppcheckModel::newFrame()
{
    //m_errors.back()->lastStack()->addFrame();
}

void CppcheckModel::reset()
{
    m_errors.clear();
    QAbstractItemModel::reset();
}

void CppcheckModel::newData(CppcheckModel::eElementType e, QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity)
{
    switch (e) {
    case error:
        m_errors.back()->incomingData(name, value, ErrorLine, ErrorFile, Message, MessageVerbose, ProjectPath, Severity);
    default:
        break;
    }
    emit static_cast<ModelEvents*>(m_modelWrapper)->modelChanged();
}
void CppcheckModel::incomingData(QString, QString, int, QString, QString, QString, QString, QString)
{
}

void CppcheckModel::newItem(cppcheck::ModelItem*)
{
    // TODO use it instead of the other signals
}

CppcheckItem* CppcheckModel::parent() const {
        return 0L;
    }

QAbstractItemModel*  CppcheckModel::getQAbstractItemModel(int) {
        return this;
    }

}

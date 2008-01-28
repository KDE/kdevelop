/*
 * KDevelop Problem Reporter
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "problemmodel.h"

#include <klocale.h>

#include <hashedstring.h>
#include <duchain.h>
#include <duchainlock.h>

#include "problemreporterplugin.h"

using namespace KDevelop;

ProblemModel::ProblemModel(ProblemReporterPlugin * parent)
  : QAbstractItemModel(parent)
{
}

ProblemModel::~ ProblemModel()
{
  m_problems.clear();
}

int ProblemModel::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return m_problems.count();

    if (parent.internalId() && parent.column() == 0)
        return m_problems.at(parent.row())->locationStack().count();

    return 0;
}

QVariant ProblemModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    DUChainReadLocker lock(DUChain::lock());
  
    ProblemPointer p = problemForIndex(index);

    if (!index.internalId()) {
        // Top level
        switch (role) {
            case Qt::DisplayRole:
                switch (index.column()) {
                    case Source:
                        switch (p->source()) {
                            case Problem::Unknown:
                            default:
                                return i18n("Unknown");
                            case Problem::Disk:
                                return i18n("Disk");
                            case Problem::Preprocessor:
                                return i18n("Preprocessor");
                            case Problem::Lexer:
                                return i18n("Lexer");
                            case Problem::Parser:
                                return i18n("Parser");
                            case Problem::DUChainBuilder:
                                return i18n("Definition-Use Chain");
                        }
                        break;
                    case Error:
                        return p->description();
                    case File:
                        return p->finalLocation().document().str();
                    case Line:
                        return QString::number(p->finalLocation().start().line() + 1);
                    case Column:
                        return QString::number(p->finalLocation().start().column());
                }
                break;

            case Qt::ToolTipRole:
                return p->explanation();

            default:
                break;
        }

    } else {
        switch (role) {
            case Qt::DisplayRole:
                switch (index.column()) {
                    case Error:
                        return i18n("In file included from:");
                    case File:
                        return p->locationStack().at(index.row()).document().str();
                    case Line:
                        return QString::number(p->locationStack().at(index.row()).line() + 1);
                    case Column:
                        return QString::number(p->locationStack().at(index.row()).column());
                }
                break;

            default:
                break;
        }
    }

    return QVariant();
}

QModelIndex ProblemModel::parent(const QModelIndex & index) const
{
    if (index.internalId())
        return createIndex(m_problems.indexOf(problemForIndex(index)), 0, 0);

    return QModelIndex();
}

QModelIndex ProblemModel::index(int row, int column, const QModelIndex & parent) const
{
    DUChainReadLocker lock(DUChain::lock());
  
    if (row < 0 || column < 0 || column >= LastColumn)
        return QModelIndex();

    if (parent.isValid()) {
        if (parent.internalId())
            return QModelIndex();

        if (parent.column() != 0)
            return QModelIndex();

        ProblemPointer problem = problemForIndex(parent);
        if (row >= problem->locationStack().count())
            return QModelIndex();

        return createIndex(row, column, row);
    }

    if (row < m_problems.count())
        return createIndex(row, column, 0);

    return QModelIndex();
}

int ProblemModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return LastColumn;
}

ProblemReporterPlugin * ProblemModel::plugin() const
{
    return static_cast<ProblemReporterPlugin*>(const_cast<QObject*>(sender()));
}

KDevelop::ProblemPointer ProblemModel::problemForIndex(const QModelIndex & index) const
{
    if (index.internalId())
        return m_problems.at(index.internalId());
    else
        return m_problems.at(index.row());
}

void ProblemModel::addProblem(KDevelop::ProblemPointer problem)
{
    beginInsertRows(QModelIndex(), m_problems.count(), m_problems.count());
    m_problems.append(problem);
    endInsertRows();
}

void ProblemModel::setProblems(const QList<KDevelop::ProblemPointer>& problems)
{
  m_problems = problems;
  reset();
}

void ProblemModel::clear()
{
  m_problems.clear();
  reset();
}

QVariant ProblemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
        case Source:
            return i18n("Source");
        case Error:
            return i18n("Problem");
        case File:
            return i18n("File");
        case Line:
            return i18n("Line");
        case Column:
            return i18n("Column");
    }

    return QVariant();
}

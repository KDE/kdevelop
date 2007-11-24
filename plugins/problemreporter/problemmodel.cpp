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

#include "problemreporterpart.h"

using namespace KDevelop;

ProblemModel::ProblemModel(ProblemReporterPart * parent)
  : QAbstractItemModel(parent)
{
}

ProblemModel::~ ProblemModel()
{
    qDeleteAll(m_problems);
}

int ProblemModel::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return m_problems.count();

    if (!parent.internalPointer() && parent.column() == 0)
        return m_problems.at(parent.row())->locationStack().count();

    return 0;
}

QVariant ProblemModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Problem* p = problemForIndex(index);

    if (!index.internalPointer()) {
        // Top level
        switch (role) {
            case Qt::DisplayRole:
                switch (index.column()) {
                    case 0:
                        return p->description();
                    case 1:
                        return p->finalLocation().document().prettyUrl();
                    case 2:
                        return QString::number(p->finalLocation().start().line() + 1);
                    case 3:
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
                    case 0:
                        return i18n("In file included from:");
                    case 1:
                        return p->locationStack().at(index.row()).document().prettyUrl();
                    case 2:
                        return QString::number(p->locationStack().at(index.row()).line() + 1);
                    case 3:
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
    if (index.internalPointer())
        return createIndex(m_problems.indexOf(problemForIndex(index)), 0, 0);

    return QModelIndex();
}

QModelIndex ProblemModel::index(int row, int column, const QModelIndex & parent) const
{
    if (row < 0 || column < 0 || column > 3)
        return QModelIndex();

    if (parent.isValid()) {
        if (parent.internalPointer())
            return QModelIndex();

        if (parent.column() != 0)
            return QModelIndex();

        Problem* problem = problemForIndex(parent);
        if (row >= problem->locationStack().count())
            return QModelIndex();

        return createIndex(row, column, problem);
    }

    if (row < m_problems.count())
        return createIndex(row, column, 0);

    return QModelIndex();
}

int ProblemModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return 4;
}

ProblemReporterPart * ProblemModel::part() const
{
    return static_cast<ProblemReporterPart*>(const_cast<QObject*>(sender()));
}

KDevelop::Problem * ProblemModel::problemForIndex(const QModelIndex & index) const
{
    if (index.internalPointer())
        return static_cast<KDevelop::Problem*>(index.internalPointer());
    else
        return m_problems.at(index.row());
}

void ProblemModel::addProblem(KDevelop::Problem * problem)
{
    beginInsertRows(QModelIndex(), m_problems.count(), m_problems.count());
    m_problems.append(problem);
    endInsertRows();
}

QVariant ProblemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
        case 0:
            return i18n("Problem");
        case 1:
            return i18n("File");
        case 2:
            return i18n("Line");
        case 3:
            return i18n("Column");
    }

    return QVariant();
}

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

#include <language/editor/hashedstring.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

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

QString getDisplayUrl(const QString &url, const KUrl &base) {
    KUrl location(url);
    QString displayedUrl;
    if ( location.protocol() == base.protocol() &&
            location.user() == base.user() &&
            location.host() == base.host() ) {
        bool isParent;
        displayedUrl = KUrl::relativePath(base.path(), location.path(), &isParent );
        if ( !isParent ) {
            displayedUrl = location.pathOrUrl();
        }
    } else {
        displayedUrl = location.pathOrUrl();
    }
    return displayedUrl;
}

QVariant ProblemModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

//     Locking the duchain here leads to a deadlock, because kate triggers some paint to the outside while holding the smart-lock
//     DUChainReadLocker lock(DUChain::lock());

    ProblemPointer p = problemForIndex(index);

    if (!index.internalId()) {
        // Top level
        switch (role) {
            case Qt::DisplayRole:
                switch (index.column()) {
                    case Source:
                        return p->sourceString();
                        break;
                    case Error:
                        return p->description();
                    case File: {
                        return getDisplayUrl(p->finalLocation().document().str(), m_base);
                    }
                    case Line:
                        if (p->finalLocation().isValid())
                            return QString::number(p->finalLocation().start().line() + 1);
                        break;
                    case Column:
                        if (p->finalLocation().isValid())
                            return QString::number(p->finalLocation().start().column() + 1);
                        break;
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
                    case File: {
                        return getDisplayUrl(p->locationStack().at(index.row()).document().str(), m_base);
                    } case Line:
                        if (p->finalLocation().isValid())
                            return QString::number(p->finalLocation().start().line() + 1);
                        break;
                    case Column:
                        if (p->finalLocation().isValid())
                            return QString::number(p->finalLocation().start().column() + 1);
                        break;
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
        ///@todo Make location-stack work again

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

void ProblemModel::setProblems(const QList<KDevelop::ProblemPointer>& problems, KUrl base)
{
  m_base = base;
  m_problems = problems;
  reset();
}

QList< ProblemPointer > ProblemModel::allProblems() const
{
    return m_problems;
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

/* KDevelop coverage plugin
 *    Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team <bernd@kdevelop.org>
 *    Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "covoutputmodel.h"

#include <QModelIndex>
#include <KGlobalSettings>
#include <interfaces/icore.h>

using Veritas::CovOutputModel;

CovOutputModel::CovOutputModel(QObject *parent)
        : QStandardItemModel(parent)
{}

CovOutputModel::~CovOutputModel()
{}

QModelIndex CovOutputModel::nextHighlightIndex(const QModelIndex& currentIndex)
{
    Q_UNUSED(currentIndex);
    return QModelIndex();
}

QModelIndex CovOutputModel::previousHighlightIndex(const QModelIndex& currentIndex)
{
    Q_UNUSED(currentIndex);
    return QModelIndex();
}


QVariant CovOutputModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::FontRole) {
        return QStandardItemModel::data(index, role);
    } else {
        return KGlobalSettings::fixedFont();
    }
}

void CovOutputModel::activate(const QModelIndex &idx)
{}

void CovOutputModel::appendOutputs(const QStringList &lines)
{
    // stdout gets the lcov output, which contains the useful stuff
    // so do nothing here. stderr gets status messages
}

void CovOutputModel::appendErrors(const QStringList &lines)
{
    foreach(const QString& line, lines) {
        appendRow(new QStandardItem(line));
    }
}

void CovOutputModel::slotCompleted()
{
}

void CovOutputModel::slotFailed()
{
    appendRow(new QStandardItem("Failed"));
}

#include "covoutputmodel.moc"

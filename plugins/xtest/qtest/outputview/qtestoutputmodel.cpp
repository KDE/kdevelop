/* KDevelop xUnit plugin
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

#include "qtestoutputmodel.h"

#include <QModelIndex>
#include <KGlobalSettings>
#include <kdevplatform/interfaces/icore.h>

QTestOutputItem::QTestOutputItem(const QString &text)
        : QStandardItem(), m_text(text)
{
    setText(text);
}

QTestOutputItem::~QTestOutputItem()
{}

QTestOutputModel::QTestOutputModel(QObject *parent)
        : QStandardItemModel(parent)
{}

QTestOutputModel::~QTestOutputModel()
{}

QVariant QTestOutputModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::FontRole) {
        return QStandardItemModel::data(index, role);
    } else {
        return KGlobalSettings::fixedFont();
    }
}

void QTestOutputModel::activate(const QModelIndex &idx)
{
    QStandardItem *stditem = itemFromIndex(idx);
    QTestOutputItem *qtestoutputitem = dynamic_cast<QTestOutputItem*>(stditem);
    if (!qtestoutputitem) return;
}

QModelIndex QTestOutputModel::nextHighlightIndex(const QModelIndex& currentIndex)
{
    int nextRow = currentIndex.row() + 1;
    int rows = rowCount(currentIndex.parent());
    while (nextRow != currentIndex.row()) {
        QModelIndex idx = index(nextRow, currentIndex.column(), currentIndex.parent());
        if (QTestOutputItem* item = dynamic_cast<QTestOutputItem*>(itemFromIndex(idx))) {
            return idx;
        }
        ++nextRow;
        if (nextRow >= rows) {
            if (currentIndex.row() == -1) {
                break;
            } else {
                nextRow = 0;
            }
        }
    }
    return QModelIndex();
}

QModelIndex QTestOutputModel::previousHighlightIndex(const QModelIndex& currentIndex)
{
    int prevRow = currentIndex.row() - 1;
    int rows = rowCount(currentIndex.parent());
    do {
        if (prevRow < 0) {
            prevRow = rows - 1;
        }
        QModelIndex idx = index(prevRow, currentIndex.column(), currentIndex.parent());
        if (QTestOutputItem* item = dynamic_cast<QTestOutputItem*>(itemFromIndex(idx))) {
            return idx;
        }
        --prevRow;
    } while (prevRow != currentIndex.row());
    return QModelIndex();
}

void QTestOutputModel::appendOutputs(const QStringList &lines)
{
    foreach(QString line, lines) {
        appendRow(new QStandardItem(line));
    }
}

void QTestOutputModel::appendErrors(const QStringList &lines)
{
    foreach(QString line, lines) {
        appendRow(new QStandardItem(line));
    }
}

void QTestOutputModel::slotCompleted()
{}

void QTestOutputModel::slotFailed()
{
    appendRow(new QStandardItem("Failed"));
}

#include "qtestoutputmodel.moc"

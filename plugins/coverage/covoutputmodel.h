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

#ifndef VERITAS_COVERAGE_COVOUTPUTMODEL_H
#define VERITAS_COVERAGE_COVOUTPUTMODEL_H

#include <QStandardItemModel>
#include <outputview/ioutputviewmodel.h>

class QModelIndex;

namespace Veritas
{

class CovOutputItem : public QStandardItem
{
public:
    CovOutputItem(const QString &text);
    ~CovOutputItem();
    QString m_text;
};

class CovOutputModel : public QStandardItemModel, public KDevelop::IOutputViewModel
{
    Q_OBJECT

public:
    explicit CovOutputModel(QObject *parent);
    ~CovOutputModel();

    void activate(const QModelIndex &idx);
    QModelIndex nextHighlightIndex(const QModelIndex& currentIndex);
    QModelIndex previousHighlightIndex(const QModelIndex& currentIndex);
    QVariant data(const QModelIndex& index, int role) const;

public Q_SLOTS:
    void appendOutputs(const QStringList &lines);
    void appendErrors(const QStringList &lines);
    void slotCompleted();
    void slotFailed();
};

}

#endif // VERITAS_COVERAGE_COVOUTPUTMODEL_h

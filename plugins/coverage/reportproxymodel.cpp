/* KDevelop coverage plugin
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


#include "reportproxymodel.h"
#include "reportmodel.h"

using Veritas::ReportModel;
using Veritas::ReportProxyModel;

ReportProxyModel::ReportProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent), m_state(DirView)
{
    setDynamicSortFilter(true);
    setSortRole(ReportModel::SortRole);
}

ReportProxyModel::~ReportProxyModel()
{}


bool ReportProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
{
    Q_UNUSED(source_parent);
    switch(m_state) {
    case DirView: {
        return source_column == 0 || source_column == 1;
    }case FileView: {
        return true;
    } default: {
        Q_ASSERT_X(0, "filterAcceptsColumn", "Serious corruption, impossible m_state value.");
        return false;
    }}
}

void ReportProxyModel::setFileViewState()
{
    m_state = ReportProxyModel::FileView;
}

void ReportProxyModel::setDirViewState()
{
    m_state = ReportProxyModel::DirView;
}


#include "reportproxymodel.moc"

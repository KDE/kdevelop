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

#ifndef VERITAS_COVERAGE_REPORTPROXYMODEL_H
#define VERITAS_COVERAGE_REPORTPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "coverageexport.h"

namespace Veritas
{

/*! Used to filter coverage directories from the model */
class VERITAS_COVERAGE_EXPORT ReportProxyModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
    ReportProxyModel(QObject* parent);
    virtual ~ReportProxyModel();

    void setFileViewState();
    void setDirViewState();

protected:
    bool filterAcceptsColumn(int, const QModelIndex&) const;

private:
    enum State { DirView, FileView } m_state;
};

}

#endif

/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#ifndef VERITAS_INTERNAL_ITESTFRAMEWORK_P_H
#define VERITAS_INTERNAL_ITESTFRAMEWORK_P_H

#include <QtCore/QObject>

namespace Sublime { class View; }

namespace Veritas
{

class ITestFramework;
class ITestFrameworkPrivate : public QObject
{
Q_OBJECT
public:
    ITestFrameworkPrivate();
    ITestFramework* self;

private:
    void removeResultsView(const QString& docId);

private slots:
    void fixMovedResultsView(Sublime::View* v);
    void maybeRemoveResultsView(Sublime::View* v);
};

}

#endif // VERITAS_INTERNAL_ITESTFRAMEWORK_P_H

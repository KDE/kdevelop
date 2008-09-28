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

#ifndef VERITAS_TOOLVIEWDATA_H
#define VERITAS_TOOLVIEWDATA_H

#include <QMap>
#include <sublime/view.h>
#include "../itestrunner.h"
#include "../itestframework.h"

namespace Veritas
{

struct ToolViewData;
extern QMap<ITestFramework*, ToolViewData> g_toolViewStore;

struct ToolViewData
{
    ToolViewData() : runnerToolCounter(0) {}

    int runnerToolCounter; // Ugly toolview identifer that increments whenever
                           // a new runnertoolview is created. Used to implement
                           // parent-child relation between runner and
                           // result toolview. To be removed one day.
    QMap<Sublime::View*, int> view2id; // more of the above, maps sublime views 
                                       // to their id
    QMap<ITestRunner*, int> runner2id;

    void registerToolView(Sublime::View* v) {
        view2id[v] = runnerToolCounter-1;
    }
};
}

#endif // VERITAS_TOOLVIEWDATA_H

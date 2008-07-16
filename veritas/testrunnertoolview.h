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

#ifndef VERITAS_TESTRUNNERTOOLVIEW_H
#define VERITAS_TESTRUNNERTOOLVIEW_H

#include "interfaces/iplugin.h"
#include "veritas/veritasexport.h"

class QWidget;
class KComponentData;
namespace KDevelop { class IProject; }

namespace Veritas
{
class Test;
class RunnerModel;
class RunnerWindow;
class TestRunnerToolViewPrivate;

class VERITAS_EXPORT TestRunnerToolView : public KDevelop::IPlugin
{
Q_OBJECT
public:
    TestRunnerToolView(const KComponentData&, QObject*);
    virtual ~TestRunnerToolView();

    /*! Create a new test runner widget
        Call this in your toolview factory's create() member. */
    QWidget* spawnWindow();

protected:
    /*!
     * Reload the test tree.
     * To be implemented by concrete plugins.
     */
    virtual Test* registerTests() = 0;
    KDevelop::IProject* project() const;

private Q_SLOTS:
    void reload();
    void setSelected(QAction*);

private:
    TestRunnerToolViewPrivate* const d;
};

}

#endif // VERITAS_TESTRUNNER_H

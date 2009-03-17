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

#ifndef VERITAS_ITESTRUNNER_H
#define VERITAS_ITESTRUNNER_H

#include "../interfaces/iplugin.h"
#include "veritasexport.h"

class QWidget;
namespace KDevelop { class IProject; }
namespace Sublime { class View; }

namespace Veritas
{
class Test;
class ITestFramework;
class ITestTreeBuilder;

/*! This Test runner combines 2 toolviews:
 *  (i)  a runner-toolview which contains the test-tree
 *  (ii) a results-toolview that holds test status information,
 *       for example failure message, location and so on.
 *
 * TestRunner objects belong to an ITestFramework (plugin). */
class VERITAS_EXPORT TestRunner : public QObject
{
Q_OBJECT
public:
    TestRunner(ITestFramework*, ITestTreeBuilder*);
    virtual ~TestRunner();

    /*! Create a new test runner widget
        Call this in your toolview factory's create() member. */
    QWidget* runnerWidget();

    /*! The associated test results widget
        This shows assertion failures with source location */
    QWidget* resultsWidget();

public Q_SLOTS:
    void setupToolView(Veritas::Test*);

protected:
    KDevelop::IProject* project() const;

private Q_SLOTS:
    void removeResultsView();
    void reloadTree();
    void resetOnProjectClose(KDevelop::IProject*);

private:
    void spawnResultsView();

    class Private;
    Private* const d;
};

}

#endif // VERITAS_ITESTRUNNER_H

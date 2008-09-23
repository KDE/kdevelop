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

#include "interfaces/iplugin.h"
#include "veritas/veritasexport.h"

class QWidget;
namespace KDevelop { class IProject; }
namespace Sublime { class View; }

namespace Veritas
{
class Test;
class ITestFramework;

/*! Abstract test runner. A test runner has of 2 kdevelop toolviews:
 *  (i)  a runner-toolview which contains the test-tree
 *  (ii) a results-toolview that holds test status information,
 *       for example failure message, location and so on.
 *
 * ITestRunner objects belong to an ITestFramework (plugin). One single
 * framework can have multiple ITestRunners active at the same time.
 *
 *  for example implementations see kdevelop/plugins/xtest/qtest. */
class VERITAS_EXPORT ITestRunner : public QObject
{
Q_OBJECT
public:
    ITestRunner(ITestFramework*);
    virtual ~ITestRunner();

    /*! Create a new test runner widget
        Call this in your toolview factory's create() member. */
    QWidget* runnerWidget();

    /*! The associated test results widget
        This shows assertion failures with source location */
    QWidget* resultsWidget();

protected:
    /*! Reload the test tree.
     * To be implemented by concrete plugins. */
    virtual void registerTests() = 0;

    KDevelop::IProject* project() const;
    virtual QString resultsViewId() = 0;

protected Q_SLOTS:
    virtual void openVerbose(Test*) = 0;

Q_SIGNALS:
    void registerFinished(Veritas::Test* root);

private Q_SLOTS:
    void reload();
    void setSelected(QAction*);
    void removeResultsView();
    void setupToolView(Veritas::Test*);

private:
    void spawnResultsView();

    class Private;
    Private* const d;
};

}

#endif // VERITAS_ITESTRUNNER_H

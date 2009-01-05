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

#include "testtoolviewfactory.h"
#include "itestframework.h"
#include "testrunner.h"
#include "internal/toolviewdata.h"

#include <sublime/view.h>

#include <QWidget>
#include <QMap>

using Veritas::ITestFramework;
using Veritas::TestRunner;
using Veritas::TestToolViewFactory;

class TestToolViewFactory::TestToolViewFactoryPrivate
{
public:
    TestToolViewFactoryPrivate() : framework(0) {}
    ITestFramework* framework; // a kdevelop plugin
};

TestToolViewFactory::TestToolViewFactory(ITestFramework* framework)
    : d(new TestToolViewFactoryPrivate)
{
    d->framework = framework;
}

TestToolViewFactory::~TestToolViewFactory()
{
    delete d;
}

QWidget* TestToolViewFactory::create(QWidget *parent)
{
    Q_UNUSED(parent);
    TestRunner* runner = d->framework->createRunner();
    QWidget* runnerWidget = runner->runnerWidget();
    QObject::connect(runnerWidget, SIGNAL(destroyed(QObject*)),
                     runner, SLOT(deleteLater()));
    return runnerWidget;
}

Qt::DockWidgetArea TestToolViewFactory::defaultPosition()
{
    return Qt::LeftDockWidgetArea;
}

QString TestToolViewFactory::id() const
{
    return QString("org.kdevelop.%1TestRunner").arg(d->framework->name());
}

void TestToolViewFactory::viewCreated(Sublime::View* view)
{
    Q_ASSERT(g_toolViewStore.contains(d->framework));
    ToolViewData& tv = g_toolViewStore[d->framework];
    tv.registerToolView(view);
}

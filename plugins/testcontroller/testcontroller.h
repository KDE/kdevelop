/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVELOP_TESTCONTROLLER_H
#define KDEVELOP_TESTCONTROLLER_H

#include "interfaces/itestcontroller.h"
#include "interfaces/iplugin.h"
#include <QtCore/QVariantList>

class TestControllerPrivate;

class TestController : public KDevelop::IPlugin, public KDevelop::ITestController
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ITestController)
public:
    explicit TestController(QObject *parent, const QVariantList &args = QVariantList());
    virtual ~TestController();

    virtual void removeTestSuite(KDevelop::ITestSuite* suite);
    virtual void addTestSuite(KDevelop::ITestSuite* suite);

    virtual QList< KDevelop::ITestSuite* > testSuites() const;    
    virtual KDevelop::ITestSuite* testSuiteForUrl(const KUrl& url) const;
    virtual QList< KDevelop::ITestSuite* > testSuitesForProject(KDevelop::IProject* project) const;

signals:
    virtual void testSuiteAdded(KDevelop::ITestSuite* suite) const;
    virtual void testSuiteRemoved(KDevelop::ITestSuite* suite) const;

private:
    TestControllerPrivate* const d;
};

#endif // KDEVELOP_TESTCONTROLLER_H

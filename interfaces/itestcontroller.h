/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVELOP_ITESTCONTROLLER_H
#define KDEVELOP_ITESTCONTROLLER_H

#include "interfacesexport.h"

#include <QtCore/QList>
#include <QtCore/QObject>

class KJob;
class KUrl;

namespace KDevelop {

class IProject;
class ITestSuite;

class KDEVPLATFORMINTERFACES_EXPORT ITestController
{
public:
    virtual ~ITestController();

    virtual void addTestSuite(ITestSuite* suite) = 0;
    virtual void removeTestSuite(ITestSuite* suite) = 0;

    virtual QList<ITestSuite*> testSuites() const = 0;
    virtual ITestSuite* testSuiteForUrl(const KUrl& url) const = 0;
    virtual QList<ITestSuite*> testSuitesForProject(IProject* project) const = 0;
    
    virtual KJob* reloadTestSuites() = 0;

protected:
    virtual void testSuiteAdded(ITestSuite* suite) const = 0;
    virtual void testSuiteRemoved(ITestSuite* suite) const = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::ITestController, "org.kdevelop.ITestController")

#endif // KDEVELOP_ITESTCONTROLLER_H

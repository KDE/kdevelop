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

#ifndef VERITAS_ITESTFRAMEWORK_H
#define VERITAS_ITESTFRAMEWORK_H

#include <interfaces/iextension.h>
#include "veritasexport.h"

namespace Veritas
{
class ITestRunner;

/*! An extension interface for (xUnit) test frameworks. Plugins that implement this
 *  will have access to the veritas runner & result toolview.
 *
 *  For examples see the qtest, check or cppunit implementations in
 *  kdevelop/plugins/xtest */
class ITestFrameworkPrivate;
class VERITAS_EXPORT ITestFramework
{
public:
    ITestFramework();
    virtual ~ITestFramework();

    /*! Single word that describes the framework. eg 'QTest', 'CppUnit' */
    virtual QString name() const = 0;

    /*! Factory method that constructs a test runner. To be implemented
     *  by concrete frameworks. @see Veritas::ITestRunner */
    virtual ITestRunner* createRunner() = 0;

private:
    ITestFrameworkPrivate* const d;
};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS( Veritas, ITestFramework, "org.kdevelop.ITestFramework")
Q_DECLARE_INTERFACE( Veritas::ITestFramework, "org.kdevelop.ITestFramework")

#endif // VERITAS_ITESTFRAMEWORK_H

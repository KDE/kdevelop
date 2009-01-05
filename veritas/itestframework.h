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
#include <QVariantList>
#include "veritasexport.h"


namespace KDevelop { class ProjectConfigSkeleton; }

namespace Veritas
{

class TestRunner;

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

    /*! Factory method which constructs a test runner. To be implemented
     *  by concrete frameworks. @see Veritas::ITestRunner */
    virtual TestRunner* createRunner() = 0;

    /*! Factory method which constructs a configuration widget for this framework.
     *  Implementations should both create the widget and set the current configuration
     *  values in this widget. If null is returned, no extra config widget is present.
     *  Caller takes ownership. */
    virtual QWidget* createConfigWidget() { return 0; }

    /*! Framework specific configuration. */
    virtual KDevelop::ProjectConfigSkeleton* configSkeleton(const QVariantList& args) { Q_UNUSED(args); return 0; }

private:
    ITestFrameworkPrivate* const d;
};

/*! Initialize a ProjectConfigSkeleton [the template parameter] */
template <typename T>
void initializeProjectConfig(const QVariantList& args) {
    // NOTE does not quite belong here.
    Q_ASSERT( args.count() > 3 );
    T::instance( args.at(0).toString() );
    T::self()->setDeveloperTempFile( args.at(0).toString() );
    T::self()->setProjectTempFile( args.at(1).toString() );
    T::self()->setProjectFileUrl( args.at(2).toString() );
    T::self()->setDeveloperFileUrl( args.at(3).toString() );
}

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS( Veritas, ITestFramework, "org.kdevelop.ITestFramework")
Q_DECLARE_INTERFACE( Veritas::ITestFramework, "org.kdevelop.ITestFramework")

#endif // VERITAS_ITESTFRAMEWORK_H

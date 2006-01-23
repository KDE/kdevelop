/* This file is part of the KDE project
 * Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
 * Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <kgenericfactory.h>
#include <kaboutdata.h>

/**
@file kdevgenericfactory.h
KDevelop generic plugin factory.
*/

/**
This class provides a generic implementation of a KLibFactory for
use with KDevelop plugins.
Usually it is convenient to use K_EXPORT_COMPONENT_FACTORY macro
to create factories for KDevelop plugins. For example, for DummyPlugin
the factory can be created (in dummyplugin.cpp file) as:
@code
typedef KDevGenericFactory<DummyPlugin> DummyPluginFactory;
K_EXPORT_COMPONENT_FACTORY(libkdevdummyplugin, DummyPluginFactory( data ) )
@endcode
Data should be a const static object. This way it complies with the requirements
for data objecs of KDevGenericFactory constructor.

<b>Important:</b><br>
There is no need to create @ref KAboutData objects. It is more useful to create 
a static const @ref KDevPluginInfo object which can be used also in the constructor
of a plugin.

For example, dummyplugin.cpp file could contain:
@code
#include <kdevplugininfo.h>

static const KDevPluginInfo data("KDevDummyPlugin");
typedef KDevGenericFactory<DummyPlugin> DummyPluginFactory;
K_EXPORT_COMPONENT_FACTORY(libkdevdummyplugin, DummyPluginFactory( data ) )

DummyPlugin::DummyPlugin(QObject *parent, const char *name, const QStringList & )
    :KDevPlugin(&data, parent)
{
}
@endcode

In the example above the duplication of information is avoided as same @ref KDevPluginInfo
objects are used for plugin and for plugin factory. This is possible because @ref KDevPluginInfo
class has an operator to cast @ref KDevPluginInfo to @ref KAboutData.
*/
template <class T, class ParentType = QObject>
class KDevGenericFactory: public KGenericFactory<T, ParentType>
{
public:
    /**Constructor.
    @param data A reference to KAboutData with an information about the plugin.
    Data should have:
    - plugin name as an application name;
    - untranslated plugin generic name as a product name;
    - license type number.
    .
    data object should live as long as factory lives.*/
    KDevGenericFactory(KAboutData *data)
        :KGenericFactory<T, ParentType>(data->appName()), aboutData(data)
    {
    }

    /**Creates an instance.*/
    KInstance *createInstance()
    {
        return new KInstance(aboutData);
    }

private:
    KAboutData *aboutData;

};


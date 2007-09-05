/* This file is part of the KDE project
   Copyright (C) 2007 Alexander Dymo <adymo@kdevelop.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KDEVQUICKOPEN_H
#define KDEVQUICKOPEN_H

#include <kdevplugin.h>
#include <kurl.h>

/**
@file kdevquickopen.h
Source formatter interface.
*/

/**
Quick open plugin interface.

Use it when you need to present a dialog to choose between files to open.
@code
KDevQuickOpen *qo = extension<KDevQuickOpen>("KDevelop/QuickOpen");
if (qo) {
    // do something
} else {
    // fail
}
@endcode
@sa @ref KDevPlugin::extension method documentation.
@sa @ref whatisextension and @ref creatingextension sections of Platform API documentation.
*/
class KDevQuickOpen : public KDevPlugin
{
public:
    /**Constructor.
    @param info Important information about the plugin - plugin internal and generic
    (GUI) name, description, a list of authors, etc. That information is used to show
    plugin information in various places like "about application" dialog, plugin selector
    dialog, etc. Plugin does not take ownership on info object, also its lifetime should
    be equal to the lifetime of the plugin.
    @param parent The parent object for the plugin. Parent object must implement @ref KDevApi
    interface. Otherwise the plugin will not be constructed.
    @param name The internal name which identifies the plugin.*/
    KDevQuickOpen(const KDevPluginInfo *info, QObject* parent, const char* name)
        :KDevPlugin(info, parent, name) {}

    /**Shows the file selection dialog.
    @param text A list of urls to open.*/
    virtual void quickOpenFile(const KURL::List urls) = 0;
};

#endif
/* This file is part of the KDE project
   Copyright (C) 2007 Alexander Dymo <adymo@kdevelop.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KDEVQUICKOPEN_H
#define KDEVQUICKOPEN_H

#include <kdevplugin.h>
#include <kurl.h>

/**
@file kdevquickopen.h
Source formatter interface.
*/

/**
Quick open plugin interface.

Use it when you need to present a dialog to choose between files to open.
@code
KDevQuickOpen *qo = extension<KDevQuickOpen>("KDevelop/QuickOpen");
if (qo) {
    // do something
} else {
    // fail
}
@endcode
@sa @ref KDevPlugin::extension method documentation.
@sa @ref whatisextension and @ref creatingextension sections of Platform API documentation.
*/
class KDevQuickOpen : public KDevPlugin
{
public:
    /**Constructor.
    @param info Important information about the plugin - plugin internal and generic
    (GUI) name, description, a list of authors, etc. That information is used to show
    plugin information in various places like "about application" dialog, plugin selector
    dialog, etc. Plugin does not take ownership on info object, also its lifetime should
    be equal to the lifetime of the plugin.
    @param parent The parent object for the plugin. Parent object must implement @ref KDevApi
    interface. Otherwise the plugin will not be constructed.
    @param name The internal name which identifies the plugin.*/
    KDevQuickOpen(const KDevPluginInfo *info, QObject* parent, const char* name)
        :KDevPlugin(info, parent, name) {}

    /**Shows the file selection dialog.
    @param text A list of urls to open.*/
    virtual void quickOpenFile(const KURL::List urls) = 0;
};

#endif

/* This file is part of the KDE project
   Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>

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
#ifndef KDEVSOURCEFORMATTER_H
#define KDEVSOURCEFORMATTER_H

#include <kdevplugin.h>

/**
@file kdevsourceformatter.h
Source formatter interface.
*/

/**
Source formatter interface.
This interface is responsible for formatting source files and strings of code.

Instances that implement this interface are available through extension architecture:
@code
KDevSourceFormatter *sf = extension<KDevSourceFormatter>("KDevelop/SourceFormatter");
if (sf) {
    // do something
} else {
    // fail
}
@endcode
@sa @ref KDevPlugin::extension method documentation.
@sa @ref whatisextension and @ref creatingextension sections of Platform API documentation.
*/
class KDevSourceFormatter : public KDevPlugin
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
    KDevSourceFormatter(const KDevPluginInfo *info, QObject* parent, const char* name)
        :KDevPlugin(info, parent, name) {}

    /**Formats the source.
    @param text A string with a code.
    @return The formatted string.*/
    virtual QString formatSource(const QString text) = 0;
    
    /**@return The indentation string. For example, tab or four spaces can be returned.*/
    virtual QString indentString() const = 0;
};

#endif

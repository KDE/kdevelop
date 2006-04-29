/* This file is part of the KDE project
   Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>
   Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>

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
#ifndef KDEVDIFFFRONTEND_H
#define KDEVDIFFFRONTEND_H

#include <kurl.h>
#include <kdevplugin.h>

/**
@file kdevdifffrontend.h
Diff frontend interface.
*/

/**
KDevelop diff frontend interface.
This is the abstract base class for plugins that want to display differencies between
files.

Instances that implement this interface are available through extension architecture:
@code
KDevDiffFrontend *df = extension<KDevDiffFrontend>("KDevelop/DiffFrontend");
if (df) {
    // do something
} else {
    // fail
}
@endcode
@sa KDevPlugin::extension method documentation.
*/
class KDEVINTERFACES_EXPORT KDevDiffFrontend : public KDevPlugin
{
  Q_OBJECT
public:
    /**Constructor.
    @param info Important information about the plugin - plugin internal and generic
    (GUI) name, description, a list of authors, etc. That information is used to show
    plugin information in various places like "about application" dialog, plugin selector
    dialog, etc. Plugin does not take ownership on info object, also its lifetime should
    be equal to the lifetime of the plugin.
    @param parent The parent object for the plugin. Parent object must implement @ref KDevApi
    interface. Otherwise the plugin will not be constructed.*/
    KDevDiffFrontend( const KDevPluginInfo *info, QObject *parent = 0);
    virtual ~KDevDiffFrontend();

    /**Displays the patch.
    @param diff A string which contains a patch in unified format.*/
    virtual void showDiff( const QString& diff ) = 0;

    /**Displays a patch file.
    @param url An url of the patch file.*/
    virtual void openURL( const KUrl &url ) = 0;

    /**Displays the difference between the two files.
    @param url1 First file to compare.
    @param url2 Second file to compare.*/
    virtual void showDiff( const KUrl &url1, const KUrl &url2 ) = 0;

};

#endif

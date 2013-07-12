/*
* This file is part of KDevelop
* Copyright 2010 Milian Wolff <mail@milianw.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_PLUGIN_IOPENWITH_H
#define KDEVPLATFORM_PLUGIN_IOPENWITH_H

#include <KUrl>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>

namespace KDevelop {

/**
 * Interface for the OpenWith plugin.
 */
class IOpenWith {
public:
    virtual ~IOpenWith() {}
    /**
     * Open @p files with the preferred applications or parts as chosen by the user.
     *
     * If the open with plugin was disabled by the user, the files will be opened
     * as text documents.
     */
    static void openFiles(const KUrl::List &files)
    {
        IPlugin* i = ICore::self()->pluginController()->pluginForExtension( "org.kdevelop.IOpenWith" );
        if (i) {
            KDevelop::IOpenWith* openWith = i->extension<KDevelop::IOpenWith>();
            Q_ASSERT(openWith);
            openWith->openFilesInternal(files);
            return;
        }

        foreach(const KUrl& url, files) {
            ICore::self()->documentController()->openDocument( url );
        }
    }

protected:
    virtual void openFilesInternal(const KUrl::List &files) = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::IOpenWith, "org.kdevelop.IOpenWith" )

#endif // KDEVPLATFORM_PLUGIN_IOPENWITH_H

/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_IOPENWITH_H
#define KDEVPLATFORM_PLUGIN_IOPENWITH_H

#include <QUrl>

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
    static void openFiles(const QList<QUrl> &files)
    {
        IPlugin* i = ICore::self()->pluginController()->pluginForExtension( QStringLiteral( "org.kdevelop.IOpenWith" ) );
        if (i) {
            auto* openWith = i->extension<KDevelop::IOpenWith>();
            Q_ASSERT(openWith);
            openWith->openFilesInternal(files);
            return;
        }

        for (const QUrl& url : files) {
            ICore::self()->documentController()->openDocument( url );
        }
    }

protected:
    virtual void openFilesInternal(const QList<QUrl> &files) = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::IOpenWith, "org.kdevelop.IOpenWith" )

#endif // KDEVPLATFORM_PLUGIN_IOPENWITH_H

/* This file is part of the KDevelop project
   Copyright (C) 2006 Matt Rogers <mattr@kde.org>

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

#ifndef KDEVREADWRITEPART_H
#define KDEVREADWRITEPART_H_

#include <kparts/part.h>
#include "kdevexport.h"

class KDevApi;
class KDevMainWindow;
class KDevCore;
class KDevProject;
class KDevLanguageSupport;
class KDevPluginController;
class KDevDocumentController;
class KDevPlugin;
class QDomDocument;
class Extension;


class KDEVINTERFACES_EXPORT KDevReadWritePart : public KParts::ReadWritePart
{
public:
    KDevReadWritePart( QObject* parent = 0 );
    virtual ~KDevReadWritePart();

    virtual void setApiInstance( KDevApi* api );

    /** @return A reference to the toplevel widget. */
    KDevMainWindow *mainWindow();

    /**
     * @return A reference to the application core - an object which provides
     *  basic functionalities for interpart communication / cooperation.
     */
    KDevCore *core() const;

    /**
     * @return A pointer to the current project or 0 if no
     * project is loaded.
     */
    KDevProject *project() const;

    /**
     * @return A reference to the language support component or 0 if no
     * support available.
     */
    KDevLanguageSupport *languageSupport() const;

    /**
     * @return A reference to the DOM tree that represents the project
     * file or 0 if no project is loaded.
     */
    QDomDocument *projectDom() const;

    /**
     * @return A reference to the document controller which is used to
     * manipulate loaded KParts.
     */
    KDevDocumentController *documentController() const;

    /**
     * @return A reference to the plugin controller which is used to
     * manipulate loaded plugin.
     */
    virtual KDevPluginController *pluginController() const;

    /**
     * Queries for the plugin which supports given service type (such plugins
     * are called extensions in KDevelop). Plugins that are already loaded will
     * be queried and the <b>first one loaded</b> to support the service type
     * will be returned. Any plugin can be an extension, only a
     * "ServiceTypes=..." entry is required in the .desktop file for that
     * plugin.
     *
     * The template argument is used as a type to cast the result to. This is
     * done because an extension is usually derived from a specific base class
     * and not directly from KDevPlugin.
     *
     * @param serviceType The service type of an extension (like
     * "KDevelop/SourceFormatter").
     * @param constraint The constraint which is applied when quering for the
     * service. This constraint is a KTrader constraint statement (like
     * "[X-KDevelop-Foo]=='MyFoo'").
     *
     * @return A KDevelop extension plugin for given service type or 0 if no
     * plugin supports it
     */
    template <class Extension>
    Extension *extension( const QString &serviceType,
                          const QString &constraint = QString() )
    {
        return static_cast<Extension*>( extension_internal( serviceType,
                                                            constraint ) );
    }

private:
    KDevPlugin *extension_internal( const QString &serviceType,
                                    const QString &constraint = QString() );

    class Private;
    Private* d;

};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; indent-mode cstyle;

/* This file is part of the KDE project
   Copyright 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright 2004,2007 Alexander Dymo <adymo@kdevelop.org>
   Copyright 2006 Adam Treat <treat@kde.org>
   Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QtCore/QObject>
#include <kxmlguiclient.h>

#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QPair>
#include <QtDesigner/QExtensionManager>
#include "interfacesexport.h"

class KIconLoader;
class QAction;

/**
 * Current KDevelop plugin interface version. Interfaces declare plugin version
 * to make sure old source (or binary) incompatible plugins are not loaded.
 * Increase this if it is necessary that old plugins stop working.
 */
#define KDEVELOP_PLUGIN_VERSION 4

/**
 * This macro adds an extension interface to register with the extension manager
 * Call this macro for all interfaces your plugin implements in its constructor
 */

#define KDEV_USE_EXTENSION_INTERFACE( Extension ) \
    addExtension( Q_TYPEID( Extension ) );

namespace KDevelop
{

class ICore;
class Context;

/**
 * The base class for all KDevelop plugins.
 *
 * Plugin is a component which is loaded into KDevelop shell at startup or by
 * request. Each plugin should have corresponding .desktop file with a
 * description. The .desktop file template looks like:
 * @code
 * [Desktop Entry]
 * Encoding=UTF-8
 * Type=Service
 * Name=
 * GenericName=
 * Comment=
 * Icon=
 * X-KDevelop-Plugin-Version=
 * X-KDevelop-Plugin-Homepage=
 * X-KDevelop-Plugin-BugsEmailAddress=
 * X-KDevelop-Plugin-Copyright=
 * X-KDE-Library=
 * X-KDevelop-Version=
 * X-KDevelop-Category=
 * X-KDevelop-Properties=
 * X-KDevelop-Args=
 * @endcode
 * <b>Description of parameters in .desktop file:</b>
 * - <i>Name</i> is a non-translatable name of a plugin, it is used in KTrader
 * queries to search for a plugin (required);
 * - <i>GenericName</i> is a translatable name of a plugin, it is used to show
 * plugin names in GUI (required);
 * - <i>Comment</i> is a short description about the plugin (optional);
 * - <i>Icon</i> is a plugin icon (preferred);
 * - <i>X-KDevelop-Plugin-Version</i> is a version of a plugin (optional);
 * - <i>X-KDevelop-Interfaces</i> is a list of extension interfaces that this
 * plugin implements (optional);
 * - <i>X-KDevelop-IRequired</i> is a list of extension interfaces that this
 * plugin depends on (optional);
 * - <i>X-KDevelop-IOptional</i> is a list of extension interfaces that this
 * plugin will use if they are available (optional);
 * - <i>X-KDevelop-Plugin-Homepage</i> is a home page of a plugin (optional);
 * - <i>X-KDevelop-Plugin-License</i> is a license (optional). can be: GPL,
 * LGPL, BSD, Artistic, QPL or Custom. If this property is not set, license is
 * considered as unknown;
 * - <i>X-KDevelop-Plugin-BugsEmailAddress</i> is an email address for bug
 * reports (optional);
 * - <i>X-KDevelop-Plugin-Copyright</i> is a copyright statement (optional);
 * - <i>X-KDE-Library</i> is a name of library which contains the plugin
 * (required);
 * - <i>X-KDevelop-Version</i> is a version of KDevelop interfaces which is
 * supported by the plugin (required);
 * - <i>X-KDevelop-Category</i> is a scope of a plugin (see below for
 * explanation) (required);
 * - <i>X-KDevelop-Args</i> is a list of additional arguments passed to plugins
 * constructor (optional);
 * - <i>X-KDevelop-Properties</i> is a list of properties which this plugin
 * supports, see the Profile documentation for an explanation (required to work
 * with shells that support profiles).
 *
 * Plugin scope can be either:
 * - Global
 * - Project
 * .
 * Global plugins are plugins which require only the shell to be loaded and do not operate on
 * the Project interface and/or do not use project wide information.\n
 * Core plugins are global plugins which offer some important "core" functionality and thus
 * are not selectable by user in plugin configuration pages.\n
 * Project plugins require a project to be loaded and are usually loaded/unloaded along with
 * the project.
 * If your plugin uses the Project interface and/or operates on project-related
 * information then this is a project plugin.
 *
 *
 * @sa Core class documentation for an information about features which are available to
 * plugins from shell applications.
 */
class KDEVPLATFORMINTERFACES_EXPORT IPlugin: public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    /**Constructs a plugin.
     * @param instance The instance for this plugin.
     * @param parent The parent object for the plugin.
     */
    IPlugin(const KComponentData &instance, QObject *parent);

    /**Destructs a plugin.*/
    virtual ~IPlugin();

    /**
     * Specifies the DockWidgetArea for this plugin to use when in docked UI mode.
     * The default implementation returns Qt::NoDockWidgetArea.
     * @return the DockWidgetArea for this plugin
     */
    virtual Qt::DockWidgetArea dockWidgetAreaHint() const;

    /**
     * Tells @ref MainWindow whether to display this plugin as the central widget when
     * in top-level UI mode. The default implementation returns false.
     * @return true if this plugin should be the central widget, false if it should not.
     */
    virtual bool isCentralPlugin() const;

    /**
     * Signal the plugin that it should cleanup since it will be unloaded soon.
     */
    virtual void unload();

    /**
     * Provides access to the global icon loader
     * @return the plugin's icon loader
     */
    KIconLoader* iconLoader() const;

    ICore *core() const;

    void registerExtensions();
    void unregisterExtensions();

    QStringList extensions() const;

    template<class Extension> Extension* extension()
    {
        return qt_extension<Extension*>( extensionManager(), this );
    }

    virtual QPair<QString,QList<QAction*> > requestContextMenuActions( KDevelop::Context* );

public Q_SLOTS:
    /**
     * Re-initialize the global icon loader
     */
    void newIconLoader() const;

protected:
    void addExtension( const QString& );

private:
    class IPluginPrivate*  const d;
    QExtensionManager* extensionManager();


};

}
#endif


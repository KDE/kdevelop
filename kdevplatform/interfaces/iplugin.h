/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2004, 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IPLUGIN_H
#define KDEVPLATFORM_IPLUGIN_H

#include <QObject>

#include <KXMLGUIClient>

#include "interfacesexport.h"

class KPluginMetaData;

namespace Sublime {
class MainWindow;
}

namespace KDevelop
{

class ICore;
class ConfigPage;
class Context;
class ContextMenuExtension;
struct ProjectConfigOptions;
class IPluginPrivate;

/**
 * The base class for all KDevelop plugins.
 *
 * Plugin is a component which is loaded into KDevelop shell at startup or by
 * request. Each plugin should have corresponding .desktop file with a
 * description. The .desktop file template looks like:
 * @code
 * [Desktop Entry]
 * Type=Service
 * Exec=blubb
 * Name=
 * Comment=
 * Icon=
 * X-KDE-Library=
 * X-KDE-PluginInfo-Name=
 * X-KDE-PluginInfo-Author=
 * X-KDE-PluginInfo-Version=
 * X-KDE-PluginInfo-License=
 * X-KDE-PluginInfo-Category=
 * X-KDevelop-Version=
 * X-KDevelop-Category=
 * X-KDevelop-Mode=GUI
 * X-KDevelop-LoadMode=
 * X-KDevelop-Languages=
 * X-KDevelop-SupportedMimeTypes=
 * X-KDevelop-Interfaces=
 * X-KDevelop-IOptional=
 * X-KDevelop-IRequired=
 * @endcode
 * <b>Description of parameters in .desktop file:</b>
 * - <i>Name</i> is a translatable name of a plugin, it is used in the plugin 
 *   selection list (required);
 * - <i>Comment</i> is a short description about the plugin (optional);
 * - <i>Icon</i> is a plugin icon (preferred);
 *   <i>X-KDE-library</i>this is the name of the .so file to load for this plugin (required);
 * - <i>X-KDE-PluginInfo-Name</i> is a non-translatable user-readable plugin
 *   identifier used in KTrader queries (required);
 * - <i>X-KDE-PluginInfo-Author</i> is a non-translateable name of the plugin
 *   author (optional);
 * - <i>X-KDE-PluginInfo-Version</i> is version number of the plugin (optional);
 * - <i>X-KDE-PluginInfo-License</i> is a license (optional). can be: GPL,
 * LGPL, BSD, Artistic, QPL or Custom. If this property is not set, license is
 * considered as unknown;
 * - <i>X-KDE-PluginInfo-Category</i> is used to categorize plugins (optional). can be:
 *    Core, Project Management, Version Control, Utilities, Documentation,
 *    Language Support, Debugging, Other
 *   If this property is not set, "Other" is assumed
 * - <i>X-KDevelop-Version</i> is the KDevPlatform API version this plugin
 *   works with (required);
 * - <i>X-KDevelop-Interfaces</i> is a list of extension interfaces that this
 * plugin implements (optional);
 * - <i>X-KDevelop-IRequired</i> is a list of extension interfaces that this
 * plugin depends on (optional); A list entry can also be of the form @c interface\@pluginname,
 * in which case a plugin of the given name is required which implements the interface.
 * - <i>X-KDevelop-IOptional</i> is a list of extension interfaces that this
 * plugin will use if they are available (optional);
 * - <i>X-KDevelop-Languages</i> is a list of the names of the languages the plugin provides
 *   support for (optional);
 * - <i>X-KDevelop-SupportedMimeTypes</i> is a list of mimetypes that the 
 *   language-parser in this plugin supports (optional);
 * - <i>X-KDevelop-Mode</i> is either GUI or NoGUI to indicate whether a plugin can run
 *   with the GUI components loaded or not (required);
 * - <i>X-KDevelop-Category</i> is a scope of a plugin (see below for
 * explanation) (required);
 * - <i>X-KDevelop-LoadMode</i> can be set to AlwaysOn in which case the plugin will
 *   never be unloaded even if requested via the API. (optional);
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
 * @sa Core class documentation for information about features available to
 * plugins from shell applications.
 */
class KDEVPLATFORMINTERFACES_EXPORT IPlugin: public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    /**Constructs a plugin.
     * @param componentName The component name for this plugin.
     * @param parent The parent object for the plugin.
     * @param metaData The metadata of the plugin.
     */
    IPlugin(const QString& componentName, QObject* parent, const KPluginMetaData& metaData);

    /**Destructs a plugin.*/
    ~IPlugin() override;

    /**
     * Signal the plugin that it should cleanup since it will be unloaded soon.
     */
    virtual void unload();

    /**
     * Provides access to the ICore implementation
     */
    ICore *core() const;

    /**
     * Convenience API to access an interface inherited by this plugin
     *
     * @return Instance to the specified interface, or nullptr
     */
    template<class Extension>
    inline Extension* extension()
    {
        return qobject_cast<Extension*>(this);
    }

    /**
     * Ask the plugin for a ContextActionContainer, which contains actions
     * that will be merged into the context menu.
     * @param context the context describing where the context menu was requested
     * @param parent a widget to use for memory management of QActions, QMenus etc. created only for this request
     * @returns a container describing which actions to merge into which context menu part
     */
    virtual ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent);

    /**
     * Can create a new KXMLGUIClient, and set it up correctly with all the plugins per-window GUI actions.
     *
     * The caller owns the created object, including all contained actions. The object is destroyed as soon as
     * the mainwindow is closed.
     *
     * The default implementation calls the convenience function @ref createActionsForMainWindow and uses it to fill a custom KXMLGUIClient.
     *
     * Only override this version if you need more specific features of KXMLGUIClient, or other special per-window handling.
     *
     * @param window The main window the actions are created for
     */
    virtual KXMLGUIClient* createGUIForMainWindow( Sublime::MainWindow* window );

    /**
     * This function allows allows setting up plugin actions conveniently. Unless createGUIForMainWindow was overridden,
     * this is called for each new mainwindow, and the plugin should add its actions to @p actions, and write its KXMLGui xml file
     * into @p xmlFile.
     *
     * @param window The main window the actions are created for
     * @param xmlFile Change this to the xml file that needed for merging the actions into the GUI
     * @param actions Add your actions here. A new set of actions has to be created for each mainwindow.
     */
    virtual void createActionsForMainWindow( Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions );

    /**
     * This function is necessary because there is no proper way to signal errors from plugin constructor.
     * @returns True if the plugin has encountered an error (and therefore has an error description), 
     * false otherwise.
     */
    bool hasError() const;
    
    /**
     * Description of the last encountered error, of an empty string if none.
     */
    QString errorDescription() const;

    /**
     * Set a @p description of the error encountered. An empty error
     * description implies no error in the plugin.
     */
    void setErrorDescription(QString const& description);

    /**
     * Get the global config page with the \p number, config pages from 0 to
     * configPages()-1 are available if configPages() > 0.
     *
     * @param number index of config page
     * @param parent parent widget for config page
     * @return newly created config page or NULL, if the number is out of bounds, default implementation returns NULL.
     * This config page should inherit from ProjectConfigPage, but it is not a strict requirement.
     * The default implementation returns @c nullptr.
     * @see perProjectConfigPages(), ProjectConfigPage
     */
    virtual ConfigPage* configPage(int number, QWidget *parent);

    /**
     * Get the number of available config pages for global settings.
     * @return number of global config pages. The default implementation returns zero.
     * @see configPage()
     */
    virtual int configPages() const;

    /**
     * Get the number of available config pages for per project settings.
     * @return number of per project config pages. The default implementation returns zero.
     * @see perProjectConfigPage()
     */
    virtual int perProjectConfigPages() const;

    /**
     * Get the per project config page with the \p number, config pages from 0 to
     * perProjectConfigPages()-1 are available if perProjectConfigPages() > 0.
     *
     * @param number index of config page
     * @param options The options used to initialize the ProjectConfigPage
     * @param parent parent widget for config page
     * @return newly created config page or NULL, if the number is out of bounds, default implementation returns NULL.
     * This config page should inherit from ProjectConfigPage, but it is not a strict requirement.
     * The default implementation returns @c nullptr.
     * @see perProjectConfigPages(), ProjectConfigPage
     */
    virtual ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent);

protected:
    /**
     * Initialize the XML GUI State.
     */
    virtual void initializeGuiState();

private:
    const QScopedPointer<class IPluginPrivate> d_ptr;
    Q_DECLARE_PRIVATE(IPlugin)
    friend class IPluginPrivate;
};

}

#endif

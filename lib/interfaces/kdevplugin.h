/* This file is part of the KDE project
   Copyright (C) 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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
#ifndef KDEVPLUGIN_H
#define KDEVPLUGIN_H

#include <qobject.h>
#include <kxmlguiclient.h>
#include "kdevapi.h"

class KDevCore;
class KDevProject;
class KDevVersionControl;
class KDevLanguageSupport;
class KDevPartController;
class KDevMainWindow;
class KDevCodeRepository;
class CodeModel;
class KDevPluginInfo;
class QDomElement;

/**
@file kdevplugin.h
KDevelop plugin interface.
*/

/**Current KDevelop plugin interface version. Interfaces declare plugin version to make sure
old source (or binary) incompatible plugins are not loaded. Increase this if 
it is necessary that old plugins stop working.*/
#define KDEVELOP_PLUGIN_VERSION 3

/**
The base class for all KDevelop plugins.
Plugin is a component which is loaded into KDevelop shell at startup or by request.
Each plugin should have corresponding .desktop file with a description.
.desktop file template looks like:
@code
[Desktop Entry]
Encoding=UTF-8
Type=Service
Name=
GenericName=
Comment=
Icon=
X-KDevelop-Plugin-Version=
X-KDevelop-Plugin-Homepage=
X-KDevelop-Plugin-BugsEmailAddress=
X-KDevelop-Plugin-Copyright=
X-KDE-Library=
X-KDevelop-Version=
X-KDevelop-Scope=
X-KDevelop-Properties=
X-KDevelop-Args=
@endcode
<b>Description of parameters in .desktop file:</b>
- <i>Name</i> is a non-translatable name of a plugin, it is used in KTrader queries
to search for a plugin (required);
- <i>GenericName</i> is a translatable name of a plugin, it is used to show
plugin names in GUI (required);
- <i>Comment</i> is a short description about the plugin (optional);
- <i>Icon</i> is a plugin icon (preferred);
- <i>X-KDevelop-Plugin-Version</i> is a version of a plugin (optional);
- <i>X-KDevelop-Plugin-Homepage</i> is a home page of a plugin (optional);
- <i>X-KDevelop-Plugin-License</i> is a license (optional). can be: GPL, LGPL, BSD, Artistic,
QPL or Custom. If this property is not set, license is considered as unknown;
- <i>X-KDevelop-Plugin-BugsEmailAddress</i> is an email address for bug reports (optional);
- <i>X-KDevelop-Plugin-Copyright</i> is a copyright statement (optional);
- <i>X-KDE-Library</i> is a name of library which contains the plugin (required);
- <i>X-KDevelop-Version</i> is a version of KDevelop interfaces which is supported by the plugin (required);
- <i>X-KDevelop-Scope</i> is a scope of a plugin (see below for explanation) (required);
- <i>X-KDevelop-Args</i> is a list of additional arguments passed to plugins constructor (optional);
- <i>X-KDevelop-Properties</i> is a list of properties which this plugin supports, see @ref KDevelopProfiles page for explanation (required to work with shells that support profiles).
.
Plugin scope can be either:
- Global
- Project
.
Global plugins are plugins which require only shell to be loaded and do not operate on @ref KDevProject interface
and/or do not use project wide information.
Project plugins require a project to be loaded and are usually loaded/unloaded among with the project.
If your plugin use @ref KDevProject interface and/or operate on project-related information then this is the project plugin.

@sa KDevGenericFactory class documentation for an information about plugin instantiation
and writing factories for plugins.

@sa KDevCore class documentation for an information about features which are available to plugins
from shell applications.
*/
class KDevPlugin: public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    /**Constructs a plugin.
    @param info Important information about the plugin - plugin internal and generic
    (GUI) name, description, a list of authors, etc. That information is used to show
    plugin information in various places like "about application" dialog, plugin selector
    dialog, etc. Plugin does not take ownership on info object, also its lifetime should
    be equal to the lifetime of the plugin.
    @param parent The parent object for the plugin. Parent object must implement @ref KDevApi
    interface. Otherwise the plugin will not be constructed.
    @param name The internal name which identifies the plugin.*/
    KDevPlugin(const KDevPluginInfo *info, QObject *parent, const char *name = 0);

    /**Destructs a plugin.*/
    virtual ~KDevPlugin();

    /**Provides an information about the plugin.
    @return KAboutData object which was initialized in the constructor.*/
    const KDevPluginInfo* info();

    /**@return A reference to the toplevel widget.*/
    KDevMainWindow *mainWindow();

    /**@return A reference to the application core - an object which provides
    basic functionalities for inter-parts communications / cooperation.*/
    KDevCore *core() const;
    
    /**@return A reference to the current project component or 0 if no project is loaded.*/
    KDevProject *project() const;
    
    /**@return A reference to the language support component or 0 if no support available.*/
    KDevLanguageSupport *languageSupport() const;
        
    /**@return A reference to the memory symbol store.*/
    CodeModel *codeModel() const;

    /**@return A reference to the DOM tree that represents the project file or 0 if no project is loaded.*/
    QDomDocument *projectDom() const;

    /**@return A reference to the part controller which is used to manipulate loaded KParts.*/
    KDevPartController *partController() const;

    /**@return A reference to the code repository (accessor to persistant symbol stores).*/
    KDevCodeRepository* codeRepository() const;

    /**Same as @ref extension(const QString & serviceType) but has one template argument.
    Template argument is used as a type to cast the result to. This is useful if extension
    is derived from a certain abstract base class.
    @param serviceType a service type of an extension (like "KDevelop/SourceFormatter")
    @return a KDevelop extension plugin for given service type or 0 if no plugin supports it*/
    template <class Extension>
    Extension *extension(const QString &serviceType, const QString &constraint = "")
    {
        KDevPlugin *plugin = extension(serviceType, constraint);
        if (plugin)
            return static_cast<Extension*>(plugin);
        else
            return 0;
    }

    /**@anchor extension
    Queries for the plugin which supports given service type.
    All already loaded plugins will be queried and the first one to support the service type
    will be returned. Any plugin can be an extension, only "ServiceTypes=..." entry is
    required in .desktop file for that plugin.
    @param serviceType a service type of an extension (like "KDevelop/SourceFormatter")
    @return a KDevelop extension plugin for given service type or 0 if no plugin supports it*/
    KDevPlugin *extension(const QString &serviceType, const QString &constraint = "");
    
    /**Override this base class method to restore any settings which differs from project to project.
    Data can be read from a certain subtree of the project session file.
    During project loading, respectively project session (.kdevses) loading,
    this method will be called to give a chance to adapt the plugin to
    the newly loaded project. For instance, the debugger plugin might restore the
    list of breakpoints from the previous debug session for the certain project.
    @note Take attention to the difference to common not-project-related session stuff.
          They belong to the application rc file (kdeveloprc)
    @note Project session file is useful for settings which cannot be shared between
          developers. If a setting should be shared, modify projectDom instead.
    @param el The parent DOM element for plugins session settings.*/
    virtual void restorePartialProjectSession(const QDomElement* el);

    /**Saves session settings.
    @sa restorePartialProjectSession - this is the other way round, the same just for saving.*/
    virtual void savePartialProjectSession(QDomElement* el);

private:
    KDevApi *m_api;
    class Private;
    Private *d;
};

#endif

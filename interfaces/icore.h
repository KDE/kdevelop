/* This file is part of KDevelop
Copyright 2007 Alexander Dymo <adymo@kdevelop.org>
Copyright 2007 Kris Wong <kris.p.wong@gmail.com>

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

#ifndef KDEVPLATFORM_ICORE_H
#define KDEVPLATFORM_ICORE_H

#include <QtCore/QObject>
#include "interfacesexport.h"

#include "isessionlock.h"

class KAboutData;

namespace KParts
{
class PartManager;
}

/**
 * The KDevelop namespace contains all classes provided by the KDevelop
 * platform libraries.
 */
namespace KDevelop
{

class IUiController;
class IPluginController;
class IProjectController;
class ILanguageController;
class IDocumentController;
class ISessionController;
class IRunController;
class ISourceFormatterController;
class ISession;
class ISelectionController;
class IDocumentationController;
class IDebugController;
class IPartController;
class IDashboardController;
class ITestController;

/**
 * ICore is the container class for all the various objects in use by
 * KDevelop. If access is needed to a particular controller, then this class
 * should be used.
 *
 * ICore can provide the user with instances of the following things:
 * - the main window(s)
 *   - the document controller(s)
 * - the plugin controller
 * - the project controller
 * - the language controller
 * - the KPart manager
 *
 * When an object is provided to ICore so it can be used later, ICore
 * will take ownership of the object and upon application shutdown will take
 * responsibility for deleting the objects stored by ICore.
 */
class KDEVPLATFORMINTERFACES_EXPORT ICore: public QObject
{
    Q_OBJECT

public:
    virtual ~ICore();

    /** @return the static ICore instance */
    static ICore *self();

    /** @return ui controller */
    Q_SCRIPTABLE virtual KDevelop::IUiController *uiController() = 0;

    /** @return plugin controller */
    Q_SCRIPTABLE virtual KDevelop::IPluginController *pluginController() = 0;

    /** @return project controller */
    Q_SCRIPTABLE virtual KDevelop::IProjectController *projectController() = 0;

    /** @return language controller */
    Q_SCRIPTABLE virtual KDevelop::ILanguageController *languageController() = 0;

    /** @return part manager */
    Q_SCRIPTABLE virtual KDevelop::IPartController *partController() = 0;

    /** @return document controller */
    Q_SCRIPTABLE virtual KDevelop::IDocumentController *documentController() = 0;

    /** @return run controller */
    Q_SCRIPTABLE virtual KDevelop::IRunController *runController() = 0;

    /** @return the active session */
    Q_SCRIPTABLE virtual KDevelop::ISession *activeSession() = 0;

    /** @return the session lock for the active session */
    Q_SCRIPTABLE virtual KDevelop::ISessionLock::Ptr activeSessionLock() = 0;

    /** @return the sourceformatter controller */
    Q_SCRIPTABLE virtual KDevelop::ISourceFormatterController *sourceFormatterController() = 0;

    /** @return the selection controller */
    Q_SCRIPTABLE virtual KDevelop::ISelectionController* selectionController() = 0;

    /** @return the documentation controller */
    Q_SCRIPTABLE virtual KDevelop::IDocumentationController* documentationController() = 0;

    /** @return the debug controller */
    Q_SCRIPTABLE virtual KDevelop::IDebugController* debugController() = 0;

    /** @return the test controller */
    Q_SCRIPTABLE virtual KDevelop::ITestController* testController() = 0;

    /** @return the about data of the framework, different from the main about data which is created by the application */
    virtual KAboutData aboutData() const = 0;

    /** @return true if the application is currently being shut down */
    virtual bool shuttingDown() const = 0;

Q_SIGNALS:
    /** Emitted when the initialization of the core components has been completed */
    void initializationCompleted();
    /**
     * Emitted immediately before tearing down the session and UI.  Useful when performing any last minute
     * preparations such as saving settings.
     */
    void aboutToShutdown();
    /**
     * Emitted when the teardown of the core components has been completed.
     */
    void shutdownCompleted();

protected:
    ICore(QObject *parent = 0);
    static ICore *m_self;
};

}

#endif

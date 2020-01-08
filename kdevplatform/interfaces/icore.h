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

#include <QObject>
#include "interfacesexport.h"

#include "isessionlock.h"

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
class IRuntimeController;

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
    Q_PROPERTY(KDevelop::IProjectController* projectController READ projectController)

public:
    ~ICore() override;

    /** @return the static ICore instance */
    static ICore *self();

    /** @return ui controller */
    virtual KDevelop::IUiController *uiController() = 0;

    /** @return plugin controller */
    virtual KDevelop::IPluginController *pluginController() = 0;

    /** @return project controller */
    virtual KDevelop::IProjectController *projectController() = 0;

    /** @return language controller */
    virtual KDevelop::ILanguageController *languageController() = 0;

    /** @return part manager */
    virtual KDevelop::IPartController *partController() = 0;

    /** @return document controller */
    virtual KDevelop::IDocumentController *documentController() = 0;

    /** @return run controller */
    virtual KDevelop::IRunController *runController() = 0;

    /** @return the active session */
    virtual KDevelop::ISession *activeSession() = 0;

    /** @return the session lock for the active session */
    virtual KDevelop::ISessionLock::Ptr activeSessionLock() = 0;

    /** @return the sourceformatter controller */
    virtual KDevelop::ISourceFormatterController *sourceFormatterController() = 0;

    /** @return the selection controller */
    virtual KDevelop::ISelectionController* selectionController() = 0;

    /** @return the documentation controller */
    virtual KDevelop::IDocumentationController* documentationController() = 0;

    /** @return the debug controller */
    virtual KDevelop::IDebugController* debugController() = 0;

    /** @return the test controller */
    virtual KDevelop::ITestController* testController() = 0;

    /** @return the runtime controller */
    Q_SCRIPTABLE virtual KDevelop::IRuntimeController* runtimeController() = 0;

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
    explicit ICore(QObject *parent = nullptr);
    static ICore *m_self;
};

}

#endif

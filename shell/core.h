
/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>             *
 *   Copyright 2007 Kris Wong <kris.p.wong@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef KDEVPLATFORM_CORE_H
#define KDEVPLATFORM_CORE_H

#include "shellexport.h"
#include <interfaces/icore.h>

class KAboutData;

namespace KDevelop {

class UiController;
class PluginController;
class ProjectController;
class LanguageController;
class PartController;
class DocumentController;
class RunController;
class SessionController;
class CorePrivate;
class SourceFormatterController;
class ProgressManager;
class SelectionController;
class DocumentationController;
class DebugController;
class WorkingSetController;
class TestController;

class KDEVPLATFORMSHELL_EXPORT Core: public ICore
{
    Q_OBJECT

public:
    enum Setup { Default=0, NoUi=1 };

    static QString version();

    /** Initialize the core of the kdevplatform application
      * returns false if the initialization fails, which may happen
      * if the same session is already active in another instance
      *
      * @param splash the splashscreen instance that shows the startup progress (may be 0)
      * @param mode the mode in which to run
      * @param session the name or uuid of the session to be loaded
      *
      */
    static bool initialize(QObject* splash = 0, Setup mode=Default, const QString& session = "" );

    /**
     * \brief Provide access an instance of Core
     */
    static Core *self();

    virtual ~Core();

    /** @copydoc ICore::uiController() */
    virtual IUiController *uiController() override;

    /** @copydoc ICore::pluginController() */
    virtual IPluginController *pluginController() override;

    /** @copydoc ICore::projectController() */
    virtual IProjectController *projectController() override;

    /** @copydoc ICore::languageController() */
    virtual ILanguageController *languageController() override;

    /** @copydoc ICore::partController() */
    virtual IPartController *partController() override;

    /** @copydoc ICore::documentController() */
    virtual IDocumentController *documentController() override;

    /** @copydoc ICore::runController() */
    virtual IRunController *runController() override;

    /** @copydoc ICore::sourceFormatterController() */
    virtual ISourceFormatterController* sourceFormatterController() override;

    /** @copydoc ICore::selectionController() */
    virtual ISelectionController* selectionController() override;

    /** @copydoc ICore::documentationController() */
    virtual IDocumentationController* documentationController() override;

    /** @copydoc ICore::debugController() */
    virtual IDebugController* debugController() override;

    /** @copydoc ICore::testController() */
    virtual ITestController* testController() override;

    /** @copydoc ICore::activeSession() */
    virtual ISession *activeSession() override;

    /** @copydoc ICore::activeSessionLock() */
    virtual ISessionLock::Ptr activeSessionLock() override;

    virtual KAboutData aboutData() const override;

    /// The following methods may only be used within the shell.

    /** @return ui controller */
    UiController *uiControllerInternal();

    /** @return plugin controller */
    PluginController *pluginControllerInternal();

    /** @return project controller */
    ProjectController *projectControllerInternal();

    /** @return language controller */
    LanguageController *languageControllerInternal();

    /** @return part manager */
    PartController *partControllerInternal();

    /** @return document controller */
    DocumentController *documentControllerInternal();

    /** @return run controller */
    RunController *runControllerInternal();

    /** @return documentation controller */
    DocumentationController *documentationControllerInternal();

    /** @return debug controller */
    DebugController *debugControllerInternal();

    /** @return working set controller */
    WorkingSetController* workingSetControllerInternal();

    /** @return the real source formatter controller */
    SourceFormatterController* sourceFormatterControllerInternal();

    /** @return test controller */
    TestController* testControllerInternal();

    /**
     * @return session controller
     * @internal
     */
    SessionController *sessionController();

    /**
     * @internal
     */
    ProgressManager *progressController();

    void cleanup();

    virtual bool shuttingDown() const override;

    Core::Setup setupFlags() const;

public slots:
    void shutdown();

signals:
    void startupProgress(int percent);

protected:
    friend class CorePrivate;
    Core( KDevelop::CorePrivate* dd, QObject* parent = 0 );
    KDevelop::CorePrivate *d;
    static Core *m_self;
private:
    Core(QObject *parent = 0);
};

}

#endif


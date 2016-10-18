
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
    static bool initialize(QObject* splash = nullptr, Setup mode=Default, const QString& session = {} );

    /**
     * \brief Provide access an instance of Core
     */
    static Core *self();

    virtual ~Core();

    virtual IUiController *uiController() override;
    virtual IPluginController *pluginController() override;
    virtual IProjectController *projectController() override;
    virtual ILanguageController *languageController() override;
    virtual IPartController *partController() override;
    virtual IDocumentController *documentController() override;
    virtual IRunController *runController() override;
    virtual ISourceFormatterController* sourceFormatterController() override;
    virtual ISelectionController* selectionController() override;
    virtual IDocumentationController* documentationController() override;
    virtual IDebugController* debugController() override;
    virtual ITestController* testController() override;
    virtual ISession *activeSession() override;
    virtual ISessionLock::Ptr activeSessionLock() override;

    virtual KAboutData aboutData() const override;

    /// The following methods may only be used within the shell.

    UiController *uiControllerInternal();
    PluginController *pluginControllerInternal();
    ProjectController *projectControllerInternal();
    LanguageController *languageControllerInternal();
    PartController *partControllerInternal();
    DocumentController *documentControllerInternal();
    RunController *runControllerInternal();
    DocumentationController *documentationControllerInternal();
    DebugController *debugControllerInternal();
    WorkingSetController* workingSetControllerInternal();
    SourceFormatterController* sourceFormatterControllerInternal();
    TestController* testControllerInternal();

    /// @internal
    SessionController *sessionController();
    /// @internal
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
    Core( KDevelop::CorePrivate* dd, QObject* parent = nullptr );
    KDevelop::CorePrivate *d;
    static Core *m_self;
private:
    Core(QObject *parent = nullptr);
};

}

#endif


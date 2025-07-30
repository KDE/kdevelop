/*
    SPDX-FileCopyrightText: 1999-2001 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_DEBUGCONTROLLER_H
#define KDEVPLATFORM_DEBUGCONTROLLER_H

#include <QPointer>

#include <KXMLGUIClient>

#include "../interfaces/idebugcontroller.h"
#include "../debugger/interfaces/idebugsession.h"

class QAction;

namespace Sublime {
    class Area;
}

namespace KTextEditor {
class Document;
}

namespace KDevelop {
class Context;
class ContextMenuExtension;
class IDocument;

class DebugController : public IDebugController, public KXMLGUIClient
{
    Q_OBJECT
public:
    explicit DebugController(QObject *parent = nullptr);
    ~DebugController() override;
    void initialize();
    void cleanup();

    /// Must be called by debugger plugin that needs debugger actions and tool views.
    void initializeUi() override;

    [[nodiscard]] bool canAddSession(const QString& replaceSessionQuestionText) const override;

    void addSession(IDebugSession* session) override;
    IDebugSession* currentSession() override;

    ContextMenuExtension contextMenuExtension(Context* context, QWidget* parent);

    BreakpointModel* breakpointModel() override;
    VariableCollection* variableCollection() override;

private Q_SLOTS:
    //void restartDebugger();
    void stopDebugger();
    void interruptDebugger();
    void run();
    void runToCursor();
    void jumpToCursor();
    void stepOver();
    void stepIntoInstruction();
    void stepInto();
    void stepOverInstruction();
    void stepOut();
    void toggleBreakpoint();
    void showCurrentLine();

    void debuggerStateChanged(KDevelop::IDebugSession::DebuggerState state);
    void showStepInSource(const QUrl &file, int line);
    void clearExecutionPoint();

    void textDocumentCreated(KDevelop::IDocument* document);
    void areaChanged(Sublime::Area* newArea);

Q_SIGNALS:
    void killAllDebuggersNow();

private:
    void setupActions();
    void updateDebuggerState(KDevelop::IDebugSession::DebuggerState state, KDevelop::IDebugSession* session);
    void setContinueStartsDebug(bool startsDebug);

    QAction* m_continueDebugger = nullptr;
    //QAction* m_restartDebugger;
    QAction* m_stopDebugger = nullptr;
    QAction* m_interruptDebugger = nullptr;
    QAction* m_runToCursor = nullptr;
    QAction* m_jumpToCursor = nullptr;
    QAction* m_stepOver = nullptr;
    QAction* m_stepIntoInstruction = nullptr;
    QAction* m_stepInto = nullptr;
    QAction* m_stepOverInstruction = nullptr;
    QAction* m_stepOut = nullptr;
    QAction* m_toggleBreakpoint = nullptr;
    QAction* m_showCurrentLine = nullptr;

    IDebugSession* m_currentSession = nullptr;
    BreakpointModel *m_breakpointModel;
    VariableCollection *m_variableCollection;

    QPointer<KTextEditor::Document> m_lastExecMarkDocument;
    /**
     * A line number to which the execution mark was last added.
     * The execution mark may no longer be at this line number if the document is modified after the adding.
     * This line number is meaningless if @a m_lastExecMarkDocument is null.
     */
    int m_lastExecMarkLine = -1;

    bool m_uiInitialized = false;
    // Initialize to false to let setupActions() flip the value and initialize the UI properties of the action.
    bool m_continueStartsDebug = false;
};

}

#endif

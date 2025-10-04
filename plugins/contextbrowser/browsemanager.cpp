/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "browsemanager.h"

#include <QApplication>
#include <QMouseEvent>
#include <QTimer>
#include <QMenuBar>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include "contextbrowserview.h"
#include <interfaces/iuicontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/forwarddeclaration.h>

#include "contextbrowser.h"
#include "debug.h"

#include <KParts/MainWindow>
#include <KTextEditor/Document>
#include <KTextEditor/View>

using namespace KDevelop;
using namespace KTextEditor;

EditorViewWatcher::EditorViewWatcher(QObject* parent)
    : QObject(parent)
{
    connect(
        ICore::self()->documentController(), &IDocumentController::textDocumentCreated, this,
        &EditorViewWatcher::documentCreated);
    const auto documents = ICore::self()->documentController()->openDocuments();
    for (KDevelop::IDocument* document : documents) {
        documentCreated(document);
    }
}

void EditorViewWatcher::documentCreated(KDevelop::IDocument* document)
{
    KTextEditor::Document* textDocument = document->textDocument();
    if (textDocument) {
        connect(textDocument, &Document::viewCreated, this, &EditorViewWatcher::viewCreated);
        const auto views = textDocument->views();
        for (KTextEditor::View* view : views) {
            Q_ASSERT(view->parentWidget());
            addViewInternal(view);
        }
    }
}

void EditorViewWatcher::addViewInternal(KTextEditor::View* view)
{
    m_views << view;
    viewAdded(view);
    connect(view, &View::destroyed, this, &EditorViewWatcher::viewDestroyed);
}

void EditorViewWatcher::viewAdded(KTextEditor::View*)
{
}

void EditorViewWatcher::viewDestroyed(QObject* view)
{
    m_views.removeAll(static_cast<KTextEditor::View*>(view));
}

void EditorViewWatcher::viewCreated(KTextEditor::Document* /*doc*/, KTextEditor::View* view)
{
    Q_ASSERT(view->parentWidget());
    addViewInternal(view);
}

QList<KTextEditor::View*> EditorViewWatcher::allViews()
{
    return m_views;
}

void BrowseManager::eventuallyStartDelayedBrowsing()
{
    avoidMenuAltFocus();

    if (m_browsingByKey == Qt::Key_Alt && m_browsingStartedInView)
        emit startDelayedBrowsing(m_browsingStartedInView);
}

BrowseManager::BrowseManager(ContextBrowserPlugin* controller)
    : QObject(controller)
    , m_plugin(controller)
    , m_browsing(false)
    , m_browsingByKey(0)
    , m_watcher(this)
{
    m_delayedBrowsingTimer = new QTimer(this);
    m_delayedBrowsingTimer->setSingleShot(true);
    m_delayedBrowsingTimer->setInterval(300);

    connect(m_delayedBrowsingTimer, &QTimer::timeout, this, &BrowseManager::eventuallyStartDelayedBrowsing);
}

KTextEditor::View* viewFromWidget(QWidget* widget)
{
    if (!widget)
        return nullptr;
    auto* view = qobject_cast<KTextEditor::View*>(widget);
    if (view)
        return view;
    else
        return viewFromWidget(widget->parentWidget());
}

BrowseManager::JumpLocation BrowseManager::determineJumpLoc(KTextEditor::Cursor textCursor, const QUrl& viewUrl) const
{
    // @todo find out why this is needed, fix the code in kate
    if (textCursor.column() > 0) {
        textCursor.setColumn(textCursor.column() - 1);
    }

    // Step 1: Look for a special language object(Macro, included header, etc.)
    const auto languages = ICore::self()->languageController()->languagesForUrl(viewUrl);
    for (const auto& language : languages) {
        auto jumpTo = language->specialLanguageObjectJumpCursor(viewUrl, textCursor);
        if (jumpTo.first.isValid() && jumpTo.second.isValid()) {
            return {
                       jumpTo.first, jumpTo.second
            };
        }
    }

    // Step 2: Look for a declaration/use
    DUChainReadLocker lock;
    // Jump to definition by default, unless a definition itself was selected,
    // in which case jump to declaration.
    // Also, when the definition is in a file for which we don't know the project,
    // that usually means it's a generated file and we rather want to open the declaration
    // after all. Unless the viewUrl and textCursor points at the declaration already.
    if (auto selectedDeclaration = DUChainUtils::itemUnderCursor(viewUrl, textCursor).declaration) {
        auto jumpDestination = selectedDeclaration;
        if (selectedDeclaration->isDefinition()) {
            // A definition was clicked directly - jump to declaration instead.
            if (auto declaration = DUChainUtils::declarationForDefinition(selectedDeclaration)) {
                jumpDestination = declaration;
            }
        } else if (selectedDeclaration == DUChainUtils::declarationForDefinition(selectedDeclaration)) {
            // Clicked the declaration - jump to definition
            // unless that isn't in a project
            if (auto definition = FunctionDefinition::definition(selectedDeclaration)) {
                const auto preferDefinition = [&]() -> bool {
                    // when we are jumping from declaration, we definitely want to go to the definition
                    if (viewUrl == selectedDeclaration->url().toUrl()
                        && selectedDeclaration->rangeInCurrentRevision().contains(textCursor))
                    {
                        return true;
                    }
                    // otherwise only jump to the definition when we know the project
                    // for the file in which the definition lives
                    // this won't be the case for generated files which are usually not interesting
                    // like e.g. Qt moc files - there, we want to jump to the header first
                    return ICore::self()->projectController()->findProjectForUrl(definition->url().toUrl());
                }();
                if (preferDefinition) {
                    jumpDestination = definition;
                }
            }
        }
        return {
                   jumpDestination->url().toUrl(), jumpDestination->rangeInCurrentRevision().start()
        };
    }

    return {};
}

bool BrowseManager::eventFilter(QObject* watched, QEvent* event)
{
    QWidget* widget = qobject_cast<QWidget*>(watched);
    Q_ASSERT(widget);

    auto* keyEvent = dynamic_cast<QKeyEvent*>(event);

    const int browseKey = Qt::Key_Control;
    const int magicModifier = Qt::Key_Alt;

    KTextEditor::View* view = viewFromWidget(widget);

    //Eventually start key-browsing
    if (keyEvent && (keyEvent->key() == browseKey || keyEvent->key() == magicModifier) && !m_browsingByKey &&
        keyEvent->type() == QEvent::KeyPress) {
        m_delayedBrowsingTimer->start(); // always start the timer, to get consistent behavior regarding the ALT key and the menu activation
        m_browsingByKey = keyEvent->key();

        if (!view) {
            return false;
        }

        if (keyEvent->key() == magicModifier) {
            if (view->isCompletionActive()) {
                //Completion is active.
                avoidMenuAltFocus();
                m_delayedBrowsingTimer->stop();
            } else {
                m_browsingStartedInView = view;
            }
        }
    }

    if (keyEvent && m_browsingByKey && m_browsingStartedInView && keyEvent->type() == QEvent::KeyPress) {
        if (keyEvent->key() >= Qt::Key_1 && keyEvent->key() <= Qt::Key_9) {
            // user wants to trigger an action in the code browser
            const int index = keyEvent->key() - Qt::Key_1;
            emit invokeAction(index);
            emit stopDelayedBrowsing();
            return true;
        }
    }

    if (!view) {
        return false;
    }

    auto* focusEvent = dynamic_cast<QFocusEvent*>(event);
    //Eventually stop key-browsing
    if ((keyEvent && m_browsingByKey && ( keyEvent->key() == m_browsingByKey || keyEvent->modifiers() == Qt::ControlModifier ) && keyEvent->type() == QEvent::KeyRelease)
        || (focusEvent && focusEvent->lostFocus()) || event->type() == QEvent::WindowDeactivate) {
        m_browsingByKey = 0;
        emit stopDelayedBrowsing();
    }

    auto* mouseEvent = dynamic_cast<QMouseEvent*>(event);

    if (mouseEvent) {
        if (mouseEvent->type() == QEvent::MouseButtonPress && mouseEvent->button() == Qt::XButton1) {
            m_plugin->historyPrevious();
            return true;
        }
        if (mouseEvent->type() == QEvent::MouseButtonPress && mouseEvent->button() == Qt::XButton2) {
            m_plugin->historyNext();
            return true;
        }
    }

    if (!m_browsing && !m_browsingByKey) {
        resetChangedCursor();
        return false;
    }

    if (mouseEvent) {
        QPoint coordinatesInView = widget->mapTo(view, mouseEvent->pos());
#if KTEXTEDITOR_VERSION >= QT_VERSION_CHECK(6, 20, 0)
        const auto textCursor =
            view->coordinatesToCursor(coordinatesInView, KTextEditor::View::InvalidCursorOutsideText);
#else
        KTextEditor::Cursor textCursor = view->coordinatesToCursor(coordinatesInView);
#endif
        if (textCursor.isValid()) {
            JumpLocation jumpTo = determineJumpLoc(textCursor, view->document()->url());
            if (jumpTo.isValid()) {
                if (mouseEvent->button() == Qt::LeftButton) {
                    if (mouseEvent->type() == QEvent::MouseButtonPress) {
                        m_buttonPressPosition = textCursor;
//                         view->setCursorPosition(textCursor);
//                         return false;
                    } else if (mouseEvent->type() == QEvent::MouseButtonRelease &&
                               textCursor == m_buttonPressPosition) {
                        ICore::self()->documentController()->openDocument(jumpTo.url, jumpTo.cursor);
//                         event->accept();
//                         return true;
                    }
                } else if (mouseEvent->type() == QEvent::MouseMove) {
                    //Make the cursor a "hand"
                    setHandCursor(widget);
                    return false;
                }
            }
        }
        resetChangedCursor();
    }
    return false;
}

void BrowseManager::resetChangedCursor()
{
    QMap<QPointer<QWidget>, QCursor> cursors = m_oldCursors;
    m_oldCursors.clear();

    for (QMap<QPointer<QWidget>, QCursor>::iterator it = cursors.begin(); it != cursors.end(); ++it)
        if (it.key())
            it.key()->setCursor(QCursor(Qt::IBeamCursor));
}

void BrowseManager::setHandCursor(QWidget* widget)
{
    if (m_oldCursors.contains(widget))
        return; //Nothing to do
    m_oldCursors[widget] = widget->cursor();
    widget->setCursor(QCursor(Qt::PointingHandCursor));
}

void BrowseManager::avoidMenuAltFocus()
{
    auto mainWindow = ICore::self()->uiController()->activeMainWindow();
    if (!mainWindow)
        return;

    // send an invalid key event to the main menu bar. The menu bar will
    // stop listening when observing another key than ALT between the press
    // and the release.
    QKeyEvent event1(QEvent::KeyPress, 0, Qt::NoModifier);
    QApplication::sendEvent(mainWindow->menuBar(), &event1);
    QKeyEvent event2(QEvent::KeyRelease, 0, Qt::NoModifier);
    QApplication::sendEvent(mainWindow->menuBar(), &event2);
}

void BrowseManager::applyEventFilter(QWidget* object, bool install)
{
    if (install)
        object->installEventFilter(this);
    else
        object->removeEventFilter(this);

    const auto children = object->children();
    for (QObject* child : children) {
        if (qobject_cast<QWidget*>(child))
            applyEventFilter(qobject_cast<QWidget*>(child), install);
    }
}

void BrowseManager::viewAdded(KTextEditor::View* view)
{
    applyEventFilter(view, true);
    //We need to listen for cursorPositionChanged, to clear the shift-detector. The problem: Kate listens for the arrow-keys using shortcuts,
    //so those keys are not passed to the event-filter

    // can't use new signal/slot syntax here, these signals are only defined in KateView
    // TODO: should we really depend on kate internals here?
    connect(view, SIGNAL(navigateLeft()), m_plugin, SLOT(navigateLeft()));
    connect(view, SIGNAL(navigateRight()), m_plugin, SLOT(navigateRight()));
    connect(view, SIGNAL(navigateUp()), m_plugin, SLOT(navigateUp()));
    connect(view, SIGNAL(navigateDown()), m_plugin, SLOT(navigateDown()));
    connect(view, SIGNAL(navigateAccept()), m_plugin, SLOT(navigateAccept()));
    connect(view, SIGNAL(navigateBack()), m_plugin, SLOT(navigateBack()));
}

void Watcher::viewAdded(KTextEditor::View* view)
{
    m_manager->viewAdded(view);
}

void BrowseManager::setBrowsing(bool enabled)
{
    if (enabled == m_browsing)
        return;
    m_browsing = enabled;

    //This collects all the views
    if (enabled) {
        qCDebug(PLUGIN_CONTEXTBROWSER) << "Enabled browsing-mode";
    } else {
        qCDebug(PLUGIN_CONTEXTBROWSER) << "Disabled browsing-mode";
        resetChangedCursor();
    }
}

Watcher::Watcher(BrowseManager* manager)
    : EditorViewWatcher(manager)
    , m_manager(manager)
{
    const auto views = allViews();
    for (KTextEditor::View* view : views) {
        viewAdded(view);
    }
}

#include "moc_browsemanager.cpp"

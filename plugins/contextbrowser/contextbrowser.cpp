/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "contextbrowser.h"

#include "contextbrowserview.h"
#include "browsemanager.h"
#include "debug.h"

#include <cstdlib>

#include <QAction>
#include <QDebug>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QTimer>
#include <QToolButton>
#include <QWidgetAction>

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iplugincontroller.h>

#include <language/interfaces/codecontext.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/interfaces/iquickopen.h>

#include <language/highlighting/colorcache.h>
#include <language/highlighting/configurablecolors.h>
#include <language/highlighting/codehighlighting.h>

#include <language/duchain/duchain.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/use.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/uses.h>
#include <language/duchain/specializationstore.h>
#include <language/duchain/aliasdeclaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <language/duchain/navigation/problemnavigationcontext.h>
#include <language/duchain/navigation/quickopenembeddedwidgetcombiner.h>

#include <language/util/navigationtooltip.h>

#include <shell/problemmodel.h>
#include <shell/problemmodelset.h>

#include <util/texteditorhelpers.h>

#include <sublime/mainwindow.h>

using KTextEditor::Attribute;
using KTextEditor::View;
using namespace KDevelop;

// Helper that follows the QObject::parent() chain, and returns the highest widget that has no parent.
QWidget* masterWidget(QWidget* w)
{
    while (w && w->parent() && qobject_cast<QWidget*>(w->parent()))
        w = qobject_cast<QWidget*>(w->parent());
    return w;
}

namespace {
const unsigned int highlightingTimeout = 150;
const float highlightingZDepth = -5000;
const int maxHistoryLength = 30;

// Helper that determines the context to use for highlighting at a specific position
DUContext* contextForHighlightingAt(const KTextEditor::Cursor& position, TopDUContext* topContext)
{
    DUContext* ctx = topContext->findContextAt(topContext->transformToLocalRevision(position));
    while (ctx && ctx->parentContext()
           && (ctx->type() == DUContext::Template || ctx->type() == DUContext::Helper
               || ctx->localScopeIdentifier().isEmpty())) {
        ctx = ctx->parentContext();
    }
    return ctx;
}

///Duchain must be locked
DUContext* contextAt(const QUrl& url, KTextEditor::Cursor cursor)
{
    TopDUContext* topContext = DUChainUtils::standardContextForUrl(url);
    if (!topContext)
        return nullptr;
    return contextForHighlightingAt(KTextEditor::Cursor(cursor), topContext);
}

DeclarationPointer cursorDeclaration()
{
    KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
    if (!view) {
        return DeclarationPointer();
    }

    DUChainReadLocker lock;

    Declaration* decl =
        DUChainUtils::declarationForDefinition(DUChainUtils::itemUnderCursor(view->document()->url(),
                                                                             KTextEditor::Cursor(
                                                                                 view->cursorPosition())).declaration);
    return DeclarationPointer(decl);
}
}

class ContextBrowserViewFactory
    : public KDevelop::IToolViewFactory
{
public:
    explicit ContextBrowserViewFactory(ContextBrowserPlugin* plugin) : m_plugin(plugin) {}

    QWidget* create(QWidget* parent = nullptr) override
    {
        auto* ret = new ContextBrowserView(m_plugin, parent);
        return ret;
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        return Qt::BottomDockWidgetArea;
    }

    QString id() const override
    {
        return QStringLiteral("org.kdevelop.ContextBrowser");
    }

private:
    ContextBrowserPlugin* m_plugin;
};

KXMLGUIClient* ContextBrowserPlugin::createGUIForMainWindow(Sublime::MainWindow* window)
{
    m_browseManager = new BrowseManager(this);

    KXMLGUIClient* ret = KDevelop::IPlugin::createGUIForMainWindow(window);

    connect(
        ICore::self()->documentController(), &IDocumentController::documentJumpPerformed, this,
        &ContextBrowserPlugin::documentJumpPerformed);

    m_previousButton = new QToolButton();
    m_previousButton->setToolTip(i18nc("@info:tooltip", "Go back in context history"));
    m_previousButton->setAutoRaise(true);
    m_previousButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_previousButton->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    m_previousButton->setEnabled(false);
    m_previousButton->setFocusPolicy(Qt::NoFocus);
    m_previousMenu = new QMenu(m_previousButton);
    m_previousButton->setMenu(m_previousMenu);
    connect(m_previousButton.data(), &QToolButton::clicked, this, &ContextBrowserPlugin::historyPrevious);
    connect(m_previousMenu.data(), &QMenu::aboutToShow, this, &ContextBrowserPlugin::previousMenuAboutToShow);

    m_nextButton = new QToolButton();
    m_nextButton->setToolTip(i18nc("@info:tooltip", "Go forward in context history"));
    m_nextButton->setAutoRaise(true);
    m_nextButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_nextButton->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    m_nextButton->setEnabled(false);
    m_nextButton->setFocusPolicy(Qt::NoFocus);
    m_nextMenu = new QMenu(m_nextButton);
    m_nextButton->setMenu(m_nextMenu);
    connect(m_nextButton.data(), &QToolButton::clicked, this, &ContextBrowserPlugin::historyNext);
    connect(m_nextMenu.data(), &QMenu::aboutToShow, this, &ContextBrowserPlugin::nextMenuAboutToShow);

    auto* quickOpen =
        KDevelop::ICore::self()->pluginController()->extensionForPlugin<IQuickOpen>(QStringLiteral(
                                                                                        "org.kdevelop.IQuickOpen"));

    if (quickOpen) {
        m_outlineLine = quickOpen->createQuickOpenLine(QStringList(), QStringList(i18nc("item quick open item type", "Outline")), IQuickOpen::Outline);
        m_outlineLine->setPlaceholderText(i18nc("@info:placeholder", "Outline"));
        m_outlineLine->setToolTip(i18nc("@info:tooltip", "Navigate outline of active document, click to browse"));
    }

    connect(m_browseManager, &BrowseManager::startDelayedBrowsing,
            this, &ContextBrowserPlugin::startDelayedBrowsing);
    connect(m_browseManager, &BrowseManager::stopDelayedBrowsing,
            this, &ContextBrowserPlugin::stopDelayedBrowsing);
    connect(m_browseManager, &BrowseManager::invokeAction,
            this, &ContextBrowserPlugin::invokeAction);

    m_toolbarWidget = toolbarWidgetForMainWindow(window);
    m_toolbarWidgetLayout = new QHBoxLayout;
    m_toolbarWidgetLayout->setSizeConstraint(QLayout::SetMaximumSize);
    m_previousButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_nextButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_toolbarWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_toolbarWidgetLayout->addWidget(m_previousButton);
    if (m_outlineLine) {
        m_toolbarWidgetLayout->addWidget(m_outlineLine);
        m_outlineLine->setMaximumWidth(600);
        connect(ICore::self()->documentController(), &IDocumentController::documentClosed,
                m_outlineLine.data(), &QLineEdit::clear);
    }
    m_toolbarWidgetLayout->addWidget(m_nextButton);

    if (m_toolbarWidget->children().isEmpty())
        m_toolbarWidget->setLayout(m_toolbarWidgetLayout);

    connect(ICore::self()->documentController(), &IDocumentController::documentActivated,
            this, &ContextBrowserPlugin::documentActivated);

    return ret;
}

void ContextBrowserPlugin::createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile,
                                                      KActionCollection& actions)
{
    xmlFile = QStringLiteral("kdevcontextbrowser.rc");

    QAction* sourceBrowseMode = actions.addAction(QStringLiteral("source_browse_mode"));
    sourceBrowseMode->setText(i18nc("@action", "Source &Browse Mode"));
    sourceBrowseMode->setIcon(QIcon::fromTheme(QStringLiteral("arrow-up")));
    sourceBrowseMode->setCheckable(true);
    connect(sourceBrowseMode, &QAction::triggered, m_browseManager, &BrowseManager::setBrowsing);

    QAction* previousContext = actions.addAction(QStringLiteral("previous_context"));
    previousContext->setText(i18nc("@action", "&Previous Visited Context"));
    previousContext->setIcon(QIcon::fromTheme(QStringLiteral("go-previous-context")));
    actions.setDefaultShortcut(previousContext, Qt::META | Qt::Key_Left);
    QObject::connect(previousContext, &QAction::triggered, this, &ContextBrowserPlugin::previousContextShortcut);

    QAction* nextContext = actions.addAction(QStringLiteral("next_context"));
    nextContext->setText(i18nc("@action", "&Next Visited Context"));
    nextContext->setIcon(QIcon::fromTheme(QStringLiteral("go-next-context")));
    actions.setDefaultShortcut(nextContext, Qt::META | Qt::Key_Right);
    QObject::connect(nextContext, &QAction::triggered, this, &ContextBrowserPlugin::nextContextShortcut);

    QAction* previousUse = actions.addAction(QStringLiteral("previous_use"));
    previousUse->setText(i18nc("@action", "&Previous Use"));
    previousUse->setIcon(QIcon::fromTheme(QStringLiteral("go-previous-use")));
    actions.setDefaultShortcut(previousUse, Qt::META | Qt::SHIFT |  Qt::Key_Left);
    QObject::connect(previousUse, &QAction::triggered, this, &ContextBrowserPlugin::previousUseShortcut);

    QAction* nextUse = actions.addAction(QStringLiteral("next_use"));
    nextUse->setText(i18nc("@action", "&Next Use"));
    nextUse->setIcon(QIcon::fromTheme(QStringLiteral("go-next-use")));
    actions.setDefaultShortcut(nextUse, Qt::META | Qt::SHIFT | Qt::Key_Right);
    QObject::connect(nextUse, &QAction::triggered, this, &ContextBrowserPlugin::nextUseShortcut);

    auto* outline = new QWidgetAction(this);
    outline->setText(i18nc("@action", "Context Browser"));
    QWidget* w = toolbarWidgetForMainWindow(window);
    w->setHidden(false);
    outline->setDefaultWidget(w);
    actions.addAction(QStringLiteral("outline_line"), outline);
    // Add to the actioncollection so one can set global shortcuts for the action
    actions.addAction(QStringLiteral("find_uses"), m_findUses);
}

void ContextBrowserPlugin::nextContextShortcut()
{
    // TODO: cleanup
    historyNext();
}

void ContextBrowserPlugin::previousContextShortcut()
{
    // TODO: cleanup
    historyPrevious();
}

K_PLUGIN_FACTORY_WITH_JSON(ContextBrowserFactory, "kdevcontextbrowser.json", registerPlugin<ContextBrowserPlugin>(); )

ContextBrowserPlugin::ContextBrowserPlugin(QObject* parent, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevcontextbrowser"), parent)
    , m_viewFactory(new ContextBrowserViewFactory(this))
    , m_nextHistoryIndex(0)
    , m_textHintProvider(this)
{
    qRegisterMetaType<KDevelop::IndexedDeclaration>("KDevelop::IndexedDeclaration");

    core()->uiController()->addToolView(i18nc("@title:window", "Code Browser"), m_viewFactory);

    connect(
        core()->documentController(), &IDocumentController::textDocumentCreated, this,
        &ContextBrowserPlugin::textDocumentCreated);
    connect(DUChain::self(), &DUChain::updateReady, this, &ContextBrowserPlugin::updateReady);
    connect(ColorCache::self(), &ColorCache::colorsGotChanged, this, &ContextBrowserPlugin::colorSetupChanged);

    connect(DUChain::self(), &DUChain::declarationSelected,
            this, &ContextBrowserPlugin::declarationSelectedInUI);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, &QTimer::timeout, this, &ContextBrowserPlugin::updateViews);

    //Needed global action for the context-menu extensions
    m_findUses = new QAction(i18nc("@action", "Find Uses"), this);
    connect(m_findUses, &QAction::triggered, this, &ContextBrowserPlugin::findUses);

    const auto documents = core()->documentController()->openDocuments();
    for (KDevelop::IDocument* document : documents) {
        textDocumentCreated(document);
    }
}

ContextBrowserPlugin::~ContextBrowserPlugin()
{
    for (auto* view : std::as_const(m_textHintProvidedViews)) {
        view->unregisterTextHintProvider(&m_textHintProvider);
    }

    ///TODO: QObject inheritance should suffice?
    delete m_nextMenu;
    delete m_previousMenu;
    delete m_toolbarWidgetLayout;

    delete m_previousButton;
    delete m_outlineLine;
    delete m_nextButton;
}

void ContextBrowserPlugin::unload()
{
    core()->uiController()->removeToolView(m_viewFactory);
}

KDevelop::ContextMenuExtension ContextBrowserPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    KDevelop::ContextMenuExtension menuExt = KDevelop::IPlugin::contextMenuExtension(context, parent);

    auto* codeContext = dynamic_cast<KDevelop::DeclarationContext*>(context);

    if (!codeContext)
        return menuExt;

    DUChainReadLocker lock(DUChain::lock());

    if (!codeContext->declaration().data())
        return menuExt;

    menuExt.addAction(KDevelop::ContextMenuExtension::NavigationGroup, m_findUses);

    return menuExt;
}

void ContextBrowserPlugin::showUses(const DeclarationPointer& declaration)
{
    QMetaObject::invokeMethod(this, "showUsesDelayed", Qt::QueuedConnection,
                              Q_ARG(KDevelop::DeclarationPointer, declaration));
}

void ContextBrowserPlugin::showUsesDelayed(const DeclarationPointer& declaration)
{
    DUChainReadLocker lock;

    Declaration* decl = declaration.data();
    if (!decl) {
        return;
    }
    QWidget* toolView = ICore::self()->uiController()->findToolView(i18nc("@title:window", "Code Browser"), m_viewFactory,
                                                                    KDevelop::IUiController::CreateAndRaise);
    if (!toolView) {
        return;
    }
    auto* view = qobject_cast<ContextBrowserView*>(toolView);
    Q_ASSERT(view);
    view->allowLockedUpdate();
    view->setDeclaration(decl, decl->topContext(), true);
    //We may get deleted while the call to acceptLink, so make sure we don't crash in that case
    QPointer<AbstractNavigationWidget> widget = qobject_cast<AbstractNavigationWidget*>(view->navigationWidget());
    if (widget && widget->context()) {
        auto nextContext = widget->context()->execute(
            NavigationAction(declaration, KDevelop::NavigationAction::ShowUses));

        if (widget) {
            widget->setContext(nextContext);
        }
    }
}

void ContextBrowserPlugin::findUses()
{
    showUses(cursorDeclaration());
}

ContextBrowserHintProvider::ContextBrowserHintProvider(ContextBrowserPlugin* plugin)
    : m_plugin(plugin)
{
}

QString ContextBrowserHintProvider::textHint(View* view, const KTextEditor::Cursor& cursor)
{
    m_plugin->m_mouseHoverCursor = KTextEditor::Cursor(cursor);
    if (!view) {
        qCWarning(PLUGIN_CONTEXTBROWSER) << "could not cast to view";
    } else {
        m_plugin->m_mouseHoverDocument = view->document()->url();
        m_plugin->m_updateViews << view;
    }
    m_plugin->m_updateTimer->start(1); // triggers updateViews()

    m_plugin->showToolTip(view, cursor);
    return QString();
}

void ContextBrowserPlugin::stopDelayedBrowsing()
{
    hideToolTip();
}

void ContextBrowserPlugin::invokeAction(int index)
{
    if (!m_currentNavigationWidget)
        return;

    auto navigationWidget = qobject_cast<AbstractNavigationWidget*>(m_currentNavigationWidget);
    if (!navigationWidget)
        return;

    // TODO: Add API in AbstractNavigation{Widget,Context}?
    QMetaObject::invokeMethod(navigationWidget->context().data(), "executeAction", Q_ARG(int, index));
}

void ContextBrowserPlugin::startDelayedBrowsing(KTextEditor::View* view)
{
    if (!m_currentToolTip) {
        showToolTip(view, view->cursorPosition());
    }
}

void ContextBrowserPlugin::hideToolTip()
{
    if (m_currentToolTip) {
        m_currentToolTip->deleteLater();
        m_currentToolTip = nullptr;
        m_currentNavigationWidget = nullptr;
        m_currentToolTipProblems.clear();
        m_currentToolTipDeclaration = {};
    }
}

static QVector<KDevelop::IProblem::Ptr> findProblemsUnderCursor(TopDUContext* topContext, KTextEditor::Cursor position,
                                                                KTextEditor::Range& handleRange)
{
    QVector<KDevelop::IProblem::Ptr> problems;
    handleRange = KTextEditor::Range::invalid();

    const auto modelsData = ICore::self()->languageController()->problemModelSet()->models();
    for (const auto& modelData : modelsData) {
        const auto modelProblems = modelData.model->problems(topContext->url());
        for (const auto& problem : modelProblems) {
            DocumentRange problemRange = problem->finalLocation();
            if (problemRange.contains(position) ||
                (problemRange.isEmpty() && problemRange.boundaryAtCursor(position))) {
                problems += problem;
                // first?
                if (!handleRange.isValid()) {
                    handleRange = problemRange;
                } else {
                    handleRange.confineToRange(problemRange);
                }
            }
        }
    }

    return problems;
}

static QVector<KDevelop::IProblem::Ptr> findProblemsCloseToCursor(const TopDUContext* topContext,
                                                                  KTextEditor::Cursor position,
                                                                  KTextEditor::Range& handleRange)
{
    handleRange = KTextEditor::Range::invalid();

    QVector<KDevelop::IProblem::Ptr> allProblems;
    const auto modelsData = ICore::self()->languageController()->problemModelSet()->models();
    for (const auto& modelData : modelsData) {
        const auto problems = modelData.model->problems(topContext->url());
        allProblems.reserve(allProblems.size() + problems.size());
        for (const auto& problem : problems) {
            allProblems += problem;
        }
    }

    if (allProblems.isEmpty())
        return allProblems;

    std::sort(allProblems.begin(), allProblems.end(),
              [position](const KDevelop::IProblem::Ptr& a, const KDevelop::IProblem::Ptr& b) {
        const auto aRange = a->finalLocation();
        const auto bRange = b->finalLocation();

        const auto aLineDistance = qMin(qAbs(aRange.start().line() - position.line()),
                                        qAbs(aRange.end().line() - position.line()));
        const auto bLineDistance = qMin(qAbs(bRange.start().line() - position.line()),
                                        qAbs(bRange.end().line() - position.line()));
        if (aLineDistance != bLineDistance) {
            return aLineDistance < bLineDistance;
        }

        if (aRange.start().line() == bRange.start().line()) {
            return qAbs(aRange.start().column() - position.column()) <
            qAbs(bRange.start().column() - position.column());
        }
        return qAbs(aRange.end().column() - position.column()) <
        qAbs(bRange.end().column() - position.column());
    });

    QVector<KDevelop::IProblem::Ptr> closestProblems;

    // Show problems, located on the same line
    for (auto& problem : std::as_const(allProblems)) {
        auto r = problem->finalLocation();
        if (r.onSingleLine() && r.start().line() == position.line())
            closestProblems += problem;
        else
            break;
    }

    if (!closestProblems.isEmpty()) {
        auto it = closestProblems.constBegin();
        handleRange = (*it)->finalLocation();
        ++it;
        for (auto end = closestProblems.constEnd(); it != end; ++it) {
            handleRange.confineToRange((*it)->finalLocation());
        }
    }

    return closestProblems;
}

QWidget* ContextBrowserPlugin::navigationWidgetForPosition(KTextEditor::View* view, KTextEditor::Cursor position,
                                                           KTextEditor::Range& itemRange)
{
    QUrl viewUrl = view->document()->url();
    const auto languages = ICore::self()->languageController()->languagesForUrl(viewUrl);

    DUChainReadLocker lock(DUChain::lock());

    for (const auto language : languages) {
        auto widget = language->specialLanguageObjectNavigationWidget(viewUrl, position);
        auto navigationWidget = qobject_cast<AbstractNavigationWidget*>(widget.first);
        if (navigationWidget) {
            itemRange = widget.second;
            return navigationWidget;
        }
    }

    // Find problems under the cursor (first pass)
    TopDUContext* topContext = DUChainUtils::standardContextForUrl(view->document()->url());
    QVector<KDevelop::IProblem::Ptr> problems;
    if (topContext) {
        problems = findProblemsUnderCursor(topContext, position, itemRange);
    }

    // Find decl (declaration) under the cursor
    const auto itemUnderCursor = DUChainUtils::itemUnderCursor(viewUrl, position);
    auto declUnderCursor = itemUnderCursor.declaration;
    Declaration* decl = DUChainUtils::declarationForDefinition(declUnderCursor);
    if (decl && decl->kind() == Declaration::Alias) {
        auto* alias = dynamic_cast<AliasDeclaration*>(decl);
        Q_ASSERT(alias);
        decl = alias->aliasedDeclaration().declaration();
    }

    // Return nullptr if the found problems / decl are already being shown in the tool tip currently.
    if (m_currentToolTip &&
        problems == m_currentToolTipProblems &&
        IndexedDeclaration(decl) == m_currentToolTipDeclaration) {
        return nullptr;
    }

    // Create a widget for problems, if any have been found.
    AbstractNavigationWidget* problemWidget = nullptr;
    if (!problems.isEmpty()) {
        problemWidget = new AbstractNavigationWidget;
        auto context = new ProblemNavigationContext(problems);
        context->setTopContext(TopDUContextPointer(topContext));
        problemWidget->setContext(NavigationContextPointer(context));
    }

    // Let the context create a widget for decl, if there is one.
    // Note that createNavigationWidget() might also return nullptr for a valid decl however.
    AbstractNavigationWidget* declWidget = nullptr;
    if (decl) {
        if (itemRange.isValid()) {
            itemRange.expandToRange(itemUnderCursor.range);
        } else {
            itemRange = itemUnderCursor.range;
        }
        declWidget = decl->context()->createNavigationWidget(decl, DUChainUtils::standardContextForUrl(viewUrl));
    }

    // If at least one widget was created for problems or decl, show it.
    // If two widgets were created, combine them.
    if (problemWidget || declWidget) {
        // Remember current tool tip state.
        m_currentToolTipProblems = problems;
        m_currentToolTipDeclaration = IndexedDeclaration(decl);

        if (problemWidget && declWidget) {
            auto* combinedWidget = new QuickOpenEmbeddedWidgetCombiner;
            combinedWidget->layout()->addWidget(problemWidget);
            combinedWidget->layout()->addWidget(declWidget);
            return combinedWidget;
        }
        if (problemWidget) {
            return problemWidget;
        }
        return declWidget;
    }

    // Nothing has been found so far which created a widget.
    // Thus, find the closest problem to the cursor in a second pass.
    if (topContext) {
        problems = findProblemsCloseToCursor(topContext, position, itemRange);
        if (!problems.isEmpty()) {
            // Return nullptr if the correct contents are already being shown in the tool tip currently.
            if (m_currentToolTip &&
                problems == m_currentToolTipProblems &&
                !m_currentToolTipDeclaration.isValid()) {
                return nullptr;
            }

            // Remember current tool tip state.
            m_currentToolTipProblems = problems;
            m_currentToolTipDeclaration = {};

            auto widget = new AbstractNavigationWidget;
            // since the problem is not under cursor: show location
            widget->setContext(NavigationContextPointer(new ProblemNavigationContext(problems,
                                                                                     ProblemNavigationContext::
                                                                                     ShowLocation)));
            return widget;
        }
    }

    // Nothing to show has been found under or next to the cursor, so hide the tool tip (if visible).
    hideToolTip();
    return nullptr;
}

void ContextBrowserPlugin::showToolTip(KTextEditor::View* view, KTextEditor::Cursor position)
{
    ContextBrowserView* contextView = browserViewForWidget(view);
    if (contextView && contextView->isVisible() && !contextView->isLocked())
        return; // If the context-browser view is visible, it will care about updating by itself

    KTextEditor::Range itemRange = KTextEditor::Range::invalid();
    auto navigationWidget = navigationWidgetForPosition(view, position, itemRange);
    if (navigationWidget) {
        // If we have an invisible context-view, assign the tooltip navigation-widget to it.
        // If the user makes the context-view visible, it will instantly contain the correct widget.
        if (contextView && !contextView->isLocked())
            contextView->setNavigationWidget(navigationWidget);

        if (m_currentToolTip) {
            m_currentToolTip->deleteLater();
            m_currentToolTip = nullptr;
            m_currentNavigationWidget = nullptr;
        }

        auto* tooltip =
            new KDevelop::NavigationToolTip(view, view->mapToGlobal(view->cursorToCoordinate(position)) + QPoint(20,
                                                                                                                 40),
                                            navigationWidget);
        if (!itemRange.isValid()) {
            qCWarning(PLUGIN_CONTEXTBROWSER) << "Got navigationwidget with invalid itemrange";
            itemRange = KTextEditor::Range(position, 0);
        }

        tooltip->setHandleRect(KTextEditorHelpers::itemBoundingRect(view, itemRange));
        tooltip->resize(navigationWidget->sizeHint() + QSize(10, 10));
        QObject::connect(view, &KTextEditor::View::verticalScrollPositionChanged,
                         this, &ContextBrowserPlugin::hideToolTip);
        QObject::connect(view, &KTextEditor::View::horizontalScrollPositionChanged,
                         this, &ContextBrowserPlugin::hideToolTip);
        qCDebug(PLUGIN_CONTEXTBROWSER) << "tooltip size" << tooltip->size();
        m_currentToolTip = tooltip;
        m_currentNavigationWidget = navigationWidget;
        ActiveToolTip::showToolTip(tooltip);

        if (!navigationWidget->property("DoNotCloseOnCursorMove").toBool()) {
            connect(view, &View::cursorPositionChanged,
                    this, &ContextBrowserPlugin::hideToolTip, Qt::UniqueConnection);
        } else {
            disconnect(view, &View::cursorPositionChanged,
                       this, &ContextBrowserPlugin::hideToolTip);
        }
    } else {
        qCDebug(PLUGIN_CONTEXTBROWSER) << "not showing tooltip, no navigation-widget";
    }
}

void ContextBrowserPlugin::clearMouseHover()
{
    m_mouseHoverCursor = KTextEditor::Cursor::invalid();
    m_mouseHoverDocument.clear();
}

Attribute::Ptr ContextBrowserPlugin::highlightedUseAttribute() const
{
    if (!m_highlightAttribute) {
        m_highlightAttribute = ColorCache::self()->defaultColors()->attribute(CodeHighlightingType::HighlightUses);
    }
    return m_highlightAttribute;
}

void ContextBrowserPlugin::colorSetupChanged()
{
    m_highlightAttribute = Attribute::Ptr();
}

Attribute::Ptr ContextBrowserPlugin::highlightedSpecialObjectAttribute() const
{
    return highlightedUseAttribute();
}

void ContextBrowserPlugin::addHighlight(View* view, KDevelop::Declaration* decl)
{
    if (!view || !decl) {
        qCDebug(PLUGIN_CONTEXTBROWSER) << "invalid view/declaration";
        return;
    }

    ViewHighlights& highlights(m_highlightedRanges[view]);

    KDevelop::DUChainReadLocker lock;

    // Highlight the declaration
    highlights.highlights << decl->createRangeMoving();
    highlights.highlights.back()->setAttribute(highlightedUseAttribute());
    highlights.highlights.back()->setZDepth(highlightingZDepth);

    // Highlight uses
    {
        const auto currentRevisionUses = decl->usesCurrentRevision();
        for (auto fileIt = currentRevisionUses.constBegin(); fileIt != currentRevisionUses.constEnd(); ++fileIt) {
            const auto& document = fileIt.key();
            const auto& documentUses = fileIt.value();
            for (auto& use : documentUses) {
                highlights.highlights << PersistentMovingRange::Ptr(new PersistentMovingRange(use, document));
                highlights.highlights.back()->setAttribute(highlightedUseAttribute());
                highlights.highlights.back()->setZDepth(highlightingZDepth);
            }
        }
    }

    if (auto* def = FunctionDefinition::definition(decl)) {
        highlights.highlights << def->createRangeMoving();
        highlights.highlights.back()->setAttribute(highlightedUseAttribute());
        highlights.highlights.back()->setZDepth(highlightingZDepth);
    }
}

Declaration* ContextBrowserPlugin::findDeclaration(View* view, const KTextEditor::Cursor& position, bool mouseHighlight)
{
    Q_UNUSED(mouseHighlight);
    ENSURE_CHAIN_READ_LOCKED

    Declaration* foundDeclaration = nullptr;
    if (m_useDeclaration.data()) {
        foundDeclaration = m_useDeclaration.data();
    } else {
        //If we haven't found a special language object, search for a use/declaration and eventually highlight it
        foundDeclaration =
            DUChainUtils::declarationForDefinition(DUChainUtils::itemUnderCursor(
                                                       view->document()->url(), position).declaration);
        if (foundDeclaration && foundDeclaration->kind() == Declaration::Alias) {
            auto* alias = dynamic_cast<AliasDeclaration*>(foundDeclaration);
            Q_ASSERT(alias);
            foundDeclaration = alias->aliasedDeclaration().declaration();
        }
    }
    return foundDeclaration;
}

ContextBrowserView* ContextBrowserPlugin::browserViewForWidget(QWidget* widget) const
{
    const auto masterWidgetOfWidget = masterWidget(widget);
    auto it = std::find_if(m_views.begin(), m_views.end(), [&](ContextBrowserView* contextView) {
        return (masterWidget(contextView) == masterWidgetOfWidget);
    });

    return (it != m_views.end()) ? *it : nullptr;
}

void ContextBrowserPlugin::updateForView(View* view)
{
    bool allowHighlight = true;
    if (view->selection()) {
        // If something is selected, we unhighlight everything, so that we don't conflict with the
        // kate plugin that highlights occurrences of the selected string, and also to reduce the
        // overall amount of concurrent highlighting.
        allowHighlight = false;
    }

    if (m_highlightedRanges[view].keep) {
        m_highlightedRanges[view].keep = false;
        return;
    }

    // Clear all highlighting
    m_highlightedRanges.clear();

    // Re-highlight
    ViewHighlights& highlights = m_highlightedRanges[view];

    QUrl url = view->document()->url();
    IDocument* activeDoc = core()->documentController()->activeDocument();

    bool mouseHighlight = (url == m_mouseHoverDocument) && (m_mouseHoverCursor.isValid());
    bool shouldUpdateBrowser =
        (mouseHighlight ||
         (view == ICore::self()->documentController()->activeTextDocumentView() && activeDoc &&
          activeDoc->textDocument() == view->document()));

    KTextEditor::Cursor highlightPosition;
    if (mouseHighlight)
        highlightPosition = m_mouseHoverCursor;
    else
        highlightPosition = KTextEditor::Cursor(view->cursorPosition());

    ///Pick a language
    ILanguageSupport* const language = ICore::self()->languageController()->languagesForUrl(url).value(0);
    if (!language) {
        qCDebug(PLUGIN_CONTEXTBROWSER) << "found no language for document" << url;
        return;
    }

    ///Check whether there is a special language object to highlight (for example a macro)

    KTextEditor::Range specialRange = language->specialLanguageObjectRange(url, highlightPosition);
    ContextBrowserView* updateBrowserView = shouldUpdateBrowser ?  browserViewForWidget(view) : nullptr;

    if (specialRange.isValid()) {
        // Highlight a special language object
        if (allowHighlight) {
            highlights.highlights <<
                PersistentMovingRange::Ptr(new PersistentMovingRange(specialRange, IndexedString(url)));
            highlights.highlights.back()->setAttribute(highlightedSpecialObjectAttribute());
            highlights.highlights.back()->setZDepth(highlightingZDepth);
        }
        if (updateBrowserView)
            updateBrowserView->setSpecialNavigationWidget(language->specialLanguageObjectNavigationWidget(url,
                                                                                                          highlightPosition).first);
    } else {
        KDevelop::DUChainReadLocker lock(DUChain::lock(), 100);
        if (!lock.locked()) {
            qCDebug(PLUGIN_CONTEXTBROWSER) << "Failed to lock du-chain in time";
            return;
        }

        TopDUContext* topContext = DUChainUtils::standardContextForUrl(view->document()->url());
        if (!topContext)
            return;
        DUContext* ctx = contextForHighlightingAt(highlightPosition, topContext);
        if (!ctx)
            return;

        //Only update the history if this context is around the text cursor
        if (core()->documentController()->activeDocument() &&
            highlightPosition == KTextEditor::Cursor(view->cursorPosition()) &&
            view->document() == core()->documentController()->activeDocument()->textDocument()) {
            updateHistory(ctx, highlightPosition);
        }

        Declaration* foundDeclaration = findDeclaration(view, highlightPosition, mouseHighlight);

        if (foundDeclaration) {
            m_lastHighlightedDeclaration = highlights.declaration = IndexedDeclaration(foundDeclaration);
            if (allowHighlight)
                addHighlight(view, foundDeclaration);

            if (updateBrowserView)
                updateBrowserView->setDeclaration(foundDeclaration, topContext);
        } else {
            if (updateBrowserView)
                updateBrowserView->setContext(ctx);
        }
    }
}

void ContextBrowserPlugin::updateViews()
{
    for (View* view : std::as_const(m_updateViews)) {
        updateForView(view);
    }

    m_updateViews.clear();
    m_useDeclaration = IndexedDeclaration();
}

void ContextBrowserPlugin::declarationSelectedInUI(const DeclarationPointer& decl)
{
    m_useDeclaration = IndexedDeclaration(decl.data());
    KTextEditor::View* view = core()->documentController()->activeTextDocumentView();
    if (view)
        m_updateViews << view;

    if (!m_updateViews.isEmpty())
        m_updateTimer->start(highlightingTimeout); // triggers updateViews()
}

void ContextBrowserPlugin::updateReady(const IndexedString& file, const ReferencedTopDUContext& /*topContext*/)
{
    const auto url = file.toUrl();
    for (QMap<View*, ViewHighlights>::iterator it = m_highlightedRanges.begin(); it != m_highlightedRanges.end();
         ++it) {
        if (it.key()->document()->url() == url) {
            if (!m_updateViews.contains(it.key())) {
                qCDebug(PLUGIN_CONTEXTBROWSER) << "adding view for update";
                m_updateViews << it.key();

                // Don't change the highlighted declaration after finished parse-jobs
                (*it).keep = true;
            }
        }
    }

    if (!m_updateViews.isEmpty())
        m_updateTimer->start(highlightingTimeout);
}

void ContextBrowserPlugin::textDocumentCreated(KDevelop::IDocument* document)
{
    Q_ASSERT(document->textDocument());

    connect(document->textDocument(), &KTextEditor::Document::viewCreated, this, &ContextBrowserPlugin::viewCreated);

    const auto views = document->textDocument()->views();
    for (View* view : views) {
        viewCreated(document->textDocument(), view);
    }
}

void ContextBrowserPlugin::documentActivated(IDocument* doc)
{
    if (m_outlineLine)
        m_outlineLine->clear();

    if (View* view = doc->activeTextView()) {
        cursorPositionChanged(view, view->cursorPosition());
    }
}

void ContextBrowserPlugin::viewDestroyed(QObject* obj)
{
    m_highlightedRanges.remove(static_cast<KTextEditor::View*>(obj));
    m_updateViews.remove(static_cast<View*>(obj));
    m_textHintProvidedViews.removeOne(static_cast<KTextEditor::View*>(obj));
}

void ContextBrowserPlugin::selectionChanged(View* view)
{
    clearMouseHover();
    m_updateViews.insert(view);
    m_updateTimer->start(highlightingTimeout / 2); // triggers updateViews()
}

void ContextBrowserPlugin::cursorPositionChanged(View* view, const KTextEditor::Cursor& newPosition)
{
    const bool atInsertPosition = (view->document() == m_lastInsertionDocument && newPosition == m_lastInsertionPos);
    if (atInsertPosition) {
        //Do not update the highlighting while typing
        m_lastInsertionDocument = nullptr;
        m_lastInsertionPos = KTextEditor::Cursor();
    }

    const auto viewHighlightsIt = m_highlightedRanges.find(view);
    if (viewHighlightsIt != m_highlightedRanges.end()) {
        viewHighlightsIt->keep = atInsertPosition;
    }

    clearMouseHover();
    m_updateViews.insert(view);
    m_updateTimer->start(highlightingTimeout / 2); // triggers updateViews()
}

void ContextBrowserPlugin::textInserted(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor,
                                        const QString& text)
{
    m_lastInsertionDocument = doc;
    m_lastInsertionPos = cursor + KTextEditor::Cursor(0, text.size());
}

void ContextBrowserPlugin::viewCreated(KTextEditor::Document*, View* v)
{
    disconnect(v, &View::cursorPositionChanged, this, &ContextBrowserPlugin::cursorPositionChanged); ///Just to make sure that multiple connections don't happen
    connect(v, &View::cursorPositionChanged, this, &ContextBrowserPlugin::cursorPositionChanged);
    connect(v, &View::destroyed, this, &ContextBrowserPlugin::viewDestroyed);
    disconnect(v->document(), &KTextEditor::Document::textInserted, this, &ContextBrowserPlugin::textInserted);
    connect(v->document(), &KTextEditor::Document::textInserted, this, &ContextBrowserPlugin::textInserted);
    disconnect(v, &View::selectionChanged, this, &ContextBrowserPlugin::selectionChanged);

    if (m_textHintProvidedViews.contains(v)) {
        return;
    }
    v->setTextHintDelay(highlightingTimeout);
    v->registerTextHintProvider(&m_textHintProvider);
    m_textHintProvidedViews.append(v);
}

void ContextBrowserPlugin::registerToolView(ContextBrowserView* view)
{
    m_views << view;
}

void ContextBrowserPlugin::previousUseShortcut()
{
    switchUse(false);
}

void ContextBrowserPlugin::nextUseShortcut()
{
    switchUse(true);
}

KTextEditor::Range cursorToRange(KTextEditor::Cursor cursor)
{
    return KTextEditor::Range(cursor, cursor);
}

void ContextBrowserPlugin::switchUse(bool forward)
{
    View* view = core()->documentController()->activeTextDocumentView();
    if (view) {
        KTextEditor::Document* doc = view->document();
        KDevelop::DUChainReadLocker lock(DUChain::lock());
        KDevelop::TopDUContext* chosen = DUChainUtils::standardContextForUrl(doc->url());

        if (chosen) {
            KTextEditor::Cursor cCurrent(view->cursorPosition());
            KDevelop::CursorInRevision c = chosen->transformToLocalRevision(cCurrent);

            Declaration* decl = nullptr;
            //If we have a locked declaration, use that for jumping
            for (ContextBrowserView* view : std::as_const(m_views)) {
                decl = view->lockedDeclaration().data(); ///@todo Somehow match the correct context-browser view if there is multiple
                if (decl)
                    break;
            }

            if (!decl) //Try finding a declaration under the cursor
                decl = DUChainUtils::itemUnderCursor(doc->url(), cCurrent).declaration;

            if (decl && decl->kind() == Declaration::Alias) {
                auto* alias = dynamic_cast<AliasDeclaration*>(decl);
                Q_ASSERT(alias);
                decl = alias->aliasedDeclaration().declaration();
            }

            if (decl) {
                Declaration* target = nullptr;
                if (forward)
                    //Try jumping from definition to declaration
                    target = DUChainUtils::declarationForDefinition(decl, chosen);
                else if (decl->url().toUrl() == doc->url() && decl->range().contains(c))
                    //Try jumping from declaration to definition
                    target = FunctionDefinition::definition(decl);

                if (target && target != decl) {
                    KTextEditor::Cursor jumpTo = target->rangeInCurrentRevision().start();
                    QUrl document = target->url().toUrl();
                    lock.unlock();
                    core()->documentController()->openDocument(document, cursorToRange(jumpTo));
                    return;
                } else {
                    //Always work with the declaration instead of the definition
                    decl = DUChainUtils::declarationForDefinition(decl, chosen);
                }
            }

            if (!decl) {
                //Pick the last use we have highlighted
                decl = m_lastHighlightedDeclaration.data();
            }

            if (decl) {
                KDevVarLengthArray<IndexedTopDUContext> usingFiles = DUChain::uses()->uses(decl->id());

                if (DUChainUtils::contextHasUse(decl->topContext(),
                                                decl) && usingFiles.indexOf(decl->topContext()) == -1)
                    usingFiles.insert(usingFiles.begin(), decl->topContext());

                if (decl->range().contains(c) && decl->url() == chosen->url()) {
                    //The cursor is directly on the declaration. Jump to the first or last use.
                    if (!usingFiles.isEmpty()) {
                        TopDUContext* top = (forward ? usingFiles[0] : usingFiles.back()).data();
                        if (top) {
                            QVector<RangeInRevision> useRanges = allUses(top, decl, true);
                            std::sort(useRanges.begin(), useRanges.end());
                            if (!useRanges.isEmpty()) {
                                QUrl url = top->url().toUrl();
                                KTextEditor::Range selectUse = chosen->transformFromLocalRevision(
                                    forward ? useRanges.first() : useRanges.back());
                                lock.unlock();
                                core()->documentController()->openDocument(url, cursorToRange(selectUse.start()));
                            }
                        }
                    }
                    return;
                }
                //Check whether we are within a use
                QVector<RangeInRevision> localUses = allUses(chosen, decl, true);
                std::sort(localUses.begin(), localUses.end());
                for (int a = 0; a < localUses.size(); ++a) {
                    int nextUse = (forward ? a + 1 : a - 1);
                    bool pick = localUses[a].contains(c);

                    if (!pick && forward && a + 1 < localUses.size() && localUses[a].end <= c &&
                        localUses[a + 1].start > c) {
                        //Special case: We aren't on a use, but we are jumping forward, and are behind this and the next use
                        pick = true;
                    }
                    if (!pick && !forward && a - 1 >= 0 && c < localUses[a].start && c >= localUses[a - 1].end) {
                        //Special case: We aren't on a use, but we are jumping backward, and are in front of this use, but behind the previous one
                        pick = true;
                    }
                    if (!pick && a == 0 && c < localUses[a].start) {
                        if (!forward) {
                            //Will automatically jump to previous file
                        } else {
                            nextUse = 0; //We are before the first use, so jump to it.
                        }
                        pick = true;
                    }
                    if (!pick && a == localUses.size() - 1 && c >= localUses[a].end) {
                        if (forward) {
                            //Will automatically jump to next file
                        } else { //We are behind the last use, but moving backward. So pick the last use.
                            nextUse = a;
                        }
                        pick = true;
                    }

                    if (pick) {
                        //Make sure we end up behind the use
                        if (nextUse != a)
                            while (forward && nextUse < localUses.size() &&
                                   (localUses[nextUse].start <= localUses[a].end || localUses[nextUse].isEmpty()))
                                ++nextUse;

                        //Make sure we end up before the use
                        if (nextUse != a)
                            while (!forward && nextUse >= 0 &&
                                   (localUses[nextUse].start >= localUses[a].start || localUses[nextUse].isEmpty()))
                                --nextUse;
                        //Jump to the next use

                        qCDebug(PLUGIN_CONTEXTBROWSER) << "count of uses:" << localUses.size() << "nextUse" << nextUse;

                        if (nextUse < 0 || nextUse == localUses.size()) {
                            qCDebug(PLUGIN_CONTEXTBROWSER) << "jumping to next file";
                            //Jump to the first use in the next using top-context
                            int indexInFiles = usingFiles.indexOf(chosen);
                            if (indexInFiles != -1) {
                                int nextFile = (forward ? indexInFiles + 1 : indexInFiles - 1);
                                qCDebug(PLUGIN_CONTEXTBROWSER) << "current file" << indexInFiles << "nextFile" <<
                                    nextFile;

                                if (nextFile < 0 || nextFile >= usingFiles.size()) {
                                    //Open the declaration, or the definition
                                    if (nextFile >= usingFiles.size()) {
                                        Declaration* definition = FunctionDefinition::definition(decl);
                                        if (definition)
                                            decl = definition;
                                    }
                                    QUrl u = decl->url().toUrl();
                                    KTextEditor::Range range = decl->rangeInCurrentRevision();
                                    range.setEnd(range.start());
                                    lock.unlock();
                                    core()->documentController()->openDocument(u, range);
                                    return;
                                } else {
                                    TopDUContext* nextTop = usingFiles[nextFile].data();

                                    QUrl u = nextTop->url().toUrl();

                                    QVector<RangeInRevision> nextTopUses = allUses(nextTop, decl, true);
                                    std::sort(nextTopUses.begin(), nextTopUses.end());

                                    if (!nextTopUses.isEmpty()) {
                                        KTextEditor::Range range =  chosen->transformFromLocalRevision(
                                            forward ? nextTopUses.front() : nextTopUses.back());
                                        range.setEnd(range.start());
                                        lock.unlock();
                                        core()->documentController()->openDocument(u, range);
                                    }
                                    return;
                                }
                            } else {
                                qCDebug(PLUGIN_CONTEXTBROWSER) << "not found own file in use list";
                            }
                        } else {
                            QUrl url = chosen->url().toUrl();
                            KTextEditor::Range range = chosen->transformFromLocalRevision(localUses[nextUse]);
                            range.setEnd(range.start());
                            lock.unlock();
                            core()->documentController()->openDocument(url, range);
                            return;
                        }
                    }
                }
            }
        }
    }
}

void ContextBrowserPlugin::unRegisterToolView(ContextBrowserView* view)
{
    m_views.removeAll(view);
}

// history browsing

QWidget* ContextBrowserPlugin::toolbarWidgetForMainWindow(Sublime::MainWindow* window)
{
    //TODO: support multiple windows (if that ever gets revived)
    if (!m_toolbarWidget) {
        m_toolbarWidget = new QWidget(window);
    }
    return m_toolbarWidget;
}

void ContextBrowserPlugin::documentJumpPerformed(KDevelop::IDocument* newDocument,
                                                 const KTextEditor::Cursor& newCursor,
                                                 KDevelop::IDocument* previousDocument,
                                                 const KTextEditor::Cursor& previousCursor)
{
    DUChainReadLocker lock(DUChain::lock());

    /*TODO: support multiple windows if that ever gets revived
       if(newDocument && newDocument->textDocument() && newDocument->textDocument()->activeView() && masterWidget(newDocument->textDocument()->activeView()) != masterWidget(this))
        return;
     */

    if (previousDocument && previousCursor.isValid()) {
        qCDebug(PLUGIN_CONTEXTBROWSER) << "updating jump source";
        DUContext* context = contextAt(previousDocument->url(), previousCursor);
        if (context) {
            updateHistory(context, KTextEditor::Cursor(previousCursor), true);
        } else {
            //We just want this place in the history
            m_history.resize(m_nextHistoryIndex); // discard forward history
            m_history.append(HistoryEntry(DocumentCursor(IndexedString(previousDocument->url()),
                                                         KTextEditor::Cursor(previousCursor))));
            ++m_nextHistoryIndex;
        }
    }
    qCDebug(PLUGIN_CONTEXTBROWSER) << "new doc: " << newDocument << " new cursor: " << newCursor;
    if (newDocument && newCursor.isValid()) {
        qCDebug(PLUGIN_CONTEXTBROWSER) << "updating jump target";
        DUContext* context = contextAt(newDocument->url(), newCursor);
        if (context) {
            updateHistory(context, KTextEditor::Cursor(newCursor), true);
        } else {
            //We just want this place in the history
            m_history.resize(m_nextHistoryIndex); // discard forward history
            m_history.append(HistoryEntry(DocumentCursor(IndexedString(newDocument->url()),
                                                         KTextEditor::Cursor(newCursor))));
            ++m_nextHistoryIndex;
            if (m_outlineLine)
                m_outlineLine->clear();
        }
    }
}

void ContextBrowserPlugin::updateButtonState()
{
    m_nextButton->setEnabled(m_nextHistoryIndex < m_history.size());
    m_previousButton->setEnabled(m_nextHistoryIndex >= 2);
}

void ContextBrowserPlugin::historyNext()
{
    if (m_nextHistoryIndex >= m_history.size()) {
        return;
    }
    openDocument(m_nextHistoryIndex); // opening the document at given position
                                      // will update the widget for us
    ++m_nextHistoryIndex;
    updateButtonState();
}

void ContextBrowserPlugin::openDocument(int historyIndex)
{
    Q_ASSERT_X(historyIndex >= 0, "openDocument", "negative history index");
    Q_ASSERT_X(historyIndex < m_history.size(), "openDocument", "history index out of range");
    DocumentCursor c = m_history[historyIndex].computePosition();
    if (c.isValid() && !c.document.str().isEmpty()) {
        disconnect(
            ICore::self()->documentController(), &IDocumentController::documentJumpPerformed, this,
            &ContextBrowserPlugin::documentJumpPerformed);

        ICore::self()->documentController()->openDocument(c.document.toUrl(), c);

        connect(
            ICore::self()->documentController(), &IDocumentController::documentJumpPerformed, this,
            &ContextBrowserPlugin::documentJumpPerformed);

        KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
        updateDeclarationListBox(m_history[historyIndex].context.data());
    }
}

void ContextBrowserPlugin::historyPrevious()
{
    if (m_nextHistoryIndex < 2) {
        return;
    }
    --m_nextHistoryIndex;
    openDocument(m_nextHistoryIndex - 1); // opening the document at given position
                                          // will update the widget for us
    updateButtonState();
}

QString ContextBrowserPlugin::actionTextFor(int historyIndex) const
{
    const HistoryEntry& entry = m_history.at(historyIndex);
    QString actionText = entry.context.data() ? entry.context.data()->scopeIdentifier(true).toString() : QString();
    if (actionText.isEmpty())
        actionText = entry.alternativeString;
    if (actionText.isEmpty())
        actionText = QStringLiteral("<unnamed>");
    actionText += QLatin1String(" @ ");
    QString fileName = entry.absoluteCursorPosition.document.toUrl().fileName();
    actionText += QStringLiteral("%1:%2").arg(fileName).arg(entry.absoluteCursorPosition.line() + 1);
    return actionText;
}

/*
   inline QDebug operator<<(QDebug debug, const ContextBrowserPlugin::HistoryEntry &he)
   {
    DocumentCursor c = he.computePosition();
    debug << "\n\tHistoryEntry " << c.line << " " << c.document.str();
    return debug;
   }
 */

void ContextBrowserPlugin::nextMenuAboutToShow()
{
    QList<int> indices;
    indices.reserve(m_history.size() - m_nextHistoryIndex);
    for (int a = m_nextHistoryIndex; a < m_history.size(); ++a) {
        indices << a;
    }

    fillHistoryPopup(m_nextMenu, indices);
}

void ContextBrowserPlugin::previousMenuAboutToShow()
{
    QList<int> indices;
    indices.reserve(m_nextHistoryIndex - 1);
    for (int a = m_nextHistoryIndex - 2; a >= 0; --a) {
        indices << a;
    }

    fillHistoryPopup(m_previousMenu, indices);
}

void ContextBrowserPlugin::fillHistoryPopup(QMenu* menu, const QList<int>& historyIndices)
{
    menu->clear();
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    for (int index : historyIndices) {
        auto* action = new QAction(actionTextFor(index), menu);
        action->setData(index);
        menu->addAction(action);
        connect(action, &QAction::triggered, this, &ContextBrowserPlugin::actionTriggered);
    }
}

bool ContextBrowserPlugin::isPreviousEntry(KDevelop::DUContext* context,
                                           const KTextEditor::Cursor& /*position*/) const
{
    if (m_nextHistoryIndex == 0)
        return false;
    Q_ASSERT(m_nextHistoryIndex <= m_history.count());
    const HistoryEntry& he = m_history.at(m_nextHistoryIndex - 1);
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());   // is this necessary??
    Q_ASSERT(context);
    return IndexedDUContext(context) == he.context;
}

void ContextBrowserPlugin::updateHistory(KDevelop::DUContext* context, const KTextEditor::Cursor& position, bool force)
{
    qCDebug(PLUGIN_CONTEXTBROWSER) << "updating history";

    if (m_outlineLine && m_outlineLine->isVisible())
        updateDeclarationListBox(context);

    if (!context || (!context->owner() && !force)) {
        return; //Only add history-entries for contexts that have owners, which in practice should be functions and classes
                //This keeps the history cleaner
    }

    if (isPreviousEntry(context, position)) {
        if (m_nextHistoryIndex) {
            HistoryEntry& he = m_history[m_nextHistoryIndex - 1];
            he.setCursorPosition(position);
        }
        return;
    } else { // Append new history entry
        m_history.resize(m_nextHistoryIndex); // discard forward history
        m_history.append(HistoryEntry(IndexedDUContext(context), position));
        ++m_nextHistoryIndex;

        updateButtonState();
        if (m_history.size() > (maxHistoryLength + 5)) {
            m_history.remove(0, m_history.size() - maxHistoryLength);
            m_nextHistoryIndex = m_history.size();
        }
    }
}

void ContextBrowserPlugin::updateDeclarationListBox(DUContext* context)
{
    if (!context || !context->owner()) {
        qCDebug(PLUGIN_CONTEXTBROWSER) << "not updating box";
        m_listUrl = IndexedString(); ///@todo Compute the context in the document here
        if (m_outlineLine)
            m_outlineLine->clear();
        return;
    }

    Declaration* decl = context->owner();

    m_listUrl = context->url();

    Declaration* specialDecl = SpecializationStore::self().applySpecialization(decl, decl->topContext());

    FunctionType::Ptr function = specialDecl->type<FunctionType>();
    QString text = specialDecl->qualifiedIdentifier().toString();
    if (function)
        text += function->partToString(KDevelop::FunctionType::SignatureArguments);

    if (m_outlineLine && !m_outlineLine->hasFocus()) {
        m_outlineLine->setText(text);
        m_outlineLine->setCursorPosition(0);
    }

    qCDebug(PLUGIN_CONTEXTBROWSER) << "updated" << text;
}

void ContextBrowserPlugin::actionTriggered()
{
    auto* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    Q_ASSERT(action->data().typeId() == qMetaTypeId<int>());
    int historyPosition = action->data().toInt();
    // qCDebug(PLUGIN_CONTEXTBROWSER) << "history pos" << historyPosition << m_history.size() << m_history;
    if (historyPosition >= 0 && historyPosition < m_history.size()) {
        m_nextHistoryIndex = historyPosition + 1;
        openDocument(historyPosition);
        updateButtonState();
    }
}

void ContextBrowserPlugin::doNavigate(NavigationActionType action)
{
    auto* view = qobject_cast<KTextEditor::View*>(sender());
    if (!view) {
        qCWarning(PLUGIN_CONTEXTBROWSER) << "sender is not a view";
        return;
    }
    if (view->isCompletionActive())
        return; // If code completion is active, the actions should be handled by the completion widget

    QWidget* widget = m_currentNavigationWidget.data();

    if (!widget || !widget->isVisible()) {
        ContextBrowserView* contextView = browserViewForWidget(view);
        if (contextView)
            widget = contextView->navigationWidget();
    }

    if (auto* navWidget = dynamic_cast<QuickOpenEmbeddedWidgetInterface*>(widget)) {
        switch (action) {
        case Accept:
            navWidget->accept();
            break;
        case Back:
            navWidget->back();
            break;
        case Left:
            navWidget->previous();
            break;
        case Right:
            navWidget->next();
            break;
        case Up:
            navWidget->up();
            break;
        case Down:
            navWidget->down();
            break;
        }
    }
}

void ContextBrowserPlugin::navigateAccept()
{
    doNavigate(Accept);
}

void ContextBrowserPlugin::navigateBack()
{
    doNavigate(Back);
}

void ContextBrowserPlugin::navigateDown()
{
    doNavigate(Down);
}

void ContextBrowserPlugin::navigateLeft()
{
    doNavigate(Left);
}

void ContextBrowserPlugin::navigateRight()
{
    doNavigate(Right);
}

void ContextBrowserPlugin::navigateUp()
{
    doNavigate(Up);
}

//BEGIN HistoryEntry
ContextBrowserPlugin::HistoryEntry::HistoryEntry(const KDevelop::DocumentCursor& pos)
    : absoluteCursorPosition(pos)
{
}

ContextBrowserPlugin::HistoryEntry::HistoryEntry(IndexedDUContext ctx,
                                                 const KTextEditor::Cursor& cursorPosition) : context(ctx)
{
    //Use a position relative to the context
    setCursorPosition(cursorPosition);
    if (ctx.data())
        alternativeString = ctx.data()->scopeIdentifier(true).toString();
    if (!alternativeString.isEmpty())
        alternativeString += i18n("(changed)");     //This is used when the context was deleted in between
}

DocumentCursor ContextBrowserPlugin::HistoryEntry::computePosition() const
{
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    DocumentCursor ret;
    if (context.data()) {
        ret = DocumentCursor(context.data()->url(), relativeCursorPosition);
        ret.setLine(ret.line() + context.data()->range().start.line);
    } else {
        ret = absoluteCursorPosition;
    }
    return ret;
}

void ContextBrowserPlugin::HistoryEntry::setCursorPosition(const KTextEditor::Cursor& cursorPosition)
{
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    if (context.data()) {
        absoluteCursorPosition =  DocumentCursor(context.data()->url(), cursorPosition);
        relativeCursorPosition = cursorPosition;
        relativeCursorPosition.setLine(relativeCursorPosition.line() - context.data()->range().start.line);
    }
}

#include "contextbrowser.moc"
#include "moc_contextbrowser.cpp"

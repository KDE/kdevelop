/*
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ktexteditorpluginintegration.h"

#include <debug.h>

#include <QFileInfo>
#include <QWidget>
#include <QVBoxLayout>
#include <QStackedLayout>

#include <KParts/MainWindow>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/Editor>
#include <KTextEditor/Application>

#include <sublime/area.h>
#include <sublime/container.h>
#include <sublime/view.h>
#include <sublime/viewbarcontainer.h>

#include "core.h"
#include "uicontroller.h"
#include "documentcontroller.h"
#include "plugincontroller.h"
#include "sessioncontroller.h"
#include "mainwindow.h"
#include "textdocument.h"

#include <util/objectlist.h>

#include <algorithm>

using namespace KDevelop;

namespace {
/**
 * If a given URL is relative, return a version of the URL resolved
 * to the local file scheme; otherwise return a copy of the URL.
 *
 * KTextEditor (plugins) can look up or open a document by a relative URL.
 * DocumentController asserts that a URL passed to it is not relative. This function
 * assumes that a relative URL points to a local file and works around the assertion failures.
 *
 * @pre @p !url.isEmpty() because an empty URL is special and should be handled separately by the callers
 */
[[nodiscard]] QUrl resolvedToLocalFile(QUrl url)
{
    Q_ASSERT(!url.isEmpty());
    if (url.isRelative()) {
        url.setScheme(QStringLiteral("file"));
    }
    return url;
}

/**
 * @return whether a given URL can possibly point to a file
 *
 * KTextEditor (plugins) might look up or open a document by a local-file URL with an empty file name.
 * DocumentController asserts that a URL passed to it has a nonempty file name or is not a local file. This
 * function helps slots that implement KTextEditor API to properly fail instead of triggering the assertion failures.
 *
 * @pre @p !url.isEmpty() because an empty URL is special and should be handled separately by the callers
 * @pre @p !url.isRelative() because a relative URL is never a local file, and so must be resolved before the check
 */
[[nodiscard]] bool canPointToFile(const QUrl& url)
{
    Q_ASSERT(!url.isEmpty());
    Q_ASSERT(!url.isRelative());
    return !url.fileName().isEmpty() || !url.isLocalFile();
}

/**
 * If a given URL is a local file, return a version of the URL
 * with canonicalized path; otherwise return a copy of the URL.
 *
 * @pre @p !url.isEmpty() because an empty URL is special and should be handled separately by the callers
 * @pre @p !url.isRelative() because a relative URL is never a local file, and so must be resolved before the check
 */
[[nodiscard]] QUrl urlWithCanonicalizedPathIfLocalFile(const QUrl& url)
{
    Q_ASSERT(!url.isEmpty());
    Q_ASSERT(!url.isRelative());
    if (url.isLocalFile()) {
        const auto canonicalPath = QFileInfo{url.toLocalFile()}.canonicalFilePath();
        if (!canonicalPath.isEmpty()) {
            return QUrl::fromLocalFile(canonicalPath);
        }
    }
    return url;
}

KTextEditor::MainWindow *toKteWrapper(KParts::MainWindow *window)
{
    if (auto mainWindow = qobject_cast<KDevelop::MainWindow*>(window)) {
        return mainWindow->kateWrapper() ? mainWindow->kateWrapper()->interface() : nullptr;
    } else {
        return nullptr;
    }
}

KTextEditor::View *toKteView(Sublime::View *view)
{
    if (auto textView = qobject_cast<KDevelop::TextView*>(view)) {
        return textView->textView();
    } else {
        return nullptr;
    }
}

/**
 * @return an IDocument that wraps a given KTextEditor::Document or @c nullptr if no matching IDocument
 */
[[nodiscard]] IDocument* iDocumentFromKteDocument(KTextEditor::Document* document)
{
    if (!document) {
        return nullptr;
    }

    if (const auto url = document->url(); !url.isEmpty()) {
        // IDocument and its wrapped KTextEditor::Document have equal nonempty URLs
        auto* const iDocument = Core::self()->documentControllerInternal()->documentForUrl(url);
        if (iDocument->textDocument() == document) {
            return iDocument;
        }
        qCWarning(SHELL) << "a different document" << iDocument->textDocument()
                         << "is registered with the document controller under the URL"
                         << url.toString(QUrl::PreferLocalFile) << "of a given document" << document;
        return nullptr;
    }

    // Perform a linear search across all open documents. Cannot just look a document
    // up by an empty URL, because each IDocument has a unique nonempty URL.
    const auto documents = Core::self()->documentControllerInternal()->openDocuments();
    const auto it = std::find_if(documents.cbegin(), documents.cend(), [document](IDocument* iDocument) {
        return iDocument->textDocument() == document;
    });
    return it == documents.cend() ? nullptr : *it;
}

class ToolViewFactory;

/**
 * This HACK is required to massage the KTextEditor plugin API into the
 * GUI concepts we apply in KDevelop. Kate does not allow the user to
 * delete tool views and then readd them. We do. To support our use case
 * we prevent the widget we return to KTextEditor plugins from
 * MainWindow::createToolView from getting destroyed. This widget class
 * unsets the parent of the so called container in its dtor. The
 * ToolViewFactory handles the ownership and destroys the kate widget
 * as needed.
 */
class KeepAliveWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KeepAliveWidget(ToolViewFactory *factory, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_factory(factory)
    {
    }

    ~KeepAliveWidget() override;

private:
    ToolViewFactory* const m_factory;
};

class ToolViewFactory : public QObject, public KDevelop::IToolViewFactory
{
    Q_OBJECT
public:
    ToolViewFactory(const QString &text, const QIcon &icon, const QString &identifier,
                    KTextEditor::MainWindow::ToolViewPosition pos)
        : m_text(text)
        , m_icon(icon)
        , m_identifier(identifier)
        , m_container(new QWidget)
        , m_pos(pos)
    {
        m_container->setLayout(new QVBoxLayout);
    }

    ~ToolViewFactory() override
    {
        delete m_container;
    }

    QWidget *create(QWidget *parent = nullptr) override
    {
        auto widget = new KeepAliveWidget(this, parent);
        widget->setWindowTitle(m_text);
        widget->setWindowIcon(m_icon);
        widget->setLayout(new QVBoxLayout);
        widget->layout()->addWidget(m_container);
        widget->addActions(m_container->actions());
        return widget;
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        switch (m_pos) {
            case KTextEditor::MainWindow::Left:
                return Qt::LeftDockWidgetArea;
            case KTextEditor::MainWindow::Right:
                return Qt::RightDockWidgetArea;
            case KTextEditor::MainWindow::Top:
                return Qt::TopDockWidgetArea;
            case KTextEditor::MainWindow::Bottom:
                return Qt::BottomDockWidgetArea;
        }
        Q_UNREACHABLE();
    }

    QString id() const override
    {
        return m_identifier;
    }

    QWidget *container() const
    {
        return m_container;
    }

private:
    const QString m_text;
    const QIcon m_icon;
    const QString m_identifier;
    QPointer<QWidget> m_container;
    const KTextEditor::MainWindow::ToolViewPosition m_pos;
    friend class KeepAliveWidget;
};

KeepAliveWidget::~KeepAliveWidget()
{
    // if the container is still valid, unparent it to prevent it from getting deleted
    // this happens when the user removes a tool view
    // on shutdown, the container does get deleted, thus we must guard against that.
    if (m_factory->container()) {
        Q_ASSERT(m_factory->container()->parentWidget() == this);
        m_factory->container()->setParent(nullptr);
    }
}

}

namespace KTextEditorIntegration {

Application::Application(QObject *parent)
    : QObject(parent)
{
}

Application::~Application()
{
    KTextEditor::Editor::instance()->setApplication(nullptr);
}

KTextEditor::MainWindow *Application::activeMainWindow() const
{
    return toKteWrapper(Core::self()->uiController()->activeMainWindow());
}

QList<KTextEditor::MainWindow *> Application::mainWindows() const
{
    return {activeMainWindow()};
}

bool Application::closeDocument(KTextEditor::Document *document) const
{
    if (auto* const iDocument = iDocumentFromKteDocument(document)) {
        return iDocument->close();
    }

    qCWarning(SHELL) << "ignoring request to close a document not registered with the document controller" << document;
    // If the warning is printed in some scenario, consider returning
    // `document && document->closeUrl()` instead of `false`.
    return false;
}

KTextEditor::Plugin *Application::plugin(const QString &id) const
{
    auto kdevPlugin = Core::self()->pluginController()->loadPlugin(id);
    const auto plugin = dynamic_cast<Plugin*>(kdevPlugin);
    return plugin ? plugin->interface() : nullptr;
}

QList<KTextEditor::Document *> Application::documents()
{
    QList<KTextEditor::Document *> l;
    const auto openDocuments = Core::self()->documentControllerInternal()->openDocuments();
    l.reserve(openDocuments.size());
    for (auto* d : openDocuments) {
        l << d->textDocument();
    }
    return l;
}

KTextEditor::Document *Application::openUrl(const QUrl &url, const QString &encoding)
{
    Q_UNUSED(encoding);

    const IDocument* doc;
    if (url.isEmpty()) {
        doc = Core::self()->documentControllerInternal()->openDocumentFromText(QString());
    } else {
        const auto resolvedUrl = resolvedToLocalFile(url);
        if (!canPointToFile(resolvedUrl)) {
            return nullptr; // cannot create a document for a non-file URL
        }
        doc = Core::self()->documentControllerInternal()->openDocument(resolvedUrl);
    }
    return doc ? doc->textDocument() : nullptr;
}

KTextEditor::Document *Application::findUrl(const QUrl &url) const
{
    if (url.isEmpty()) {
        // Return the first found document with an empty URL.
        // Perform a linear search across all open documents. Cannot just look a document
        // up by an empty URL, because each IDocument has a unique nonempty URL.
        const auto documents = Core::self()->documentControllerInternal()->openDocuments();
        const auto it = std::find_if(documents.cbegin(), documents.cend(), [](const IDocument* document) {
            return DocumentController::isEmptyDocumentUrl(document->url());
        });
        return it == documents.cend() ? nullptr : (*it)->textDocument();
    }

    const auto resolvedUrl = resolvedToLocalFile(url);
    if (!canPointToFile(resolvedUrl)) {
        return nullptr; // a document cannot have a non-file URL
    }

    // ensure that a local-file URL has canonical path so that DocumentController::documentForUrl() finds the document
    const auto* const doc =
        Core::self()->documentControllerInternal()->documentForUrl(urlWithCanonicalizedPathIfLocalFile(resolvedUrl));
    return doc ? doc->textDocument() : nullptr;
}

bool Application::quit() const
{
    Core::self()->sessionController()->emitQuitSession();
    return true;
}

MainWindow::MainWindow(KDevelop::MainWindow *mainWindow)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_interface(new KTextEditor::MainWindow(this))
{
    connect(mainWindow, &Sublime::MainWindow::viewAdded, this, [this] (Sublime::View *view) {
        if (auto kteView = toKteView(view)) {
            emit m_interface->viewCreated(kteView);
        }
    });
    connect(mainWindow, &Sublime::MainWindow::activeViewChanged, this, [this] (Sublime::View *view) {
        auto kteView = toKteView(view);
        emit m_interface->viewChanged(kteView);

        if (auto viewBar = m_viewBars.value(kteView)) {
            m_mainWindow->viewBarContainer()->setCurrentViewBar(viewBar);
        }
    });
}

MainWindow::~MainWindow() = default;

QWidget *MainWindow::createToolView(KTextEditor::Plugin* plugin, const QString &identifier,
                                    KTextEditor::MainWindow::ToolViewPosition pos,
                                    const QIcon &icon, const QString &text)
{
    auto factory = new ToolViewFactory(text, icon, identifier, pos);
    Core::self()->uiController()->addToolView(text, factory);
    connect(plugin, &QObject::destroyed, this, [=] {
        Core::self()->uiController()->removeToolView(factory);
    });
    return factory->container();
}

KXMLGUIFactory *MainWindow::guiFactory() const
{
    return m_mainWindow->guiFactory();
}

QWidget *MainWindow::window() const
{
    return m_mainWindow;
}

QList<KTextEditor::View *> MainWindow::views() const
{
    QList<KTextEditor::View *> kteViews;
    const auto areas = m_mainWindow->areas();
    for (auto* area : areas) {
        const auto views = area->views();
        for (auto* view : views) {
            if (auto kteView = toKteView(view)) {
                kteViews << kteView;
            }
        }
    }
    return kteViews;
}

KTextEditor::View *MainWindow::activeView() const
{
    return toKteView(m_mainWindow->activeView());
}

KTextEditor::View* MainWindow::activateView(KTextEditor::Document* document)
{
    if (auto* const iDocument = iDocumentFromKteDocument(document)) {
        Core::self()->documentControllerInternal()->activateDocument(iDocument);
        auto* const view = activeView();
        if (view && view->document() == document) {
            return view;
        }
        qCWarning(SHELL) << "activating a document" << document << "failed, active view:" << view;
        return nullptr;
    }
    qCWarning(SHELL) << "ignoring request to activate a document not registered with the document controller"
                     << document;
    return nullptr;
}

bool MainWindow::closeView(KTextEditor::View *kteView)
{
    if (!kteView) {
        return false;
    }

    const auto areas = m_mainWindow->areas();
    for (auto* area : areas) {
        const auto views = area->views();
        for (auto* view : views) {
            if (toKteView(view) == kteView) {
                area->closeView(view);
                return true;
            }
        }
    }

    return false;
}

bool MainWindow::closeSplitView(KTextEditor::View *kteView)
{
    return closeView(kteView);
}

bool MainWindow::viewsInSameSplitView(KTextEditor::View *kteView1, KTextEditor::View *kteView2) const
{
    if (!kteView1 || !kteView2) {
        return false;
    }
    if (kteView1 == kteView2) {
        return true;
    }

    bool view1Found = false;
    bool view2Found = false;
    const auto containers = m_mainWindow->containers();
    for (const auto* container : containers) {
        const auto views = container->views();
        for (auto* view : views) {
            const KTextEditor::View *kteView = toKteView(view);
            if (kteView == kteView1) {
                view1Found = true;
            } else if (kteView == kteView2) {
                view2Found = true;
            }

            if (view1Found && view2Found) {
                // both views found in the same container
                return true;
            }
        }

        if (view1Found != view2Found) {
            // only one view being found implies that the other is in a different container
            return false;
        }
    }

    return false;
}

QObject *MainWindow::pluginView(const QString &id) const
{
    return m_pluginViews.value(id);
}

QWidget *MainWindow::createViewBar(KTextEditor::View *view)
{
    Q_UNUSED(view);

    // we reuse the central view bar for every view
    return m_mainWindow->viewBarContainer();
}

void MainWindow::deleteViewBar(KTextEditor::View *view)
{
    auto viewBar = m_viewBars.take(view);
    m_mainWindow->viewBarContainer()->removeViewBar(viewBar);
    delete viewBar;
}

void MainWindow::showViewBar(KTextEditor::View *view)
{
    auto viewBar = m_viewBars.value(view);
    Q_ASSERT(viewBar);

    m_mainWindow->viewBarContainer()->showViewBar(viewBar);
}

void MainWindow::hideViewBar(KTextEditor::View *view)
{
    auto viewBar = m_viewBars.value(view);
    Q_ASSERT(viewBar);
    m_mainWindow->viewBarContainer()->hideViewBar(viewBar);
}

void MainWindow::addWidgetToViewBar(KTextEditor::View *view, QWidget *widget)
{
    Q_ASSERT(widget);
    m_viewBars[view] = widget;

    m_mainWindow->viewBarContainer()->addViewBar(widget);
}

KTextEditor::View *MainWindow::openUrl(const QUrl &url, const QString &encoding)
{
    if (auto* const document = KTextEditor::Editor::instance()->application()->openUrl(url, encoding)) {
        // Application::openUrl() activates a view of the opened document, so just return the active view.
        auto* const view = activeView();
        Q_ASSERT(view);
        Q_ASSERT(view->document() == document);
        return view;
    }
    return nullptr;
}

bool MainWindow::showToolView(QWidget *widget)
{
    if (widget->parentWidget()) {
        Core::self()->uiController()->raiseToolView(widget->parentWidget());
        return true;
    }
    return false;
}

KTextEditor::MainWindow *MainWindow::interface() const
{
    return m_interface;
}

void MainWindow::addPluginView(const QString &id, QObject *view)
{
    m_pluginViews.insert(id, view);
    emit m_interface->pluginViewCreated(id, view);
}

void MainWindow::removePluginView(const QString &id)
{
    auto view = m_pluginViews.take(id).data();
    delete view;
    emit m_interface->pluginViewDeleted(id, view);
}

Plugin::Plugin(KTextEditor::Plugin* plugin, QObject* parent, const KPluginMetaData& metaData)
    : IPlugin({}, parent, metaData)
    , m_plugin(plugin)
    , m_tracker(new ObjectListTracker(ObjectListTracker::CleanupWhenDone, this))
{
}

Plugin::~Plugin() = default;

void Plugin::unload()
{
    if (auto mainWindow = KTextEditor::Editor::instance()->application()->activeMainWindow()) {
        auto integration = qobject_cast<MainWindow*>(mainWindow->parent());
        if (integration) {
            integration->removePluginView(pluginId());
        }
    }
    m_tracker->deleteAll();
    delete m_plugin;
}

KXMLGUIClient *Plugin::createGUIForMainWindow(Sublime::MainWindow* window)
{
    auto ret = IPlugin::createGUIForMainWindow(window);
    auto mainWindow = qobject_cast<KDevelop::MainWindow*>(window);
    Q_ASSERT(mainWindow);

    auto wrapper = mainWindow->kateWrapper();
    auto view = m_plugin->createView(wrapper->interface());
    wrapper->addPluginView(pluginId(), view);
    // ensure that unloading the plugin kills all views
    m_tracker->append(view);

    return ret;
}

KTextEditor::Plugin *Plugin::interface() const
{
    return m_plugin.data();
}

QString Plugin::pluginId() const
{
    return Core::self()->pluginController()->pluginInfo(this).pluginId();
}

void initialize()
{
    auto app = new KTextEditor::Application(new Application(Core::self()));
    KTextEditor::Editor::instance()->setApplication(app);
}

void MainWindow::splitView(Qt::Orientation orientation)
{
    m_mainWindow->split(orientation);
}

}

#include "ktexteditorpluginintegration.moc"
#include "moc_ktexteditorpluginintegration.cpp"

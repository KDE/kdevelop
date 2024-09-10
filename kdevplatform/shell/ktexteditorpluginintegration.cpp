/*
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ktexteditorpluginintegration.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QChildEvent>

#include <KParts/MainWindow>
#include <KTextEditor/View>
#include <KTextEditor/Editor>
#include <KTextEditor/Application>

#include <sublime/area.h>
#include <sublime/container.h>
#include <sublime/view.h>
#include <sublime/viewbarcontainer.h>
#include <outputview/outputjob.h>
#include <outputview/outputmodel.h>
#include <outputview/outputdelegate.h>

#include <shell/editorconfigpage.h>

#include "core.h"
#include "debug.h"
#include "uicontroller.h"
#include "documentcontroller.h"
#include "plugincontroller.h"
#include "sessioncontroller.h"
#include "mainwindow.h"
#include "textdocument.h"

#include <util/objectlist.h>

using namespace KDevelop;

namespace {

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

class ToolViewWidget : public QWidget
{
    Q_OBJECT
public:
    ToolViewWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
    }

protected:
    void childEvent(QChildEvent* ev) override
    {
        // copied kate's behaviour
        if (ev->type() == QEvent::ChildAdded) {
            if (QWidget* widget = qobject_cast<QWidget*>(ev->child())) {
                setFocusProxy(widget);
                layout()->addWidget(widget);
            }
        }

        QWidget::childEvent(ev);
    }
};

class ToolViewFactory : public QObject, public KDevelop::IToolViewFactory
{
    Q_OBJECT
public:
    ToolViewFactory(const QString& text, const QIcon& icon, const QString& identifier,
                    KTextEditor::MainWindow::ToolViewPosition pos)
        : m_text(text)
        , m_icon(icon)
        , m_identifier(identifier)
        , m_container(new ToolViewWidget)
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
    const auto& openDocuments = Core::self()->documentControllerInternal()->openDocuments();
    for (auto doc : openDocuments) {
        if (doc->textDocument() == document) {
            return doc->close();
        }
    }
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

    auto documentController = Core::self()->documentControllerInternal();
    auto doc = url.isEmpty() ? documentController->openDocumentFromText(QString()) : documentController->openDocument(url);
    return doc->textDocument();
}

KTextEditor::Document *Application::findUrl(const QUrl &url) const
{
    auto doc = Core::self()->documentControllerInternal()->documentForUrl(url);
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

class ShowMessagesJob : public OutputJob
{
public:
    ShowMessagesJob()
        : OutputJob()
    {
        setStandardToolView(IOutputView::StandardToolView::Messages);
        setVerbosity(OutputJob::Silent);
        if (!qobject_cast<OutputModel*>(model()))
        {
            setModel(new OutputModel);
            setDelegate(new OutputDelegate);
        }
    }

    void postMessages(std::vector<QVariantMap> const& messages)
    {
        auto const KeyCategory = QLatin1String("category");
        auto const KeyText = QLatin1String("text");
        for (auto const& message : messages) {
            for (auto const& messageLine: message[KeyText].toString().split(QLatin1String("\n"))) {
                auto const line =
                    QLatin1String("%1: %2").arg(message[KeyCategory].toString()).arg(messageLine);
                static_cast<OutputModel*>(model())->appendLine(line);
            }
        }
    }

    void start() override
    {
        startOutput();
    }

private:
    std::vector<QVariantMap> m_messages;
};

void MainWindow::showMessage(QVariantMap message)
{
    qCInfo(SHELL) << "received message:" << message;
    if (!m_showMessagesOutputJob)
    {
        m_showMessagesOutputJob = std::make_shared<ShowMessagesJob>();
        m_showMessagesOutputJob->start();
    }
    m_showMessagesOutputJob->postMessages({message});
}

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

KTextEditor::View *MainWindow::activateView(KTextEditor::Document *doc)
{
    const auto areas = m_mainWindow->areas();
    for (auto* area : areas) {
        const auto views = area->views();
        for (auto* view : views) {
            if (auto kteView = toKteView(view)) {
                if (kteView->document() == doc) {
                    m_mainWindow->activateView(view);
                    return kteView;
                }
            }
        }
    }

    return activeView();
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
    return activateView(KTextEditor::Editor::instance()->application()->openUrl(url, encoding));
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

KDevelop::ConfigPage* Plugin::configPage(int number, QWidget *parent)
{
    return new KDevelop::KTextEditorConfigPageAdapter(m_plugin->configPage(number, parent));
}

int Plugin::configPages() const
{
    return m_plugin->configPages();
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

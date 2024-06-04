/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2016 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "standarddocumentationview.h"
#include "documentationfindwidget.h"
#include "debug.h"

#include <util/kdevstringhandler.h>
#include <util/zoomcontroller.h>

#include <KConfigGroup>
#include <KSharedConfig>

#include <QVBoxLayout>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QUrl>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPointer>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineUrlScheme>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

using namespace KDevelop;

namespace {
auto qtHelpSchemeName() { return QByteArrayLiteral("qthelp"); }

class StandardDocumentationPage : public QWebEnginePage
{
    Q_OBJECT
public:
    StandardDocumentationPage(QWebEngineProfile* profile, KDevelop::StandardDocumentationView* parent)
        : QWebEnginePage(profile, parent),
          m_view(parent)
    {
    }

    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override
    {
        if (DOCUMENTATION().isDebugEnabled()) {
            if (url.scheme() == QLatin1String("data")) {
                qCDebug(DOCUMENTATION) << "navigating to a manually constructed page because" << type;
            } else {
                qCDebug(DOCUMENTATION) << "navigating to" << url << "because" << type;
            }
        }

        if (type == NavigationTypeLinkClicked && m_isDelegating) {
            emit m_view->linkClicked(url);
            return false;
        }

        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }

    void setLinkDelegating(bool isDelegating) { m_isDelegating = isDelegating; }

private:
    KDevelop::StandardDocumentationView* const m_view;
    bool m_isDelegating = false;
};

} // unnamed namespace

void StandardDocumentationView::registerCustomUrlSchemes()
{
    QWebEngineUrlScheme scheme(qtHelpSchemeName());
    QWebEngineUrlScheme::registerScheme(scheme);
}

class KDevelop::StandardDocumentationViewPrivate
{
public:
    ZoomController* m_zoomController = nullptr;
    IDocumentation::Ptr m_doc;

    QWebEngineView* m_view = nullptr;
    StandardDocumentationPage* m_page = nullptr;

    ~StandardDocumentationViewPrivate()
    {
        // make sure the page is deleted before the profile
        // see https://doc.qt.io/qt-5/qwebenginepage.html#QWebEnginePage-1
        delete m_page;
    }

    void init(StandardDocumentationView* parent)
    {
        // prevent QWebEngine (Chromium) from overriding the signal handlers of KCrash
        const auto chromiumFlags = qgetenv("QTWEBENGINE_CHROMIUM_FLAGS");
        if (!chromiumFlags.contains("disable-in-process-stack-traces")) {
            qputenv("QTWEBENGINE_CHROMIUM_FLAGS", QByteArray(chromiumFlags + " --disable-in-process-stack-traces"));
        }
        // not using the shared default profile here:
        // prevents conflicts with qthelp scheme handler being registered onto that single default profile
        // due to async deletion of old pages and their CustomSchemeHandler instance
        auto* profile = new QWebEngineProfile(parent);
        m_page = new StandardDocumentationPage(profile, parent);
        m_view = new QWebEngineView(parent);
        m_view->setPage(m_page);
        m_view->setContextMenuPolicy(Qt::NoContextMenu);

        // The event filter is necessary for handling mouse events since they are swallowed by QWebEngineView.
        m_view->installEventFilter(parent);
    }
};

StandardDocumentationView::StandardDocumentationView(DocumentationFindWidget* findWidget, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new StandardDocumentationViewPrivate)
{
    Q_D(StandardDocumentationView);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    d->init(this);
    layout()->addWidget(d->m_view);

    findWidget->setEnabled(true);
    connect(findWidget, &DocumentationFindWidget::searchRequested, this, &StandardDocumentationView::search);
    connect(findWidget, &DocumentationFindWidget::searchDataChanged, this, &StandardDocumentationView::searchIncremental);
    connect(findWidget, &DocumentationFindWidget::searchFinished, this, &StandardDocumentationView::finishSearch);
}

KDevelop::StandardDocumentationView::~StandardDocumentationView()
{
    Q_D(StandardDocumentationView);

    // Prevent getting a loadFinished() signal on destruction.
    disconnect(d->m_view, nullptr, this, nullptr);
}

void StandardDocumentationView::search ( const QString& text, DocumentationFindWidget::FindOptions options )
{
    Q_D(StandardDocumentationView);

    QWebEnginePage::FindFlags ff = {};
    if(options & DocumentationFindWidget::Previous)
        ff |= QWebEnginePage::FindBackward;

    if(options & DocumentationFindWidget::MatchCase)
        ff |= QWebEnginePage::FindCaseSensitively;

    d->m_view->page()->findText(text, ff);
}

void StandardDocumentationView::searchIncremental(const QString& text, DocumentationFindWidget::FindOptions options)
{
    Q_D(StandardDocumentationView);

    QWebEnginePage::FindFlags findFlags;

    if (options & DocumentationFindWidget::MatchCase)
        findFlags |= QWebEnginePage::FindCaseSensitively;

    // calling with changed text with added or removed chars at end will result in current
    // selection kept, if also matching new text
    // behaviour on changed case sensitivity though is advancing to next match even if current
    // would be still matching. as there is no control about currently shown match, nothing
    // we can do about it. thankfully case sensitivity does not happen too often, so should
    // not be too grave UX
    // at least with webengine 5.9.1 there is a bug when switching from no-casesensitivy to
    // casesensitivity, that global matches are not updated and the ones with non-matching casing
    // still active. no workaround so far.
    d->m_view->page()->findText(text, findFlags);
}

void StandardDocumentationView::finishSearch()
{
    Q_D(StandardDocumentationView);

    // passing empty string to reset search, as told in API docs
    d->m_view->page()->findText(QString());
}

void StandardDocumentationView::initZoom(const QString& configSubGroup)
{
    Q_D(StandardDocumentationView);

    Q_ASSERT_X(!d->m_zoomController, "StandardDocumentationView::initZoom", "Can not initZoom a second time.");

    const KConfigGroup outerGroup(KSharedConfig::openConfig(), QStringLiteral("Documentation View"));
    const KConfigGroup configGroup(&outerGroup, configSubGroup);
    d->m_zoomController = new ZoomController(configGroup, this);
    connect(d->m_zoomController, &ZoomController::factorChanged,
            this, &StandardDocumentationView::updateZoomFactor);
    updateZoomFactor(d->m_zoomController->factor());
}

void StandardDocumentationView::setDocumentation(const IDocumentation::Ptr& doc)
{
    Q_D(StandardDocumentationView);

    if(d->m_doc)
        disconnect(d->m_doc.data());
    d->m_doc = doc;
    update();
    if(d->m_doc)
        connect(d->m_doc.data(), &IDocumentation::descriptionChanged, this, &StandardDocumentationView::update);
}

void StandardDocumentationView::update()
{
    Q_D(StandardDocumentationView);

    if(d->m_doc) {
        setHtml(d->m_doc->description());
    } else
        qCDebug(DOCUMENTATION) << "calling StandardDocumentationView::update() on an uninitialized view";
}


void KDevelop::StandardDocumentationView::setOverrideCssFile(const QString& cssFilePath)
{
    QFile file(cssFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(DOCUMENTATION) << "cannot read CSS file" << cssFilePath << ':' << file.error() << file.errorString();
        return;
    }
    const auto cssCode = file.readAll();
    setOverrideCssCode(cssCode);
}

void StandardDocumentationView::setOverrideCssCode(const QByteArray& cssCode)
{
    Q_D(StandardDocumentationView);

    const auto scriptName = QStringLiteral("OverrideCss");
    auto& scripts = d->m_view->page()->scripts();

    const auto oldScripts = scripts.find(scriptName);
    Q_ASSERT_X(oldScripts.size() <= 1, Q_FUNC_INFO,
               "There should be at most one OverrideCss script, ours, if we set it before.");
    if (!oldScripts.empty()) {
        scripts.remove(oldScripts.front());
    }

    if (cssCode.isEmpty()) {
        return;
    }

    // The loading of CSS via JavaScript has a downside: pages are first loaded as is, then
    // reloaded with the style applied. When a page is large, the reloading is conspicuous
    // or causes flickering. For example, this can be seen on cmake-modules man page.
    // This cannot be fixed by specifying an earlier injection point - DocumentCreation -
    // because, according to QWebEngineScript documentation, this is not suitable for any
    // DOM operation. So with the DocumentCreation injection point the CSS style is not
    // applied and the following error appears in KDevelop's output:
    // js: Uncaught TypeError: Cannot read property 'appendChild' of null
    QWebEngineScript script;
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setName(scriptName);
    script.setRunsOnSubFrames(false);
    script.setSourceCode(QLatin1String("const css = document.createElement('style');"
                                       "css.innerText = '%1';"
                                       "document.head.appendChild(css);")
                             .arg(QString::fromUtf8(escapeJavaScriptString(cssCode))));
    script.setWorldId(QWebEngineScript::ApplicationWorld);

    scripts.insert(script);
}

void KDevelop::StandardDocumentationView::load(const QUrl& url)
{
    Q_D(StandardDocumentationView);

    d->m_view->page()->load(url);
}

void KDevelop::StandardDocumentationView::setHtml(const QString& html)
{
    Q_D(StandardDocumentationView);

    d->m_view->page()->setHtml(html);
}

class CustomSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    explicit CustomSchemeHandler(QNetworkAccessManager* nam, QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent), m_nam(nam) {}

    void requestStarted(QWebEngineUrlRequestJob *job) override {
        const QUrl url = job->requestUrl();

        auto* const reply = m_nam->get(QNetworkRequest(url));

        // Deliberately don't use job as context in this connection: if job is destroyed
        // before reply is finished, reply would be leaked. Using reply as context does
        // not impact behavior, but silences Clazy checker connect-3arg-lambda (level1).
        connect(reply, &QNetworkReply::finished, reply, [reply, job = QPointer{job}] {
            // At this point reply is no longer written to and can be safely
            // destroyed once job ends reading from it.
            if (job) {
                connect(job, &QObject::destroyed, reply, &QObject::deleteLater);
            } else {
                reply->deleteLater();
            }
        });

        job->reply(reply->header(QNetworkRequest::ContentTypeHeader).toByteArray(), reply);
    }

private:
    QNetworkAccessManager* m_nam;
};

void KDevelop::StandardDocumentationView::setNetworkAccessManager(QNetworkAccessManager* manager)
{
    Q_D(StandardDocumentationView);

    d->m_view->page()->profile()->installUrlSchemeHandler(qtHelpSchemeName(), new CustomSchemeHandler(manager, this));
}

void KDevelop::StandardDocumentationView::setDelegateLinks(bool delegate)
{
    Q_D(StandardDocumentationView);

    d->m_page->setLinkDelegating(delegate);
}

QMenu* StandardDocumentationView::createStandardContextMenu()
{
    Q_D(StandardDocumentationView);

    auto menu = new QMenu(this);
    auto copyAction = d->m_view->pageAction(QWebEnginePage::Copy);
    if (copyAction) {
        copyAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
        menu->addAction(copyAction);
    }
    return menu;
}

bool StandardDocumentationView::eventFilter(QObject* object, QEvent* event)
{
    Q_D(StandardDocumentationView);

    if (object == d->m_view) {
        /* HACK / Workaround for QTBUG-43602
         * Need to set an eventFilter on the child of WebengineView because it swallows
         * mouse events.
         */
        if (event->type() == QEvent::ChildAdded) {
            QObject* child = static_cast<QChildEvent*>(event)->child();
            if(qobject_cast<QWidget*>(child)) {
                child->installEventFilter(this);
            }
        } else if (event->type() == QEvent::ChildRemoved) {
            QObject* child = static_cast<QChildEvent*>(event)->child();
            if(qobject_cast<QWidget*>(child)) {
                child->removeEventFilter(this);
            }
        }
    }

    if (event->type() == QEvent::Wheel) {
        auto* const wheelEvent = static_cast<QWheelEvent*>(event);
        if (d->m_zoomController && d->m_zoomController->handleWheelEvent(wheelEvent))
            return true;
    } else if (event->type() == QEvent::MouseButtonPress) {
        const auto button = static_cast<QMouseEvent*>(event)->button();
        switch (button) {
        case Qt::MouseButton::ForwardButton:
            emit browseForward();
            event->accept();
            return true;
        case Qt::MouseButton::BackButton:
            emit browseBack();
            event->accept();
            return true;
        default:
            break;
        }
    }
    return QWidget::eventFilter(object, event);
}

void StandardDocumentationView::contextMenuEvent(QContextMenuEvent* event)
{
    auto menu = createStandardContextMenu();
    if (menu->isEmpty()) {
        delete menu;
        return;
    }

    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->exec(event->globalPos());
}

void StandardDocumentationView::updateZoomFactor(double zoomFactor)
{
    Q_D(StandardDocumentationView);

    d->m_view->setZoomFactor(zoomFactor);
}

void StandardDocumentationView::keyReleaseEvent(QKeyEvent* event)
{
    // Handle keyReleaseEvent instead of the usual keyPressEvent as a workaround
    // for the conflicting reset font size Ctrl+0 shortcut added into KTextEditor
    // in version 5.60. This new global shortcut prevents the Qt::Key_0 part of the
    // shortcut from reaching KeyPress events, but it doesn't affect KeyRelease events.
    // The end result is that Ctrl+0 always resets font size in the text editor
    // because its shortcut is global. In addition, Ctrl+0 resets zoom factor in
    // the current documentation provider if Documentation tool view has focus.
    // Unfortunately there is no way to reset documentation zoom factor without
    // simultaneously resetting font size in the text editor.
    // An alternative workaround - creating one more Ctrl+0 shortcut -
    // inevitably leads to conflicts with the KTextEditor's global shortcut.
    Q_D(StandardDocumentationView);

    if (d->m_zoomController && d->m_zoomController->handleKeyPressEvent(event)) {
        return;
    }
    QWidget::keyReleaseEvent(event);
}

#include "standarddocumentationview.moc"
#include "moc_standarddocumentationview.cpp"

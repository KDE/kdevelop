/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 * Copyright 2016 Igor Kushnir <igorkuo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "standarddocumentationview.h"
#include "documentationfindwidget.h"
#include "debug.h"

#include <util/zoomcontroller.h>

#include <KConfigGroup>
#include <KSharedConfig>

#include <QVBoxLayout>
#include <QContextMenuEvent>
#include <QMenu>

#ifdef USE_QTWEBKIT
#include <QFontDatabase>
#include <QWebView>
#include <QWebFrame>
#include <QWebSettings>
#else
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineProfile>
#endif

using namespace KDevelop;

#ifndef USE_QTWEBKIT
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
        qCDebug(DOCUMENTATION) << "navigating to..." << url << type;

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
#endif

class KDevelop::StandardDocumentationViewPrivate
{
public:
    ZoomController* m_zoomController = nullptr;
    IDocumentation::Ptr m_doc;

#ifdef USE_QTWEBKIT
    QWebView *m_view = nullptr;
    void init(StandardDocumentationView* parent)
    {
        m_view = new QWebView(parent);
        m_view->setContextMenuPolicy(Qt::NoContextMenu);
        QObject::connect(m_view, &QWebView::linkClicked, parent, &StandardDocumentationView::linkClicked);
    }
#else
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
            qputenv("QTWEBENGINE_CHROMIUM_FLAGS", chromiumFlags + " --disable-in-process-stack-traces");
        }
        // not using the shared default profile here:
        // prevents conflicts with qthelp scheme handler being registered onto that single default profile
        // due to async deletion of old pages and their CustomSchemeHandler instance
        auto* profile = new QWebEngineProfile(parent);
        m_page = new StandardDocumentationPage(profile, parent);
        m_view = new QWebEngineView(parent);
        m_view->setPage(m_page);
        // workaround for Qt::NoContextMenu broken with QWebEngineView, contextmenu event is always eaten
        // see https://bugreports.qt.io/browse/QTBUG-62345
        // we have to enforce deferring of event ourselves
        m_view->installEventFilter(parent);
    }
#endif
};

StandardDocumentationView::StandardDocumentationView(DocumentationFindWidget* findWidget, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new StandardDocumentationViewPrivate)
{
    Q_D(StandardDocumentationView);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    d->init(this);
    layout()->addWidget(d->m_view);

    findWidget->setEnabled(true);
    connect(findWidget, &DocumentationFindWidget::searchRequested, this, &StandardDocumentationView::search);
    connect(findWidget, &DocumentationFindWidget::searchDataChanged, this, &StandardDocumentationView::searchIncremental);
    connect(findWidget, &DocumentationFindWidget::searchFinished, this, &StandardDocumentationView::finishSearch);

#ifdef USE_QTWEBKIT
    QFont sansSerifFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    QFont monospaceFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    QWebSettings* s = d->m_view->settings();

    s->setFontFamily(QWebSettings::StandardFont, sansSerifFont.family());
    s->setFontFamily(QWebSettings::SerifFont, QStringLiteral("Serif"));
    s->setFontFamily(QWebSettings::SansSerifFont, sansSerifFont.family());
    s->setFontFamily(QWebSettings::FixedFont, monospaceFont.family());

    s->setFontSize(QWebSettings::DefaultFontSize, QFontInfo(sansSerifFont).pixelSize());
    s->setFontSize(QWebSettings::DefaultFixedFontSize, QFontInfo(monospaceFont).pixelSize());

    // Fixes for correct positioning. The problem looks like the following:
    //
    // 1) Some page is loaded and loadFinished() signal is emitted,
    //    after this QWebView set right position inside page.
    //
    // 2) After loadFinished() emitting, page JS code finishes it's work and changes
    //    font settings (size). This leads to page contents "moving" inside view widget
    //    and as a result we have wrong position.
    //
    // Such behavior occurs for example with QtHelp pages.
    //
    // To fix the problem, first, we disable view painter updates during load to avoid content
    // "flickering" and also to hide font size "jumping". Secondly, we reset position inside page
    // after loading with using standard QWebFrame method scrollToAnchor().

    connect(d->m_view, &QWebView::loadStarted, d->m_view, [this]() {
        Q_D(StandardDocumentationView);
        d->m_view->setUpdatesEnabled(false);
    });

    connect(d->m_view, &QWebView::loadFinished, this, [this](bool) {
        Q_D(StandardDocumentationView);
        if (d->m_view->url().isValid()) {
            d->m_view->page()->mainFrame()->scrollToAnchor(d->m_view->url().fragment());
        }
        d->m_view->setUpdatesEnabled(true);
    });
#endif
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

#ifdef USE_QTWEBKIT
    using WebkitThing = QWebPage;
#else
    using WebkitThing = QWebEnginePage;
#endif
    WebkitThing::FindFlags ff = {};
    if(options & DocumentationFindWidget::Previous)
        ff |= WebkitThing::FindBackward;

    if(options & DocumentationFindWidget::MatchCase)
        ff |= WebkitThing::FindCaseSensitively;

    d->m_view->page()->findText(text, ff);
}

void StandardDocumentationView::searchIncremental(const QString& text, DocumentationFindWidget::FindOptions options)
{
    Q_D(StandardDocumentationView);

#ifdef USE_QTWEBKIT
    using WebkitThing = QWebPage;
#else
    using WebkitThing = QWebEnginePage;
#endif
    WebkitThing::FindFlags findFlags;

    if (options & DocumentationFindWidget::MatchCase)
        findFlags |= WebkitThing::FindCaseSensitively;

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

    // passing emptry string to reset search, as told in API docs
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

void KDevelop::StandardDocumentationView::setOverrideCss(const QUrl& url)
{
    Q_D(StandardDocumentationView);

#ifdef USE_QTWEBKIT
    d->m_view->settings()->setUserStyleSheetUrl(url);
#else
    d->m_view->page()->runJavaScript(QLatin1String(
        "var link = document.createElement( 'link' );"
        "link.href = '") + url.toString() + QLatin1String("';"
        "link.type = 'text/css';"
        "link.rel = 'stylesheet';"
        "link.media = 'screen,print';"
        "document.getElementsByTagName( 'head' )[0].appendChild( link );")
    );
#endif
}

void KDevelop::StandardDocumentationView::load(const QUrl& url)
{
    Q_D(StandardDocumentationView);

#ifdef USE_QTWEBKIT
    d->m_view->load(url);
#else
    d->m_view->page()->load(url);
#endif
}

void KDevelop::StandardDocumentationView::setHtml(const QString& html)
{
    Q_D(StandardDocumentationView);

#ifdef USE_QTWEBKIT
    d->m_view->setHtml(html);
#else
    d->m_view->page()->setHtml(html);
#endif
}

#ifndef USE_QTWEBKIT
class CustomSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    explicit CustomSchemeHandler(QNetworkAccessManager* nam, QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent), m_nam(nam) {}

    void requestStarted(QWebEngineUrlRequestJob *job) override {
        const QUrl url = job->requestUrl();

        auto reply = m_nam->get(QNetworkRequest(url));
        job->reply("text/html", reply);
    }

private:
    QNetworkAccessManager* m_nam;
};
#endif

void KDevelop::StandardDocumentationView::setNetworkAccessManager(QNetworkAccessManager* manager)
{
    Q_D(StandardDocumentationView);

#ifdef USE_QTWEBKIT
    d->m_view->page()->setNetworkAccessManager(manager);
#else
    d->m_view->page()->profile()->installUrlSchemeHandler("qthelp", new CustomSchemeHandler(manager, this));
#endif
}

void KDevelop::StandardDocumentationView::setDelegateLinks(bool delegate)
{
    Q_D(StandardDocumentationView);

#ifdef USE_QTWEBKIT
    d->m_view->page()->setLinkDelegationPolicy(delegate ? QWebPage::DelegateAllLinks : QWebPage::DontDelegateLinks);
#else
    d->m_page->setLinkDelegating(delegate);
#endif
}

QMenu* StandardDocumentationView::createStandardContextMenu()
{
    Q_D(StandardDocumentationView);

    auto menu = new QMenu(this);
#ifdef USE_QTWEBKIT
    using WebkitThing = QWebPage;
#else
    using WebkitThing = QWebEnginePage;
#endif
    auto copyAction = d->m_view->pageAction(WebkitThing::Copy);
    if (copyAction) {
        copyAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
        menu->addAction(copyAction);
    }
    return menu;
}

bool StandardDocumentationView::eventFilter(QObject* object, QEvent* event)
{
    Q_D(StandardDocumentationView);

#ifndef USE_QTWEBKIT
    if (object == d->m_view) {
        // help QWebEngineView properly behave like expected as if Qt::NoContextMenu was set
        if (event->type() == QEvent::ContextMenu) {
            event->ignore();
            return true;
        }
    }
#endif

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

void StandardDocumentationView::keyPressEvent(QKeyEvent* event)
{
    Q_D(StandardDocumentationView);

    if (d->m_zoomController && d->m_zoomController->handleKeyPressEvent(event)) {
        return;
    }
    QWidget::keyPressEvent(event);
}

void StandardDocumentationView::wheelEvent(QWheelEvent* event)
{
    Q_D(StandardDocumentationView);

    if (d->m_zoomController && d->m_zoomController->handleWheelEvent(event)) {
        return;
    }
    QWidget::wheelEvent(event);
}

#ifndef USE_QTWEBKIT
#include "standarddocumentationview.moc"
#endif

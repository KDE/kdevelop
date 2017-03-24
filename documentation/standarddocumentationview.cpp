/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include <QVBoxLayout>

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

struct KDevelop::StandardDocumentationViewPrivate
{
    IDocumentation::Ptr m_doc;

#ifdef USE_QTWEBKIT
    QWebView *m_view = nullptr;
    void init(QWidget* parent) { m_view = new QWebView(parent); }
#else
    QWebEngineView* m_view = nullptr;
    void init(QWidget* parent) { m_view = new QWebEngineView(parent); }
#endif
};

StandardDocumentationView::StandardDocumentationView(DocumentationFindWidget* findWidget, QWidget* parent)
    : QWidget(parent)
    , d(new StandardDocumentationViewPrivate)
{
    setLayout(new QVBoxLayout(this));
    d->init(this);
    layout()->addWidget(d->m_view);

    findWidget->setEnabled(true);
    connect(findWidget, &DocumentationFindWidget::newSearch, this, &StandardDocumentationView::search);

#ifdef USE_QTWEBKIT
    QFont sansSerifFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    QFont monospaceFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    QFont minimalFont = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);

    QWebSettings* s = d->m_view->settings();

    s->setFontFamily(QWebSettings::StandardFont, sansSerifFont.family());
    s->setFontFamily(QWebSettings::SerifFont, "Serif");
    s->setFontFamily(QWebSettings::SansSerifFont, sansSerifFont.family());
    s->setFontFamily(QWebSettings::FixedFont, monospaceFont.family());

    s->setFontSize(QWebSettings::DefaultFontSize, QFontInfo(sansSerifFont).pixelSize());
    s->setFontSize(QWebSettings::DefaultFixedFontSize, QFontInfo(monospaceFont).pixelSize());
    s->setFontSize(QWebSettings::MinimumFontSize, QFontInfo(minimalFont).pixelSize());

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
        d->m_view->setUpdatesEnabled(false);
    });

    connect(d->m_view, &QWebView::loadFinished, this, [this](bool) {
        if (d->m_view->url().isValid()) {
            d->m_view->page()->mainFrame()->scrollToAnchor(d->m_view->url().fragment());
        }
        d->m_view->setUpdatesEnabled(true);
    });
#endif
}

KDevelop::StandardDocumentationView::~StandardDocumentationView() = default;

void StandardDocumentationView::search ( const QString& text, DocumentationFindWidget::FindOptions options )
{
#ifdef USE_QTWEBKIT
    typedef QWebPage WebkitThing;
#else
    typedef QWebEnginePage WebkitThing;
#endif
    WebkitThing::FindFlags ff = 0;
    if(options & DocumentationFindWidget::Previous)
        ff |= WebkitThing::FindBackward;

    if(options & DocumentationFindWidget::MatchCase)
        ff |= WebkitThing::FindCaseSensitively;

    d->m_view->page()->findText(text, ff);
}

void StandardDocumentationView::setDocumentation(const IDocumentation::Ptr& doc)
{
    if(d->m_doc)
        disconnect(d->m_doc.data());
    d->m_doc = doc;
    update();
    if(d->m_doc)
        connect(d->m_doc.data(), &IDocumentation::descriptionChanged, this, &StandardDocumentationView::update);
}

void StandardDocumentationView::update()
{
    if(d->m_doc) {
        setHtml(d->m_doc->description());
    } else
        qCDebug(DOCUMENTATION) << "calling StandardDocumentationView::update() on an uninitialized view";
}

void KDevelop::StandardDocumentationView::setOverrideCss(const QUrl& url)
{
#ifdef USE_QTWEBKIT
    d->m_view->settings()->setUserStyleSheetUrl(url);
#else
    d->m_view->page()->runJavaScript(
        "var link = document.createElement( 'link' );"
        "link.href = " + url.toString().toUtf8() + ";"
        "link.type = 'text/css';"
        "link.rel = 'stylesheet';"
        "link.media = 'screen,print';"
        "document.getElementsByTagName( 'head' )[0].appendChild( link );"
    );
#endif
}

void KDevelop::StandardDocumentationView::load(const QUrl& url)
{
#ifdef USE_QTWEBKIT
    d->m_view->load(url);
#else
    d->m_view->page()->load(url);
#endif
}

void KDevelop::StandardDocumentationView::setHtml(const QString& html)
{
#ifdef USE_QTWEBKIT
    d->m_view->setHtml(html);
#else
    d->m_view->page()->setHtml(html);
#endif
}

#ifndef USE_QTWEBKIT
class CustomSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    explicit CustomSchemeHandler(QNetworkAccessManager* nam, QObject *parent = 0)
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
#ifdef USE_QTWEBKIT
    d->m_view->page()->setNetworkAccessManager(manager);
#else
    d->m_view->page()->profile()->installUrlSchemeHandler("qthelp", new CustomSchemeHandler(manager, this));
#endif
}

#ifndef USE_QTWEBKIT
class PageInterceptor : public QWebEnginePage
{
public:
    PageInterceptor(KDevelop::StandardDocumentationView* parent) : QWebEnginePage(parent), m_view(parent) {}

    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool /*isMainFrame*/) override {
        qCDebug(DOCUMENTATION) << "navigating to..." << url << type;
        if (type == NavigationTypeLinkClicked) {
            m_view->load(url);
            return false;
        }
        return true;
    }

    KDevelop::StandardDocumentationView* m_view;
};
#endif

void KDevelop::StandardDocumentationView::setDelegateLinks(bool delegate)
{
#ifdef USE_QTWEBKIT
    d->m_view->page()->setLinkDelegationPolicy(delegate ? QWebPage::DelegateAllLinks : QWebPage::DontDelegateLinks);
#else
    if (delegate)
        d->m_view->setPage(new PageInterceptor(this));
    else
        d->m_view->setPage(new QWebEnginePage(this));
#endif
}

QAction * KDevelop::StandardDocumentationView::copyAction() const
{
#ifdef USE_QTWEBKIT
    typedef QWebPage WebkitThing;
#else
    typedef QWebEnginePage WebkitThing;
#endif
    return d->m_view->pageAction(WebkitThing::Copy);
}

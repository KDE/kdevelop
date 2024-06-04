/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpdocumentation.h"

#include <QLabel>
#include <QUrl>
#include <QTreeView>
#include <QHelpContentModel>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QActionGroup>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>
#include <QMimeDatabase>
#include <QBuffer>
#include <QThread>

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <documentation/standarddocumentationview.h>
#include "qthelpproviderabstract.h"

#include <algorithm>

using namespace KDevelop;

QtHelpDocumentation::QtHelpDocumentation(QtHelpProviderAbstract* provider, const QString& name,
                                         const QList<QHelpLink>& info)
    : m_provider(provider)
    , m_name(name)
    , m_info(info)
    , m_current(info.constBegin())
    , lastView(nullptr)
{
}

namespace {
QList<QHelpLink>::const_iterator findTitle(const QList<QHelpLink>& links, const QString& title)
{
    return std::find_if(links.begin(), links.end(), [title](const QHelpLink& helpLink) {
        return helpLink.title == title;
    });
}
}

QtHelpDocumentation::QtHelpDocumentation(QtHelpProviderAbstract* provider, const QString& name,
                                         const QList<QHelpLink>& info, const QString& key)
    : m_provider(provider)
    , m_name(name)
    , m_info(info)
    , m_current(::findTitle(m_info, key))
    , lastView(nullptr)
{
    Q_ASSERT(m_current != m_info.constEnd());
}

namespace {
// remove HTML cruft to produce a clean description
QString cleanupDescription(QString description)
{
    {
        // Completely remove the first large header found, since we don't need a header
        const auto headerRegExp = QStringLiteral("<\\s*h\\d[^>]*>.*?<\\s*/\\s*h\\d\\s*>");
        static const auto findHeader = QRegularExpression(headerRegExp);
        const auto match = findHeader.match(description);
        if (match.hasMatch()) {
            description.remove(match.capturedStart(), match.capturedLength());
        }
    }

    {
        // Replace all gigantic header-font sizes with <big>
        {
            const auto sizeRegExp = QStringLiteral("<\\s*h\\d\\s*");
            static const auto findSize = QRegularExpression(sizeRegExp);
            description.replace(findSize, QStringLiteral("<big "));
        }
        {
            const auto sizeCloseRegExp = QStringLiteral("<\\s*/\\s*h\\d\\s*>");
            static const auto closeSize = QRegularExpression(sizeCloseRegExp);
            description.replace(closeSize, QStringLiteral("</big><br />"));
        }
    }

    {
        // Replace paragraphs by newlines
        const auto begin = QStringLiteral("<\\s*p\\s*>");
        static const auto findBegin = QRegularExpression(begin);
        description.replace(findBegin, {});

        const auto end = QStringLiteral("<\\s*/p\\s*>");
        static const auto findEnd = QRegularExpression(end);
        description.replace(findEnd, QStringLiteral("<br />"));
    }

    {
        // Remove links, because they won't work
        const auto link = QStringLiteral("<\\s*a\\s*href\\s*=\\s*['\"].*?['\"]\\s*");
        static const auto exp = QRegularExpression(link, QRegularExpression::CaseInsensitiveOption);
        description.replace(exp, QStringLiteral("<a "));
    }

    return description;
}

// try to extract description using comment markers
QString descriptionFromCommentMarkers(const QByteArray& utf8Fragment, const QByteArray& utf8Data)
{
    // find the start marker
    const auto commentMarkerStart = utf8Data.indexOf(QByteArray("<!-- $$$" + utf8Fragment));
    if (commentMarkerStart == -1) {
        return {};
    }

    // find the end marker
    auto commentMarkerEnd =
        utf8Data.indexOf(QByteArray("<!-- @@@" + utf8Fragment), commentMarkerStart + 8 + utf8Fragment.size());
    if (commentMarkerEnd == -1) {
        return {};
    }

    // then cleanup the data inbetween these two places
    return cleanupDescription(
        QString::fromUtf8(utf8Data.mid(commentMarkerStart, commentMarkerEnd - commentMarkerStart)));
}

// try to extract description via HTML parsing using new Qt6 documentation format
QString descriptionFromNewHtmlData(const QString& fragment, const QString& data)
{
    // find the header that references the fragment
    const auto sectionStartPattern = QRegularExpression(QLatin1String("<\\s*h(\\d)[^>]*id=\"%1\"").arg(fragment));
    const auto matchStart = sectionStartPattern.match(data);
    if (!matchStart.hasMatch()) {
        return {};
    }

    // if that fails find the start of the next section by the header
    const auto headerType = matchStart.capturedView(1);
    const auto sectionStart = matchStart.capturedStart(0);
    const auto sectionEndPattern = QRegularExpression(QLatin1String("<\\s*h%1[^>]*id=\"").arg(headerType));
    const auto matchEnd = sectionEndPattern.match(data, matchStart.capturedEnd(0));
    const auto sectionEnd = matchEnd.hasMatch() ? matchEnd.capturedStart(0) : data.size();

    // then cleanup the data inbetween these two places
    return cleanupDescription(data.mid(sectionStart, sectionEnd - sectionStart));
}

// extract description via HTML parsing using the old Qt5-like documentation format
QString descriptionFromOldHtmlData(const QString& fragment, const QString& dataString)
{
    const QString p = QStringLiteral("((\\\")|(\\\'))");
    const QString optionalSpace = QStringLiteral(" *");

    QString::size_type pos = 0;

    {
        // Find the title, and start from it in order to skip the useless navigation bar and table of contents.
        // In case of empty fragment (class documentation), the entire title matches the findHeader regex and
        // is removed below. This title should be removed, because it duplicates information already present
        // at the top of a navigation widget. A title example: "QString Class".
        const auto titleRegExp =
            QStringLiteral("< h\\d class = \"title\"[^>]*>").replace(QLatin1Char(' '), optionalSpace);
        const QRegularExpression findTitle(titleRegExp);
        const auto titlePos = dataString.indexOf(findTitle);
        if (titlePos != -1) {
            pos = titlePos;
        }
    }

    auto nextFragmentSearchPos = pos;

    if (!fragment.isEmpty()) {
        const QString exp = QString(QLatin1String("< a name = ") + p + fragment + p + QLatin1String(" > < / a >")).replace(QLatin1Char(' '), optionalSpace);

        const QRegularExpression findFragment(exp);
        QRegularExpressionMatch findFragmentMatch;
        pos = dataString.indexOf(findFragment, pos, &findFragmentMatch);
        if (pos == -1) {
            return {};
        }
        nextFragmentSearchPos = findFragmentMatch.capturedEnd();

        // Check if there is a title opening-tag right before the fragment, and if yes add it in order to match
        // and remove the entire title using the findHeader regex below. This title should be removed, because it
        // duplicates information already present in a structured form at the top of a navigation widget. A title
        // example: "bool QString::contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const".
        const QString titleRegExp = QStringLiteral("< h\\d class = \".*?\" >").replace(QLatin1Char(' '), optionalSpace);
        const QRegularExpression findTitle(titleRegExp);
        QRegularExpressionMatch match;
        const auto titleStart = dataString.lastIndexOf(findTitle, pos, &match);
        Q_ASSERT(titleStart < pos);
        if(titleStart != -1) {
            const auto titleEnd = match.capturedEnd();
            Q_ASSERT(titleEnd <= pos);
            const auto between = QStringView{dataString}.mid(titleEnd, pos - titleEnd).trimmed();
            if(between.isEmpty())
                pos = titleStart;
        }
    }

    const QString exp = QString(QStringLiteral("< a name = ") + p + QStringLiteral("((\\S)*)") + p + QStringLiteral(" > < / a >")).replace(QLatin1Char(' '), optionalSpace);
    const QRegularExpression nextFragmentExpression(exp);
    auto endPos = dataString.indexOf(nextFragmentExpression, nextFragmentSearchPos);
    if(endPos == -1) {
        endPos = dataString.size();
    }

    {
        //Find the end of the last paragraph or newline, so we don't add prefixes of the following fragment
        const QString newLineRegExp = QStringLiteral ("< br / > | < / p >").replace(QLatin1Char(' '), optionalSpace);
        const QRegularExpression lastNewLine(newLineRegExp);
        const auto newEnd = dataString.lastIndexOf(lastNewLine, endPos);
        if (newEnd > pos) {
            // Also remove the trailing line break to prevent two consecutive empty lines in a navigation widget.
            endPos = newEnd;
        }
    }

    return cleanupDescription(dataString.mid(pos, endPos - pos));
}

QString descriptionFallback(const QList<QHelpLink>& info)
{
    QStringList titles;
    titles.reserve(info.size());
    for (const auto& link : info) {
        titles.append(link.title);
    }
    return titles.join(QLatin1String(", "));
}
} // unnamed namespace

QString QtHelpDocumentation::description() const
{
    const auto url = currentUrl();
    const auto fragment = url.fragment();

    if (!fragment.isEmpty()) {
        // Extract a short description from the html data

        // assume the data is utf8 encoded
        // this is true for new data at least with <meta charset="utf-8">
        const auto utf8FileData = m_provider->engine()->fileData(url);

        // first, fast pass that does not require regular expression matching
        if (auto ret = descriptionFromCommentMarkers(fragment.toUtf8(), utf8FileData); !ret.isEmpty()) {
            return ret;
        }

        // otherwise fallback with ugly HTML parsing using regexp magic, what could go wrong?
        const auto fileData = QString::fromUtf8(utf8FileData);
        if (auto ret = descriptionFromNewHtmlData(fragment, fileData); !ret.isEmpty()) {
            return ret;
        }

        if (auto ret = descriptionFromOldHtmlData(fragment, fileData); !ret.isEmpty()) {
            return ret;
        }
    }

    return descriptionFallback(m_info);
}

namespace {

class QtHelpSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    explicit QtHelpSchemeHandler(QtHelpProviderAbstract* provider, QObject* parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent)
        , m_provider(provider)
    {
    }

    void requestStarted(QWebEngineUrlRequestJob* job) override
    {
        Q_ASSERT(QThread::currentThread() == m_provider->thread());

        const auto url = job->requestUrl();
        auto mimeType = QMimeDatabase().mimeTypeForUrl(url).name().toUtf8();
        if (mimeType == "application/x-extension-html") {
            // see also: https://bugs.kde.org/show_bug.cgi?id=288277
            // firefox seems to add this bullshit mimetype above
            // which breaks displaying of qthelp documentation :(
            mimeType = QByteArrayLiteral("text/html");
        }

        auto data = m_provider->engine()->fileData(url);

        // Fix flickering when loading, the page has the offline-simple.css stylesheet which is replaced
        // later by offline.css  by javascript which causes flickering so we force the full stylesheet
        // from the beginning
        if (url.fileName().endsWith(QLatin1String(".html"))) {
            data.replace("offline-simple.css", "offline.css");
        }

        auto buffer = new QBuffer(job);
        buffer->setData(data);
        buffer->open(QIODevice::ReadOnly);

        job->reply(mimeType, buffer);
    }

private:
    const QtHelpProviderAbstract* const m_provider;
};

} // unnamed namespace

QWidget* QtHelpDocumentation::documentationWidget(DocumentationFindWidget* findWidget, QWidget* parent)
{
    if(m_info.isEmpty()) { //QtHelp sometimes has empty info maps. e.g. availableaudioeffects i 4.5.2
        return new QLabel(i18n("Could not find any documentation for '%1'", m_name), parent);
    } else {
        auto* view = new StandardDocumentationView(findWidget, parent);
        view->initZoom(m_provider->name());
        view->setDelegateLinks(true);
        view->installUrlSchemeHandler(QByteArrayLiteral("qthelp"), new QtHelpSchemeHandler(m_provider, this));
        view->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(view, &StandardDocumentationView::linkClicked, this, &QtHelpDocumentation::jumpedTo);
        connect(view, &StandardDocumentationView::customContextMenuRequested, this, &QtHelpDocumentation::viewContextMenuRequested);

        view->load(currentUrl());
        lastView = view;
        return view;
    }
}

void QtHelpDocumentation::viewContextMenuRequested(const QPoint& pos)
{
    auto* view = qobject_cast<StandardDocumentationView*>(sender());
    if (!view)
        return;

    auto menu = view->createStandardContextMenu();

    if (m_info.count() > 1) {
        if (!menu->isEmpty()) {
            menu->addSeparator();
        }

        auto* actionGroup = new QActionGroup(menu);
        for (auto it = m_info.constBegin(), end = m_info.constEnd(); it != end; ++it) {
            const QString& name = it->title;
            auto* act=new QtHelpAlternativeLink(name, this, actionGroup);
            act->setCheckable(true);
            act->setChecked(name==currentTitle());
            menu->addAction(act);
        }
    }

    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->exec(view->mapToGlobal(pos));
}


void QtHelpDocumentation::jumpedTo(const QUrl& newUrl)
{
    Q_ASSERT(lastView);
    m_provider->jumpedTo(newUrl);
}

IDocumentationProvider* QtHelpDocumentation::provider() const
{
    return m_provider;
}

QtHelpAlternativeLink::QtHelpAlternativeLink(const QString& name, const QtHelpDocumentation* doc, QObject* parent)
    : QAction(name, parent), mDoc(doc), mName(name)
{
    connect(this, &QtHelpAlternativeLink::triggered, this, &QtHelpAlternativeLink::showUrl);
}

void QtHelpAlternativeLink::showUrl()
{
    IDocumentation::Ptr newDoc(new QtHelpDocumentation(mDoc->qtHelpProvider(), mName, mDoc->info(), mName));
    ICore::self()->documentationController()->showDocumentation(newDoc);
}

HomeDocumentation::HomeDocumentation(QtHelpProviderAbstract* provider)
    : m_provider(provider)
{
}

QWidget* HomeDocumentation::documentationWidget(DocumentationFindWidget*, QWidget* parent)
{
    auto* w=new QTreeView(parent);
    // install an event filter to get the mouse events out of it
    w->viewport()->installEventFilter(this);
    w->header()->setVisible(false);
    w->setModel(m_provider->engine()->contentModel());

    connect(w, &QTreeView::clicked, this, &HomeDocumentation::clicked);
    return w;
}

void HomeDocumentation::clicked(const QModelIndex& idx)
{
    QHelpContentModel* model = m_provider->engine()->contentModel();
    QHelpContentItem* it=model->contentItemAt(idx);

    const QList<QHelpLink> info{{it->url(), it->title()}};
    IDocumentation::Ptr newDoc(new QtHelpDocumentation(m_provider, it->title(), info));
    ICore::self()->documentationController()->showDocumentation(newDoc);
}

QString HomeDocumentation::name() const
{
    return i18n("%1 Home Page", m_provider->name());
}

IDocumentationProvider* HomeDocumentation::provider() const
{
    return m_provider;
}

bool HomeDocumentation::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress) {
        // Here we need to set accpeted to false to let it propagate up
        event->setAccepted(false);
    }
    return QObject::eventFilter(obj, event);
}

#include "qthelpdocumentation.moc"
#include "moc_qthelpdocumentation.cpp"

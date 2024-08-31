/*
    SPDX-FileCopyrightText: 2010 Yannick Motta <yannick.motta@gmail.com>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "manpagedocumentation.h"

#include "manpageplugin.h"
#include "manpagedocumentationwidget.h"
#include "debug.h"

#include <documentation/standarddocumentationview.h>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>

#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringView>
#include <QUrl>

namespace {
/**
 * This class makes sure that CSS embedded in man pages works and applies a custom style sheet on top.
 *
 * TODO: once Qt WebKit support is dropped, register with Qt WebEngine the "man" and "help" URL schemes as
 *       local; handle them. This will also make file:// links work properly. So this class would no longer
 *       have to fix embedded links and only need to embed our custom manpagedocumentation.css style like
 *       this: "<link href='file://%1' rel='stylesheet'>". Registering and handling the schemes might even
 *       allow to simplify the whole kdevmanpage plugin implementation.
 */
class StyleSheetFixer
{
public:
    static void process(QString& htmlPage)
    {
        static StyleSheetFixer instance;
        instance.fix(htmlPage);
    }

private:
    template <typename Location>
    static QString styleElementWithCode(const QByteArray& cssCode, const Location& location)
    {
        if (cssCode.isEmpty()) {
            qCWarning(MANPAGE) << "empty CSS file" << location;
            return QString();
        }
        return QString::fromUtf8("<style>" + cssCode + "</style>");
    }

    /**
     * Read the file contents and return it wrapped in a &lt;style&gt; HTML element.
     *
     * @return The &lt;style&gt; HTML element or an empty string in case of error.
     *
     * @note Referencing a local file via absolute path or file:// URL inside a &lt;link&gt;
     *       HTML element does not work because Qt WebEngine forbids such file system access.
     *       A comment under QTBUG-55902 proposes a workaround: pass "file://" as the baseUrl
     *       argument to QWebEnginePage::setHtml(). Unfortunately this base URL does not persist
     *       during back/forward web history navigation, so such navigation loads unstyled pages.
     */
    static QString readStyleSheet(const QString& fileName)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCWarning(MANPAGE) << "cannot read CSS file" << fileName << ':' << file.error() << file.errorString();
            return QString();
        }
        const auto cssCode = file.readAll();
        return styleElementWithCode(cssCode, fileName);
    }

    /**
     * Get the URL contents and return it wrapped in a &lt;style&gt; HTML element.
     *
     * @return The &lt;style&gt; HTML element or an empty string in case of error.
     */
    static QString getStyleSheetContents(const QUrl& url)
    {
        auto* const job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
        if (!job->exec()) {
            qCWarning(MANPAGE) << "couldn't get the contents of CSS file" << url << ':'
                               << job->error() << job->errorString();
            return QString();
        }
        const auto cssCode = job->data();
        return styleElementWithCode(cssCode, url);
    }

    static QString readCustomStyleSheet()
    {
        const auto customStyleSheetFile = QStringLiteral("kdevmanpage/manpagedocumentation.css");
        const QString cssFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, customStyleSheetFile);
        if (cssFilePath.isEmpty()) {
            qCWarning(MANPAGE) << "couldn't find" << customStyleSheetFile;
            return QString();
        }
        return readStyleSheet(cssFilePath);
    }

    StyleSheetFixer()
        : m_customStyleSheet{readCustomStyleSheet()}
    {
    }

    void fix(QString& htmlPage)
    {
        const QLatin1String headEndTag("</head>");
        const auto headEndTagPos = htmlPage.indexOf(headEndTag, 0, Qt::CaseInsensitive);
        if (headEndTagPos == -1) {
            qCWarning(MANPAGE) << "missing" << headEndTag << "on the HTML page.";
            return;
        }

        // Apply our custom style sheet to normalize look of the page. Embed the <style> element
        // into the HTML code directly rather than inject it with JavaScript to avoid reloading
        // and flickering of large pages such as cmake-modules man page.
        if (!m_customStyleSheet.isEmpty()) {
            htmlPage.insert(headEndTagPos, m_customStyleSheet);
        }

        expandUnsupportedLinks(htmlPage, headEndTagPos);
    }

    void expandUnsupportedLinks(QString& htmlPage, int endPos)
    {
        Q_ASSERT(endPos >= 0);

        static const QRegularExpression linkElement(QStringLiteral(R"(<link\s[^>]*rel="stylesheet"[^>]*>)"),
                                                    QRegularExpression::CaseInsensitiveOption);
        int startPos = 0;
        while (true) {
            const auto remainingPartOfThePage = QStringView{htmlPage}.sliced(startPos, endPos - startPos);
            const auto linkElementMatch = linkElement.match(remainingPartOfThePage);
            if (!linkElementMatch.hasMatch()) {
                break; // no more links to expand
            }
            startPos += linkElementMatch.capturedEnd();

            static const QRegularExpression hrefAttribute(QStringLiteral(R"|(\shref="([^"]*)")|"),
                                                          QRegularExpression::CaseInsensitiveOption);
            const auto hrefAttributeMatch = hrefAttribute.match(linkElementMatch.capturedView());
            if (!hrefAttributeMatch.hasMatch()) {
                qCWarning(MANPAGE) << "missing href attribute in a stylesheet <link> element.";
                continue;
            }

            const QUrl url{hrefAttributeMatch.captured(1)};
            const auto styleSheet = expandStyleSheet(url);
            if (styleSheet.isEmpty()) {
                continue; // no code => skip this <link> element as expanding it won't make a difference
            }

            const auto linkElementLength = linkElementMatch.capturedLength();
            const auto linkElementPos = startPos - linkElementLength;
            htmlPage.replace(linkElementPos, linkElementLength, styleSheet);

            const auto htmlPageSizeIncrement = styleSheet.size() - linkElementLength;
            startPos += htmlPageSizeIncrement;
            endPos += htmlPageSizeIncrement;
        }
    }

    QString expandStyleSheet(const QUrl& url)
    {
        const bool isLocalFile = url.isLocalFile();
        const bool isHelpUrl = !isLocalFile && url.scheme() == QLatin1String{"help"};
        if (!isLocalFile && !isHelpUrl) {
            qCDebug(MANPAGE) << "not expanding CSS file URL with scheme" << url.scheme();
            return QString();
        }

        // Must do it this way because when an empty string is the value stored
        // for url, it should be returned rather than re-read from disk.
        const auto alreadyExpanded = m_expandedStyleSheets.constFind(url);
        if (alreadyExpanded != m_expandedStyleSheets.cend()) {
            return alreadyExpanded.value();
        }

        QString newlyExpanded;
        if (isLocalFile) {
            newlyExpanded = readStyleSheet(url.toLocalFile());
        } else {
            Q_ASSERT(isHelpUrl);
            // Neither Qt WebKit nor Qt WebEngine knows about the help protocol and URL scheme.
            // Expand the file contents at the help URL to apply the style sheet.
            newlyExpanded = getStyleSheetContents(url);
        }

        m_expandedStyleSheets.insert(url, newlyExpanded);
        return newlyExpanded;
    }

    /// The style sheet does not change => read it once and store in a constant.
    const QString m_customStyleSheet;
    /// Referenced style sheets should be few and rarely modified => read them once and store in this cache.
    QHash<QUrl, QString> m_expandedStyleSheets;
};
} // unnamed namespace

ManPagePlugin* ManPageDocumentation::s_provider=nullptr;

ManPageDocumentation::ManPageDocumentation(const QString& name, const QUrl& url)
    : m_url(url), m_name(name)
{
    KIO::StoredTransferJob* transferJob = KIO::storedGet(m_url, KIO::NoReload, KIO::HideProgressInfo);
    connect( transferJob, &KIO::StoredTransferJob::finished, this, &ManPageDocumentation::finished);
    transferJob->start();
}

void ManPageDocumentation::finished(KJob* j)
{
    auto* job = qobject_cast<KIO::StoredTransferJob*>(j);
    if(job && job->error()==0) {
        m_description = QString::fromUtf8(job->data());
        StyleSheetFixer::process(m_description);
    } else {
        m_description.clear();
    }
    emit descriptionChanged();
}

KDevelop::IDocumentationProvider* ManPageDocumentation::provider() const
{
    return s_provider;
}

QString ManPageDocumentation::description() const
{
    return m_description;
}

QWidget* ManPageDocumentation::documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent )
{
    auto* view = new KDevelop::StandardDocumentationView(findWidget, parent);
    view->initZoom(provider()->name());
    view->setDocumentation(IDocumentation::Ptr(this));
    view->setDelegateLinks(true);
    QObject::connect(view, &KDevelop::StandardDocumentationView::linkClicked, ManPageDocumentation::s_provider->model(), &ManPageModel::showItemFromUrl);
    return view;
}

bool ManPageDocumentation::providesWidget() const
{
    return false;
}

QWidget* ManPageHomeDocumentation::documentationWidget(KDevelop::DocumentationFindWidget *findWidget, QWidget *parent){
    Q_UNUSED(findWidget);
    return new ManPageDocumentationWidget(parent);
}


QString ManPageHomeDocumentation::name() const
{
    return i18n("Man Content Page");
}

KDevelop::IDocumentationProvider* ManPageHomeDocumentation::provider() const
{
    return ManPageDocumentation::s_provider;
}

#include "moc_manpagedocumentation.cpp"

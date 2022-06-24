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

#include <KIO/TransferJob>
#include <KLocalizedString>

#include <QFile>
#include <QStandardPaths>

namespace {
class StyleSheetFixer
{
public:
    static void process(QString& htmlPage)
    {
        static StyleSheetFixer instance;
        instance.fix(htmlPage);
    }

private:
    /**
     * Read the file contents and return it wrapped in a &lt;style&gt; HTML element.
     *
     * @return The &lt;style&gt; HTML element or an empty string in case of error.
     *
     * @note Referencing a local file via absolute path or file:// URL inside a &lt;link&gt;
     *       HTML element does not work because Qt WebEngine forbids such file system access.
     */
    static QString readStyleSheet(const QString& fileName)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCWarning(MANPAGE) << "cannot read CSS file" << fileName << ':' << file.error() << file.errorString();
            return QString();
        }
        const auto cssCode = file.readAll();
        return QString::fromUtf8("<style>" + cssCode + "</style>");
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
        if (m_customStyleSheet.isEmpty()) {
            return; // nothing to do
        }

        const QLatin1String headEndTag("</head>");
        const auto headEndTagPos = htmlPage.indexOf(headEndTag, 0, Qt::CaseInsensitive);
        if (headEndTagPos == -1) {
            qCWarning(MANPAGE) << "missing" << headEndTag << "on the HTML page.";
            return;
        }

        // Apply our custom style sheet to normalize look of the page. Embed the <style> element
        // into the HTML code directly rather than inject it with JavaScript to avoid reloading
        // and flickering of large pages such as cmake-modules man page.
        htmlPage.insert(headEndTagPos, m_customStyleSheet);
    }

    /// The style sheet does not change => read it once and store in a constant.
    const QString m_customStyleSheet;
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


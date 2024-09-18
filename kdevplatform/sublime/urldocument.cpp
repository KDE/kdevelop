/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "urldocument.h"

#include <KTextEdit>
#include <KLocalizedString>

#include <QIcon>
#include <QMimeDatabase>
#include <QMimeType>
#include <QWidget>

namespace Sublime {

// class UrlDocumentPrivate

class UrlDocumentPrivate
{
public:
    QUrl url;
    QMimeType mimeType;
};



// class UrlDocument

UrlDocument::UrlDocument(Controller *controller, const QUrl &url)
    : Document(url.fileName(), controller)
    , d_ptr(new UrlDocumentPrivate())
{
    setUrl(url);
}

UrlDocument::~UrlDocument() = default;

QUrl UrlDocument::url() const
{
    Q_D(const UrlDocument);

    return d->url;
}

void UrlDocument::setUrl(const QUrl& newUrl)
{
    Q_D(UrlDocument);

    Q_ASSERT(newUrl.adjusted(QUrl::NormalizePathSegments) == newUrl);
    d->url = newUrl;
    d->mimeType = QMimeDatabase().mimeTypeForUrl(d->url);
    // remote URLs might not have a file name
    Q_ASSERT(!newUrl.fileName().isEmpty() || !newUrl.isLocalFile());
    auto title = newUrl.fileName();
    if (title.isEmpty()) {
        title = i18n("Untitled");
    }
    setTitle(title);
    setToolTip(newUrl.toDisplayString(QUrl::PreferLocalFile));
}

QWidget *UrlDocument::createViewWidget(QWidget *parent)
{
    ///@todo adymo: load file contents here
    return new KTextEdit(parent);
}

QString UrlDocument::documentType() const
{
    return QStringLiteral("Url");
}

QString UrlDocument::documentSpecifier() const
{
    Q_D(const UrlDocument);

    return d->url.url();
}

QMimeType UrlDocument::mimeType() const
{
    Q_D(const UrlDocument);

    return d->mimeType;
}

QIcon UrlDocument::defaultIcon() const
{
    Q_D(const UrlDocument);

    return QIcon::fromTheme(d->mimeType.iconName());
}

QString UrlDocument::title(TitleType type) const
{
    if (type == Extended)
        return Document::title() + QLatin1String(" (") + url().toDisplayString(QUrl::PreferLocalFile) + QLatin1Char(')');
    else
        return Document::title();
}

}

#include "moc_urldocument.cpp"

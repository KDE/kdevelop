/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_URLDOCUMENT_H
#define KDEVPLATFORM_URLDOCUMENT_H

#include <QUrl>

#include "sublimeexport.h"

#include "document.h"

class QMimeType;

namespace Sublime {

class UrlDocumentPrivate;

/**
@short Basic document that has an URL.
*/
class KDEVPLATFORMSUBLIME_EXPORT UrlDocument: public Document
{
    Q_OBJECT
public:
    UrlDocument(Controller *controller, const QUrl &url);
    ~UrlDocument() override;

    QString documentType() const override;

    QString documentSpecifier() const override;

    QUrl url() const;

    /**
     * @return the MIME type for the document's URL cached at the time of the last URL change
     */
    QMimeType mimeType() const;

    /**
     * @return the icon for the mimetype of the document url.
     */
    QIcon defaultIcon() const override;

    QString title(TitleType type) const override;

protected:
    QWidget *createViewWidget(QWidget *parent = nullptr) override;
    void setUrl(const QUrl& newUrl);

private:
    const QScopedPointer<class UrlDocumentPrivate> d_ptr;
    Q_DECLARE_PRIVATE(UrlDocument)
};

}

#endif


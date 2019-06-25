/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef KDEVPLATFORM_URLDOCUMENT_H
#define KDEVPLATFORM_URLDOCUMENT_H

#include <QUrl>

#include "sublimeexport.h"

#include "document.h"

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


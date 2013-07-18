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

#include <kurl.h>

#include "sublimeexport.h"

#include "document.h"

namespace Sublime {

/**
@short Basic document that has an URL.
*/
class SUBLIME_EXPORT UrlDocument: public Document {
public:
    UrlDocument(Controller *controller, const KUrl &url);
    ~UrlDocument();

    virtual QString documentType() const;

    virtual QString documentSpecifier() const;

    KUrl url() const;

    /**
     * @return the icon for the mimetype of the document url.
     */
    QIcon defaultIcon() const;

protected:
    virtual QWidget *createViewWidget(QWidget *parent = 0);
    void setUrl(const KUrl& newUrl);

private:
    struct UrlDocumentPrivate * const d;

};

}

#endif


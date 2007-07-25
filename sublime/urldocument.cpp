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
#include "urldocument.h"

#include <QWidget>
#include <QTextEdit>

namespace Sublime {

// struct UrlDocumentPrivate

struct UrlDocumentPrivate {
    KUrl url;
};



// class UrlDocument

UrlDocument::UrlDocument(Controller *controller, const KUrl &url)
    :Document(url.fileName(), controller), d( new UrlDocumentPrivate() )
{
    d->url = url;
}

UrlDocument::~UrlDocument()
{
    delete d;
}

KUrl UrlDocument::url() const
{
    return d->url;
}

QWidget *UrlDocument::createViewWidget(QWidget *parent)
{
    ///@todo adymo: load file contents here
    return new QTextEdit(parent);
}

}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

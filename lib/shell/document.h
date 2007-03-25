/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
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
#ifndef KDEV_DOCUMENT_H
#define KDEV_DOCUMENT_H

#include "idocument.h"
#include <sublime/urldocument.h>

namespace Sublime { class View; }

namespace KDevelop {

class Core;

/**
The abstract KDevelop document.

This document is designed to be subclassed by all document implementations.
*/
class Document: public Sublime::UrlDocument, public IDocument {
    Q_OBJECT
public:
    Document(const KUrl &url);
    virtual ~Document();

    virtual KUrl url() const;

    /**Performs document activation actions if any.
    This implementation just notifies the document controller about activation.*/
    virtual void activate(Sublime::View *activeView);

protected:
    void notifyStateChanged();
    void notifySaved();
    void notifyActivated();

private:
    class DocumentPrivate *d;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
